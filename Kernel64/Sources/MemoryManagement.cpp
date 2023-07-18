//////////////////////////////////////////////////////////////////////////////////
// File "MemoryManagement.cpp"                                                  //
// Written by : Juha Cho                                                        //
// Started Date : 2022.10.18                                                    //
// Description : Contains //
//////////////////////////////////////////////////////////////////////////////////

#include <MemoryManagement.hpp>
#include <Kernel.hpp>

// #define DEBUG

// for C++ standard

void *operator new(unsigned long Size) {
    return MemoryManagement::Allocate(Size);
}

void *operator new[](unsigned long Size) {
    return MemoryManagement::Allocate(Size);
}

void operator delete(void *Pointer , unsigned long) {
    MemoryManagement::Free(Pointer);
}

void MemoryManagement::Initialize(void) {
	int E820EntryCount = 0;
	unsigned long TotalUsableMemory = 0;
	MemoryManagement::NodeManager *NodeManager;
	// Location of the E820, if you are curioused, check line 36 of the Kernel16.asm
	QuerySystemAddressMap *E820 = (QuerySystemAddressMap *)MEMORYMANAGEMENT_E820_ADDRESS;
	NodeManager = (MemoryManagement::NodeManager*)MEMORYMANAGEMENT_MEMORY_STARTADDRESS; // Allocate system structure
	printf("Node Manager Location : 0x%X\n" , MEMORYMANAGEMENT_MEMORY_STARTADDRESS);
	printf("E820 Address          : 0x%X\n" , MEMORYMANAGEMENT_E820_ADDRESS);
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
		// If entry type is zero, we reached the end of the table.
		if(E820[E820EntryCount].Type == 0x00) {
			break;
		}
		// printf("%X~%X %d\n" , E820[E820EntryCount].Address , E820[E820EntryCount].Address+E820[E820EntryCount].Length , E820[E820EntryCount].Type);
		E820EntryCount += 1;
	}
	TotalUsableMemory = GetUsableMemory(MEMORYMANAGEMENT_E820_ADDRESS , MEMORYMANAGEMENT_MEMORY_STARTADDRESS);
	// Initialize the node manager
	NodeManager->Initialize(MEMORYMANAGEMENT_MEMORY_STARTADDRESS+sizeof(MemoryManagement::NodeManager) , TotalUsableMemory
	                       , (QuerySystemAddressMap *)MEMORYMANAGEMENT_E820_ADDRESS , E820EntryCount);
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

static bool IsAligned4K(unsigned long Address) {
	if((Address & 0xFFF) == 0x00) {
		return true;
	}
	return false;
}

static bool IsAligned8K(unsigned long Address) {
	if((Address & 0x1FFF) == 0x00) {
		return true;
	}
	return false;
}

