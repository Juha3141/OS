//////////////////////////////////////////////////////////////////////////////////
// File "MemoryManagement.cpp"                                                  //
// Written by : Juha Cho                                                        //
// Started Date : 2022.10.18                                                    //
// Description : Contains //
//////////////////////////////////////////////////////////////////////////////////

#include <MemoryManagement.hpp>
#include <Kernel.hpp>

// #define DEBUG

void Kernel::MemoryManagement::Initialize(void) {
	int i = 0;
	int j;
	unsigned long TotalUsableMemory = 0;
	Kernel::MemoryManagement::NodeManager *NodeManager;
	QuerySystemAddressMap *E820 = (QuerySystemAddressMap *)MEMORYMANAGEMENT_E820_ADDRESS;
	// Location of the E820, if you are curious, check line 36 of the Kernel16.asm
	NodeManager = (Kernel::MemoryManagement::NodeManager*)MEMORYMANAGEMENT_MEMORY_STARTADDRESS; // Allocate system structure
	Kernel::printf("Node Manager Location : 0x%X\n" , MEMORYMANAGEMENT_MEMORY_STARTADDRESS);
	Kernel::printf("E820 Address          : 0x%X\n" , MEMORYMANAGEMENT_E820_ADDRESS);
	/* The problem was that the g++ compiler somehow interpretes the code incorrectly, and made a unintentional codes.
	 * So, to solve the problem, I just made a new function made out of assembly language.
	 * 
	 * To : Me from the future
	 * If you find some weird code that seems like a problem of compiler, just code it to assembly language.
	 * Remember... assembly language is the way to solve everything..
	*/
	// Get the unusable memory
	// To-do : Mask unusable memory
	while(1) {
		if(E820[i].Type == 0x00) {
			break;
		}
		Kernel::printf("0x%X~0x%X : 0x%X\n" , E820[i].Address , E820[i].Address+E820[i].Length , E820[i].Type);
		i += 1;
	}
	TotalUsableMemory = GetUsableMemory(MEMORYMANAGEMENT_E820_ADDRESS , MEMORYMANAGEMENT_MEMORY_STARTADDRESS);
	NodeManager->Initialize(MEMORYMANAGEMENT_MEMORY_STARTADDRESS+sizeof(Kernel::MemoryManagement::NodeManager) , TotalUsableMemory);		// Initialize the node manager
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// Description : Find the suitable node for required size, and if it requires, seperate the node.    //
// The function allocates segment by this sequence :                                                 //
// 1. Search the suitable node, if there is no suitable node, create one.                            //
// 2. If the size of suitable node, seperate the node                                                //
// Creating Node : Create new node after the last node                                               //
// If there is no node : Create new one                                                              //
// Seperating Sequence :                                                                             //
// 1. Create new node in the existing node(Existing node should be set to required size,             //
//      more specifics on the actual code.)                                                          //
// 2. Set the NextNode value of the newly created node to existing node's NextNode value.            //
// 3. Set the PreviousNode value of the newly created node to address of existing node.              //
// 4. Set the NextNode value of the existing node to address of newly created node                   //
// -> the PreviousNode value of the existing node should not be changed.                             //
//                                                                                                   //
// Examples of the situation :                                                                       //
// 1. There is no suitable segment                 2. There is suitable segment                      //
// +-----------------+                               +-----------------+                             //
// |  Require : 3MB  |                             |  Require : 3MB  |                               //
// +-----------------+                               +-----------------+                             //
// Memory Pool :                                    Memory Pool :                                    //
// +-----+-----+-----+---------------------+       +-----------------+-----------------------+       //
// | 1MB | 1MB | 1MB | ...                   |       |       3MB       | ...                   |     //
// +-----+-----+-----+---------------------+       +-----------------+-----------------------+       //
// -> Solve : Create new node after the last node  -> Solve : Allot the suitable memory              //
// +-----+-----+-----+---------------+-----+       +-----------------+-----------------------+       //
// | 1MB | 1MB | 1MB | Soothed : 3MB | ... |       |  Soothed : 3MB  | ...                   |       //
// +-----+-----+-----+---------------+-----+       +-----------------+-----------------------+       //
//                                                                                                   //
// 3. There is bigger segment                        4. The worst case : External Fragmentation      //
// +-----------------+                               +-----------------+                             //
// |  Require : 3MB  |                               |  Require : 3MB  |                             //
// +-----------------+                               +-----------------+                             //
// Memory Pool :                                    Memory Pool :                                    //
// +-----------------------------+---------+       +-----+-----+-----+-----------+-----+-----+       //
// |              5MB            | ...     |       |1MB U|1MB F|1MB U|   2MB F   |1MB U|1MB U|       //
// +-----------------------------+---------+       +-----+-----+-----+-----------+-----+-----+       //
// -> Solve : Seperate the segment                   -> There is no space to allocate, even there is //
// +-----------------+-----------+---------+       actually available space.                         //
// |  Soothed : 3MB  |    2MB    | ...     |       -> Solve : To optimize the allocation             //
// +-----------------+-----------+---------+                                                         //
//                    ^~~~~~~~~~~ Usable                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////////

void *Kernel::MemoryManagement::Allocate(unsigned long Size) {
	unsigned long TotalNodeSize = 0;	// TotalNodeSize : Size of the total node that is going to be used for allocation
	// Load the NodeManager from the local address
	Kernel::MemoryManagement::NodeManager *NodeManager = (Kernel::MemoryManagement::NodeManager *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
	if(Size == 0) {
		Kernel::printf("Allocation Error #0 : Zero allocated size\n");
		return 0x00;
	}
	struct Node *Node = (struct Node *)NodeManager->SearchReasonableNode(Size); // Search available node
	struct Node *UsableNode;			// UsableNode : The node that is seperated
	if(Node->NextNode != 0) {			// If the next node is present, set the TotalNodeSize to the size of searched node.
		TotalNodeSize = Node->NextNode-((unsigned long)Node)-sizeof(struct Node);	// TotalNodeSize : size of the current node
	}
	if(Node == 0) { // If we have to create new node, create new node at the end of the segments
#ifdef DEBUG
		Kernel::printf("Create new node at CurrentAddress : 0x%X\n" , NodeManager->CurrentAddress);
#endif
		Node = (struct Node *)NodeManager->SearchNewNodeLocation();	// New node address : CurrentAddress
		/*************
		 * PROBLEM : Creating new node overwrites already existing node, we need to create
		 * search system like SearchReasonableNode.
		 * <SOLVED>
		*************/
		TotalNodeSize = 0;								// Set the value to 0 so that the <Node Seperation Sequence> can't be executed.
		Node->Using = 1;            					// Set the using flag to 1
		Node->Signature = MEMORYMANAGEMENT_SIGNATURE;   // Write a signature to mark it's the valid node
		// Next node : Offset + Size of the node + Size of the node structure
		Node->NextNode = (((unsigned long)Node)+Size+sizeof(struct Node));		// Modify the address of the next node(because the node is a fresh-made node.)
		((struct Node *)Node->NextNode)->PreviousNode = (unsigned long)Node; 	// Set PreviousNode to zero, so that the searcher
																				// (I mean - the searching sequence) can know that it's the first node.
		if((unsigned long)Node == NodeManager->StartAddress) {					// For the first node, the previous node shouldn't be exist.
			Node->PreviousNode = 0;												// Set PreviousNode to zero
		}
	}
	else {
		Node->Using = 1;            			      							// Set the using flag to 1
		Node->Signature = MEMORYMANAGEMENT_SIGNATURE;				     	    // Write a signature to mark it's the valid node
		Node->NextNode = (((unsigned long)Node)+Size+sizeof(struct Node));		// Modify the address of next node(because the node is currently seperated.)
		((struct Node *)Node->NextNode)->PreviousNode = (unsigned long)Node;	// Link the node properly
		// Same as before
		if((unsigned long)Node == NodeManager->StartAddress) {					// For the first node, the previous node shouldn't be exist.
			Node->PreviousNode = 0;												// Set PreviousNode to zero
		}
		NodeManager->CurrentAddress = Node->NextNode; // Update CurrentAddress
#ifdef DEBUG
		Kernel::printf("Found Node : 0x%08X\n" , Node);
#endif
	}
	// <Node Seperation Sequence>
	if(TotalNodeSize > Size) {	// If the size of the given segment is bigger than needed,
								// Seperate the bigger segment to needed size, and leave the left segment available
#ifdef DEBUG
		Kernel::printf("Creating usable area\n");
#endif
		UsableNode = (struct Node *)Node->NextNode;						// UsableNode : Not being used, used for another allocation
		UsableNode->Using = 0;											// Not using(Available)
		UsableNode->Signature = MEMORYMANAGEMENT_SIGNATURE;				// Mark a signature to make it valid
		// NextNode : Address of the current node(UsableNode) + Size of the unused area(TotalNodeSize-Size) + Size of the node structure
		UsableNode->NextNode = (((unsigned long)UsableNode)+(TotalNodeSize-Size)+sizeof(struct Node));	// Modify the address of next node
		((struct Node *)UsableNode->NextNode)->PreviousNode = (unsigned long)UsableNode;				// Link the node properly
	}
#ifdef DEBUG
	Kernel::printf("Updated CurrentAddress : 0x%X\n" , NodeManager->CurrentAddress);
#endif
	// Return the actual available address : Node address + size of the node structure
	return ((void *)((unsigned long)Node+sizeof(struct Node)));			// Actual address that is going to be used : 
																		// Address after area of node
}

/* To-do : Create a allocation optimizing system */

// Description : Find the node, deallocate it, and if it's needed, merge the segments that is linearly usable.
////////////////////////////////////////////////////////////////////////////////////////////////
// For example, the function merges segments in those situations :                            //
// Situation #1 :                              Situation #2 :                                 //
//                                                                                            //
// Node to deallocate                                                 Node to deallocate      //
//  VVVVVVVVVVVVVVV                                                  VVVVVVVVVVVVVVV          //
// +---------------+----------------------+  +----------------------+---------------+         //
// |  Using(1MB)   |      Usable(2MB)     |  |      Usable(2MB)     |  Using(1MB)   |         //
// +---------------+----------------------+  +----------------------+---------------+         //
// <Deallocation Sequence>                   <Deallocation Sequence>                          //
// 1. Go to the next node and check if       1. Go to the previous node and check if          //
//    it's usable(mergable) until we hit     it's usable until we hit not usable node         //
//    not usable node                                                                         //
// 2. Change the target node(= Node to       2. Change the NextNode of the last node          //
//    deallocate)'s NextNode to the node     that we lastly found from searching to           //
//       that we lastly found from searching     target node's next node.                     //
// 3. Change the node's PreviousNode to      3. Set the last node's flag to usable            //
//    target node                                                                             //
// 4. Set the target node's flag to usable                                                    //
// +---------------+----------------------+  +---------------+----------------------+         //
// |  Usable(1MB)  |      Usable(2MB)     |  |      Usable(2MB)     |  Usable(1MB)  |         //
// +---------------+----------------------+  +---------------+----------------------+         //
// This can be merged to :                      This can be also merged to :                  //
// +--------------------------------------+  +--------------------------------------+         //
// |             Usable(3MB)              |  |             Usable(3MB)              |         //
// +--------------------------------------+  +--------------------------------------+         //
////////////////////////////////////////////////////////////////////////////////////////////////

void Kernel::MemoryManagement::Free(void *Address) {
	unsigned long CurrentNode;
	unsigned long NextNode;
	unsigned long PreviousNode;
	// Load the NodeManager from the local address
	Kernel::MemoryManagement::NodeManager *NodeManager = (Kernel::MemoryManagement::NodeManager *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
	if((unsigned long)Address < NodeManager->StartAddress) {
		Kernel::printf("Deallocation Error #0 : Low Memory Address\n");
		return;
	}
	struct Node *Node = (struct Node *)(((unsigned long)Address)-sizeof(struct Node));  // Address of Node : Address - Size of the node structure
	if((Node->Using == 0)||(Node->Signature != MEMORYMANAGEMENT_SIGNATURE)) {			// If Node is not using, or not present, print error and leave.
		Kernel::printf("Deallocation Error #1 : Not Allocated Memory\n");
		return;
	}
	// Allocated Size : Location of the next node - Location of current node
#ifdef DEBUG
	Kernel::printf("Allocated Size : %d\n" , Node->NextNode-((unsigned long)Address));
#endif
	// If the previous node is usable, and present, the node can be merged.
	// (Why are we merging and seperating the segment? Because, it can reduce the problem of external fragmentation)
	if((Node->PreviousNode != 0x00) && (((struct Node *)Node->PreviousNode)->Using == 0) && (((struct Node *)Node->PreviousNode)->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
#ifdef DEBUG
		Kernel::printf("Previous area is mergable\n");
#endif
		CurrentNode = (unsigned long)Node;				// CurrentNode : Saves the current node for later
		while((!(((struct Node *)Node->PreviousNode)->Using)) && (Node->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
			// Search the nodes that is available for merging, and erase all usable node to make a free space 
			// -> until we find already using node, or not present node.
#ifdef DEBUG
			Kernel::printf("0x%X\n" , Node->PreviousNode);
#endif
			Node = (struct Node *)Node->PreviousNode; 	// Head to previous node 
			if(Node == 0) {							  	// If current node is zero, that means we reached the beginning of the memory.
				break;
			}
		}
		Node->Using = 0;							  							// Free the node
		Node->NextNode = ((struct Node *)CurrentNode)->NextNode;				// Modify the location of the next node
		((struct Node *)Node->NextNode)->PreviousNode = (unsigned long)Node;	// Modify PreviousNode value of the next node
																				// (Linking the nodes properly by the newly merged node)
		Node->Signature = MEMORYMANAGEMENT_SIGNATURE;							// Write a signature
		memset((struct Node *)CurrentNode , 0 , sizeof(struct Node));			// Erase the last current node
	}
	// If next node is usable, and present, the node can be merged.
	if((((struct Node *)Node->NextNode)->Using == 0) && (((struct Node *)Node->NextNode)->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
#ifdef DEBUG
		Kernel::printf("Allocated area is mergable\n");
#endif
		CurrentNode = (unsigned long)Node;					// CurrentNode : Saves the current node for later
		Node = (struct Node *)Node->NextNode; 				// Save the current node, and move to next node
		while((!(((struct Node *)Node->NextNode)->Using)) && (Node->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
			// Search the nodes that is available for merging, and erase all usable node to make a free space
			// -> until we find already using node, or not present node.
			NextNode = Node->NextNode;						// Save the location of next node, because the node is going to be erased, 
															// and we need the location of it for merging.
			// memset(((struct Node *)NextNode) , 0 , sizeof(struct Node));	// Erase the node to allocate something
			// Actually, removing the node is useless, because it is going to be overwrited by user of the memory area.
			// Size of the node which is going to be merged : Location of the next node of the current node - Location of the current node*
			// Current node : node that is going to be merged
#ifdef DEBUG
			Kernel::printf("Merging Node, Size : %d\n" , Node->NextNode-((unsigned long)Node-sizeof(struct Node)));
#endif
			Node = (struct Node *)Node->NextNode; 			// Go to the next node and keep search
		}
		// Done erasing : Modify the next node location to the end of the node(It's going to be using node).
		((struct Node *)CurrentNode)->NextNode = NextNode;  // Modify the location of the next node
		((struct Node *)CurrentNode)->Using = 0;			// Free the node
		NodeManager->LastFreedAddress = CurrentNode;		// Modify LastFreedAddress to find node slightly more efficient.
	}
	else { // Not mergable, just set the flag to usable and modify LastFreedAddress.
		Node->Using = 0;
		NodeManager->LastFreedAddress = (unsigned long)Node;
	}
	// If the first node is usable, and there is no next nodes, then the node will be removed.
	// But, if the first node is being used, or there is next nodes, then the node is not going to be removed.
	Node = (struct Node *)NodeManager->StartAddress;
	if((Node->Using == 0) && (((struct Node *)Node->NextNode)->Signature != 0x3141)) { // If it sooths the condition,
#ifdef DEBUG
		Kernel::printf("The Last Survived Node is killable\n");
#endif
		memset(Node , 0 , sizeof(struct Node));		  // Erase the node(Set everything to 0)
		NodeManager->CurrentAddress = NodeManager->StartAddress;	// Reset the address so that
		NodeManager->LastFreedAddress = NodeManager->StartAddress;  // Next allocation will be started at StartAddress
	}
}

// Description : Print informations of every node
void Kernel::MemoryManagement::NodeManager::MapNode(void) {
	int i = 0;
	struct Node *Node = (struct Node *)this->StartAddress;	// Start from StartAddress, 
	while(Node->Signature == MEMORYMANAGEMENT_SIGNATURE) {
		Kernel::printf("%02d:%d|%d " , i++ , Node->Using , Node->NextNode-(unsigned long)Node-sizeof(struct Node));
		Node = (struct Node *)Node->NextNode;				// Print the information of the node, until the node runs out.
	}
	Kernel::printf("\n");
}

// Description : Initializes the variables
void Kernel::MemoryManagement::NodeManager::Initialize(unsigned long StartAddress , unsigned long TotalUsableMemory) {
	this->StartAddress = StartAddress; 				// StartAddress 	 : The location of the memory pool
	this->CurrentAddress = StartAddress;			// CurrentAddress    : The location of current position
	this->LastFreedAddress = 0;						// LastFreedAddress  : The location of lastly freed segment, 0 = There's yet no freed segment
	this->TotalUsableMemory = TotalUsableMemory;	// TotalUsableMemory : Total available memory to use(Size of the memory pool)
}

// Description : Search the node that is bigger than the given size from the argument
unsigned long Kernel::MemoryManagement::NodeManager::SearchReasonableNode(unsigned long Size) {
	struct Node *Node;
	if(this->CurrentAddress == this->StartAddress) {
#ifdef DEBUG
		Kernel::printf("Returning Start of the memory\n");
#endif
		// If current address is start of the memory,
		return this->StartAddress;  // return the start address.
	}
	Node = (struct Node *)((this->LastFreedAddress == 0) ? this->CurrentAddress : this->LastFreedAddress);
	while(Node->Signature == MEMORYMANAGEMENT_SIGNATURE) {
		if((Node->Using == 0) && ((((Node->NextNode-(unsigned long)Node)-sizeof(struct Node))) >= Size)) {
#ifdef DEBUG
			Kernel::printf("Free Node Found : At 0x%08X, Size : %d\n" , Node , (Node->NextNode-(unsigned long)Node-sizeof(struct Node)));
#endif
			return (unsigned long)Node;
		}
		Node = (struct Node *)Node->NextNode;
	}
	return 0; // No node available, need to create new node
}

// Description : Search the location for new node
unsigned long Kernel::MemoryManagement::NodeManager::SearchNewNodeLocation(void) {
	struct Node *Node;
	if(this->CurrentAddress == this->StartAddress) {
#ifdef DEBUG
		Kernel::printf("Returning Start of the memory\n");
#endif
		// If current address is start of the memory,
		return this->StartAddress;  // return the start address.
	}
	// If there is no freed address -> Use CurrentAddress
	// If there is freed address 	-> Use LastFreedAddress
	Node = (struct Node *)((this->LastFreedAddress == 0) ? this->CurrentAddress : this->LastFreedAddress);
	while(Node->Signature == MEMORYMANAGEMENT_SIGNATURE) {	// Go to the last node
		Node = (struct Node *)Node->NextNode;
	}
	return (unsigned long)Node;		// Return the location of the last node.
}