void *MemoryManagement::Allocate(unsigned long Size , MemoryManagement::ALIGNMENT Alignment) {
	int i;
	unsigned long Address;
	unsigned long TotalNodeSize = 0;	// TotalNodeSize : Size of the total node that is going to be used for allocation
	// Load the NodeManager from the local address
	MemoryManagement::NodeManager *NodeManager = (MemoryManagement::NodeManager *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
	if(Size == 0) {
		printf("Allocation Error #0 : Zero allocated size\n");
		return 0x00;
	}
	struct Node *Node = (struct Node *)NodeManager->SearchReasonableNode(Size); // Search available node
	struct Node *SeparatedNode;   // UsableNode : The node that is separated
	if(Node->Next != 0) {			// If the next node is present, set TotalNodeSize to the size of searched node.
		TotalNodeSize = ((unsigned long)Node->Next)-((unsigned long)Node)-sizeof(struct Node);	// TotalNodeSize : size of the current node
	}
	if(Node == 0) { // If we have to create new node, create new node at the end of the segments
		// printf("Creating new node\n");
		Node = NodeManager->CreateNewNode(Size , Alignment);
		if(Node == 0x00) {
			printf("Error! no node can be created anymore\n");
			while(1) {
				;
			}
			return 0x00;
		}
		TotalNodeSize = 0;								// Set the value to 0 so that the <Node Seperation Sequence> can't be executed.	
		// Update this->CurrentAddress, this->CurrentAddress : Next node of lastly created node
		NodeManager->CurrentNode = Node;
		// Initialize the next node to remove potential error from garbage memory
	}
	else {	// Using already existing node
		// printf("Using existing node\n");
		// printf("NodeLocation : 0x%X\n" , Node);
		if(Alignment != NO_ALIGN) {
			// Search New alignable location
			// printf("Alignment : %d\n" , Alignment);
			Node = (struct Node *)NodeManager->SearchAlignedNode(Size , Alignment);
			// printf("Found aligned, new one : 0x%X\n" , Node);
			if(Node == 0) {
				Node = NodeManager->CreateNewNode(Size , Alignment);
			}
		}
		NodeManager->WriteNodeData(Node , 1 , Size , NO_ALIGN);
		// Seperate node
		if(TotalNodeSize >= Size) {
			// Separate node to prevent internal fragmentation(Allow residual unused area usable)
			SeparatedNode = (struct Node *)(((unsigned long)Node+sizeof(struct Node))+(Size));
			if(SeparatedNode >= Node->Next) {
				printf("Something terrible happend\n");
				// panic
				while(1) {
					;
				}
			}
			NodeManager->WriteNodeData(SeparatedNode , 0 , TotalNodeSize-Size-sizeof(struct Node) , NO_ALIGN , (unsigned long)Node->Next , (unsigned long)Node);
		}
	}
	NodeManager->CurrentlyUsingMemory += Size;
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

// problem in memory free system
void MemoryManagement::Free(void *Address) {
	bool Merged = false;
	unsigned long TotalMergingSize = 0;
	struct Node *NodePointer;
	struct Node *CurrentNode;
	struct Node *NextNode;
	struct Node *PreviousNode;
	// Temporal Change : Memory management system doesn't work properly. 
	// PreviousNode -> often value is corrupted....
	// Load the NodeManager from the local address
	MemoryManagement::NodeManager *NodeManager = (MemoryManagement::NodeManager *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
	if((unsigned long)Address < (unsigned long)NodeManager->StartNode) {
		printf("Deallocation Error #0 : Low Memory Address , 0x%X\n" , Address);
		return;
	}
	struct Node *Node = (struct Node *)(((unsigned long)Address)-sizeof(struct Node));  // Address of Node : Address - Size of the node structure
	if((Node->Using == 0)||(Node->Signature != MEMORYMANAGEMENT_SIGNATURE)) {			// If Node is not using, or not present, print error and leave.
		printf("Deallocation Error #1 : Not Allocated Memory , 0x%X\n" , Address);
		return;
	}
	// Allocated Size : Location of the next node - Location of current node
	// If next node is usable, and present, the node can be merged.
	NodeManager->CurrentlyUsingMemory -= Node->Size;
	if((Node->Next != 0x00) && (Node->Next->Using == 0) && (Node->Next->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
		// printf("Next mergable\n");
		Merged = true;
		CurrentNode = Node;									// CurrentNode : Saves the current node for later
		NodePointer = Node;									// Save the current node, and move to next node
		while((NodePointer->Next->Using == 0) && (NodePointer->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
			if(NodePointer->Next == 0x00) {
				break;
			}
			NodePointer = (struct Node *)NodePointer->Next; 				// Go to the next node and keep search
		}
		/*
		printf("Merging Node(Next) : 0x%X~0x%X\n" , CurrentNode , NodePointer);
		printf("Total Merging size : %d\n" , (((unsigned long)NodePointer->Next)-((unsigned long)CurrentNode)-sizeof(struct Node)));
		*/
		// Done erasing : Modify the next node location to the end of the node(It's going to be using node).
		NodeManager->WriteNodeData(((struct Node *)CurrentNode) , 0 , (((unsigned long)NodePointer->Next)-((unsigned long)CurrentNode)-sizeof(struct Node)) , NO_ALIGN , (unsigned long)NodePointer->Next); // Free the node
	}
	// If the previous node is usable, and present, the node can be merged.
	// (Why are we merging and seperating the segment? Because, it can reduce the external fragmentation)
	if((Node->Previous != 0x00) && (Node->Previous->Using == 0) && (Node->Previous->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
		// printf("Previous mergable\n");
		Merged = true;
		CurrentNode = Node;				// CurrentNode : Saves the current node for later
		NodePointer = Node;
		while((NodePointer->Previous->Using == 0) && (NodePointer->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
			// Search the nodes that is available for merging, and erase all usable node to make a free space 
			// -> until we find already using node, or not present node.
			if(NodePointer->Previous == 0x00) {
				break;
			}
			NodePointer = (struct Node *)NodePointer->Previous; 	    // Head to previous node 
		}
		/*
		printf("Merging Node(Prev) : 0x%X~0x%X\n" , CurrentNode , NodePointer);
		printf("CurrentNode->Next : 0x%X\n" , CurrentNode->Next);
		printf("Size : %d\n" , (((unsigned long)CurrentNode->Next)-((unsigned long)NodePointer)-sizeof(struct Node)));
		*/
		NodeManager->WriteNodeData(NodePointer , 0 , (((unsigned long)CurrentNode->Next)-((unsigned long)NodePointer)-sizeof(struct Node)) , NO_ALIGN , (unsigned long)CurrentNode->Next);
		NodeManager->LastlyFreedNode = NodePointer;
		/*
		NextNode = Node->Next; // Next node of original node
		PreviousNode = Node->Previous->Previous;
		NodeManager->WriteNodeData(Node->Previous , 0 , ((unsigned long)Node)-(unsigned long)Node->Previous-sizeof(struct Node) , (unsigned long)NextNode , (unsigned long)PreviousNode);
		*/
	}
	if(Merged == false) {
		// printf("No merge\n");
		Node->Using = 0;
		NodeManager->LastlyFreedNode = Node;
		if(Node->Next == 0x00) {
			// printf("No next free\n");
			Node->Previous->Next = 0x00;
			memset(Node , 0 , sizeof(struct Node));
		}
	}
	// If the first node is usable, and there is no next nodes, then the node will be removed.
	// But, if the first node is being used, or there is next nodes, then the node is not going to be removed.
	Node = NodeManager->StartNode;
	if((Node->Using == 0) && (((struct Node *)Node->Next) == 0x00)) { // If it sooths the condition,
		memset(Node , 0 , sizeof(struct Node));		  // Erase the node(Set everything to 0)
		NodeManager->CurrentNode = NodeManager->StartNode;	// Reset the address so that
		NodeManager->LastlyFreedNode = NodeManager->StartNode;  // Next allocation will be started at StartAddress
	}
}

void MemoryManagement::ProtectMemory(unsigned long StartAddress , unsigned long MemorySize) {
	MemoryManagement::NodeManager *NodeManager = (MemoryManagement::NodeManager *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
	NodeManager->AddUnusableMemory(StartAddress , MemorySize);
	printf("Added to unusable memory list : 0x%X~0x%X\n" , StartAddress , StartAddress+MemorySize);
}

// Description : Print informations of every node
void MemoryManagement::NodeManager::MapNode(void) {
	int i = 0;
	struct Node *Node = this->StartNode;	// Start from StartAddress, 
	while(Node->Signature == MEMORYMANAGEMENT_SIGNATURE) {
		printf("%02d:%d|%d " , i++ , Node->Using , ((unsigned long)Node->Next)-(unsigned long)Node-sizeof(struct Node));
		Node = Node->Next;				// Print the information of the node, until the node runs out.
	}
	printf("\n");
}

// Description : Initializes the variables
void MemoryManagement::NodeManager::Initialize(unsigned long StartAddress , unsigned long TotalUsableMemory , QuerySystemAddressMap *E820 , int E820EntryCount) {
	int i;
	this->StartNode = (struct Node *)StartAddress;   // StartAddress 	     : The location of the memory pool
	this->CurrentNode = (struct Node *)StartAddress; // CurrentAddress       : The location of current position
	this->LastlyFreedNode = 0;						 // LastFreedAddress     : The location of lastly freed segment, 0 = There's yet no freed segment
	this->TotalUsableMemory = TotalUsableMemory;	 // TotalUsableMemory    : Total available memory to use(Size of the memory pool)
	this->CurrentlyUsingMemory = 0;					 // CurrentlyUsingMemory : Currently using memory size
	printf("Unusable Memories List Location : 0x%X\n" , UnusableMemories);
	for(i = UnusableMemoryEntryCount = 0; i < E820EntryCount; i++) {
		if(E820[i].Type != MEMORYMANAGEMENT_E820_USABLE) { // If memory is not usable, put it into unusable memories list
			memcpy(&(UnusableMemories[UnusableMemoryEntryCount]) , &(E820[i]) , sizeof(QuerySystemAddressMap));
			UnusableMemoryEntryCount++;
		}
	}
}

// Description : Search the node that is bigger than the given size from the argument
struct MemoryManagement::Node *MemoryManagement::NodeManager::SearchReasonableNode(unsigned long Size) {
	struct Node *Node;
	Node = this->StartNode;
	while(Node->Signature == MEMORYMANAGEMENT_SIGNATURE) {
		if((Node->Using == 0) && (Node->Size >= Size) && (Node->Size-Size > sizeof(struct Node))) {
			// printf("Free Node Found : At 0x%X, Size : %d, %d\n" , Node , (((unsigned long)Node->Next)-(unsigned long)Node-sizeof(struct Node)) , Node->Size);
			return Node;
		}
		Node = Node->Next;
	}
	return 0; // No node available, need to create new node
}

/// @brief Searches node that is already aligned
/// @param Size Size of the node
/// @param Alignment Option of alignment
/// @return location of the node
struct MemoryManagement::Node *MemoryManagement::NodeManager::SearchAlignedNode(unsigned long Size , MemoryManagement::ALIGNMENT Alignment) {
	struct Node *Node;
	unsigned long AlignedAddress;
	Node = (struct Node *)((this->LastlyFreedNode == 0) ? this->CurrentNode : this->LastlyFreedNode);
	while(Node->Signature == MEMORYMANAGEMENT_SIGNATURE) { // going forward until we meet invalid node
		if(Node->Using == 0) { // If node is usable
			// Check whether this node is aligned, or if aligned in future, fits the required size.
			AlignedAddress = AlignAddress(((unsigned long)Node)+sizeof(struct Node) , Alignment);
			// Get the aligned address of the node
			if(((AlignedAddress+Size+sizeof(struct Node)) <= (unsigned long)Node->Next)) { // If address of aligned node is above the region of the node -> Skip.
				return (struct Node *)(AlignedAddress-sizeof(struct Node));
			}
		}
		Node = (struct Node *)Node->Next; // forward
	}
	return 0x00; // No node available, need to create new node
}

// Description : Search the location for new node
struct MemoryManagement::Node *MemoryManagement::NodeManager::SearchNewNodeLocation(unsigned long *PreviousNode) {
	struct Node *Node;
	if(this->StartNode->Using == 0) {
		// printf("Returning Start of the memory\n");
		// If current address is start of the memory,
		// printf("allocating fresh memory!\n");
		*PreviousNode = 0x00;
		// printf("Giving a fresh new memory : 0x%X\n" , this->StartNode);
		return this->StartNode;  // return the start address.
	}
	// If there is no freed address -> Use CurrentAddress
	// If there is freed address 	-> Use LastFreedAddress
	Node = this->CurrentNode;
	while((Node->Next != 0x00) && (Node->Signature != MEMORYMANAGEMENT_SIGNATURE)) {	// Go to the last node
		// printf("Node       : 0x%X\n" , Node);
		// printf("Node->Next : 0x%X\n" , Node->Next);
		Node = Node->Next;
	}
	*PreviousNode = (unsigned long)Node;
	// Return the location of the last node.
	return (struct Node *)(((unsigned long)Node)+(sizeof(struct Node))+Node->Size);
}

struct MemoryManagement::Node *MemoryManagement::NodeManager::CreateNewNode(unsigned long Size , MemoryManagement::ALIGNMENT Alignment) {
	MemoryManagement::NodeManager *NodeManager = (MemoryManagement::NodeManager *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
	unsigned long PreviousNode = 0x00;
	struct Node *Node = SearchNewNodeLocation(&(PreviousNode));	// New node address : CurrentAddress
	// break
	Node = AlignNode(Node , Alignment , PreviousNode);
	if((Alignment != NO_ALIGN) && (NodeManager->StartNode->Using == 0)) {
		NodeManager->StartNode = Node;
		NodeManager->CurrentNode = Node;
		// printf("Adjusting node from alignment\n");
	}
	// Next node : Offset + Size of the node + Size of the node structure
	WriteNodeData(Node , 1 , Size , Alignment , 0x00 , PreviousNode);
	Node = NodeManager->AdjustNode(Node);
	/*printf("NewNode : 0x%X , Previous : 0x%X" , Node , PreviousNode);
	if(Alignment != NO_ALIGN) {
		printf(" , Align : %d" , Alignment);
	}
	printf("\n");*/
	return Node;
}

/// @brief Write the data to the node. 
/// @param Node 
/// @param Using 
/// @param Size 
/// @param NextNode 
/// @param PreviousNode 
void MemoryManagement::NodeManager::WriteNodeData(struct Node *Node , unsigned char Using , unsigned long Size , MemoryManagement::ALIGNMENT Alignment , unsigned long NextNode , unsigned long PreviousNode) {
	Node->Using = Using;
	if(PreviousNode != 0xFFFFFFFFFFFFFFFF) { // auto
		Node->Previous = (struct Node *)PreviousNode;
	}
	if(NextNode != 0xFFFFFFFFFFFFFFFF) { // auto
		Node->Next = (struct Node *)NextNode;
	}
	if(Size != 0x00) {
		Node->Size = Size;
	}
	if(Node == this->StartNode) {					// For the first node, the previous node shouldn't be exist.
		Node->Previous = 0;												// Set PreviousNode to zero
	}
	if(Node->Previous != 0x00) {
		Node->Previous->Next = Node;
	}
	if(Node->Next != 0x00) {
		Node->Next->Previous = Node;
	}
	Node->Signature = MEMORYMANAGEMENT_SIGNATURE;   // Write a signature to mark that it's a valid node
	Node->IsAligned = (Alignment == NO_ALIGN) ? 0 : 1;
}

int MemoryManagement::NodeManager::IsNodeInUnusableMemory(struct Node *Node , MemoryManagement::QuerySystemAddressMap *ViolatedMemory) {
	int i;
	int ViolatedMemoryCount = 0;
	for(i = 0; i < UnusableMemoryEntryCount; i++) {
		if(IsMemoryInside(UnusableMemories[i].Address , UnusableMemories[i].Length , (unsigned long)Node , Node->Size) == true) {
			memcpy(&(ViolatedMemory[ViolatedMemoryCount++]) , &(UnusableMemories[i]) , sizeof(QuerySystemAddressMap));
			printf("Pre-violation detected!(0x%X~0x%X), Writing..\n" , UnusableMemories[i].Address , UnusableMemories[i].Address+UnusableMemories[i].Length);
		}
	}
	return ViolatedMemoryCount;
}

void MemoryManagement::NodeManager::AddUnusableMemory(unsigned long StartAddress , unsigned long MemorySize) {
	UnusableMemories[UnusableMemoryEntryCount].Address = StartAddress;
	UnusableMemories[UnusableMemoryEntryCount].Length = MemorySize;
	UnusableMemories[UnusableMemoryEntryCount].Type = MEMORYMANAGEMENT_E820_RESERVED;
	UnusableMemoryEntryCount++;
}

/// @brief Adjust location of the node, if it's in an incessible area (Which means it's in ViolatedMemoryList)
/// @param Node Pointer of the node to check whether the node violated the area
/// @return Adjusted location of the node, If it wasn't violating anything, return original address(=Node)
struct MemoryManagement::Node *MemoryManagement::NodeManager::AdjustNode(struct MemoryManagement::Node *Node) {
	// this function has the error haha
	int i;
	int ViolatedMemoryCount;
	unsigned long StartAddress = 0;
	unsigned long EndAddress;
	QuerySystemAddressMap ViolatedMemoryList[UnusableMemoryEntryCount];
	memset(ViolatedMemoryList , 0 , sizeof(ViolatedMemoryList));
	// printf("Node->Size : %d\n" , Node->Size);
	if((ViolatedMemoryCount = IsNodeInUnusableMemory(Node , ViolatedMemoryList)) == 0) {
		// printf("Node is not in reserved memory\n");
		return Node;
	}
	StartAddress = ViolatedMemoryList[0].Address;
	EndAddress = ViolatedMemoryList[0].Address+ViolatedMemoryList[0].Length;
	for(i = 0; i < ViolatedMemoryCount; i++) {
		StartAddress = MIN(StartAddress , ViolatedMemoryList[i].Address);
		EndAddress = MAX(EndAddress , (ViolatedMemoryList[i].Address+ViolatedMemoryList[i].Length));
	}
	printf("Memory Violation Detected , Violated : %d\n" , ViolatedMemoryCount);
	printf("Location : 0x%X~0x%X\n" , StartAddress , EndAddress);
	printf("Violated : 0x%X~0x%X\n" , Node , ((unsigned long)Node)+Node->Size);
	return Node;
}

unsigned long MemoryManagement::AlignAddress(unsigned long Address , MemoryManagement::ALIGNMENT Alignment) {
	if(Alignment == NO_ALIGN) {
		return Address;
	}
	Address = (((unsigned long)(Address/Alignment))+1)*Alignment;
	return Address;
}

struct MemoryManagement::Node *MemoryManagement::NodeManager::AlignNode(struct MemoryManagement::Node *Node , MemoryManagement::ALIGNMENT Alignment , unsigned long PreviousNode) {
	unsigned long AlignedAddress = 0;   // PreviousNodeAddress : Previous Node Address before aligning to 4K
	unsigned long OriginalAddress = (unsigned long)Node;
	if(Alignment == NO_ALIGN) {
//		printf("No alignment\n");
		return Node;
	}
	AlignedAddress = AlignAddress((((unsigned long)Node)+sizeof(struct Node)) , Alignment); // Get the aligned address
	((struct Node *)(AlignedAddress-sizeof(struct Node)))->Previous = Node->Previous; // Write previous node information to new aligned node
	
	Node = (struct Node *)(AlignedAddress-sizeof(struct Node));							      // Relocate node to aligned address
	if(OriginalAddress != (unsigned long)this->StartNode) {
		Node->Previous = (struct Node *)PreviousNode;
		Node->Previous->Next = (struct Node *)(AlignedAddress-sizeof(struct Node));	  // Rewrite new node information(new aligned one)
	}
	else {
		Node->Previous = 0x00;
	}
	// printf("Node start address : 0x%X\n" , Node);
	// printf("Previous Node      : 0x%X\n" , Node->Previous);
	return Node;
}

bool MemoryManagement::IsMemoryInside(unsigned long Source , unsigned long SourceLength , unsigned long Target , unsigned long TargetLength) {
	if(((Source <= Target) && (Target <= (Source+SourceLength)))
	|| ((Source <= (Target+TargetLength)) && ((Target+TargetLength) <= Source+SourceLength))) {
		return true;
	}
	return false;
}

void MemoryManagement::CheckNodeCorruption(void) {
    MemoryManagement::NodeManager *NodeManager = (MemoryManagement::NodeManager *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
    struct MemoryManagement::Node *NodePointer = NodeManager->StartNode;

    while(NodePointer->Next != 0x00) {
        if((NodePointer->Signature != MEMORYMANAGEMENT_SIGNATURE)
		||((unsigned long)NodePointer->Next <= MEMORYMANAGEMENT_MEMORY_STARTADDRESS)
		||(((((unsigned long)NodePointer->Next)-(unsigned long)NodePointer)-sizeof(struct MemoryManagement::Node)) > NodePointer->Size)
		||(NodePointer->Next <= NodePointer)) {
			if(NodePointer->IsAligned == 1) {
				NodePointer = NodePointer->Next;
				continue;
			}
            printf("Node corruption detected!!!\n");
            printf("Corrupted Node : 0x%X(Next : 0x%X) , Size : %d\n" , NodePointer , NodePointer->Next , NodePointer->Size);
            __asm__ ("cli");
            while(1) {
                ;
            }
        }
        NodePointer = NodePointer->Next;
    }
}