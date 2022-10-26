//////////////////////////////////////////////////////////////////////////////////
// File "MemoryManagement.cpp"                                                  //
// Written by : Juha Cho                                                        //
// Started Date : 2022.10.18                                                    //
// Description : Contains //
//////////////////////////////////////////////////////////////////////////////////

#include <MemoryManagement.hpp>

//#define DEBUG

unsigned long SystemStructureLocation;

void Kernel::MemoryManagement::Initialize(void) {
	int i = 0;
	int j;
	unsigned int ID;
	unsigned long TotalUsableMemory = 0;
	Kernel::MemoryManagement::NodeManager *NodeManager;
	QuerySystemAddressMap *E820 = (QuerySystemAddressMap *)MEMORYMANAGEMENT_E820_ADDRESS;
	NodeManager = (Kernel::MemoryManagement::NodeManager*)(SystemStructureLocation
	              = Kernel::SystemStructure::Allocate(sizeof(Kernel::MemoryManagement::NodeManager) , &(ID)));
	Kernel::printf("Node Manager Location : 0x%X\n" , SystemStructureLocation);
	while(1) {
		if((E820[i].Type <= 0)||(E820[i].Type > 5)) {
			break;
		}
		if((E820[i].Type != 1) && (E820[i].Address >= MEMORYMANAGEMENT_MEMORY_STARTADDRESS)) {
			Kernel::printf("Unusable Memory : 0x%016X~0x%016X\n" , E820[i].Address , E820[i].Address+E820[i].Length);
		}
		else {
			TotalUsableMemory += E820[i].Length;
		}
		i++;
	}
	TotalUsableMemory -= MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
	NodeManager->Initialize(MEMORYMANAGEMENT_MEMORY_STARTADDRESS , TotalUsableMemory);
	Kernel::printf("Total usable memory : %dMB\n" , TotalUsableMemory/1024/1024);
	Kernel::printf("Memory pool address : 0x%X\n" , MEMORYMANAGEMENT_MEMORY_STARTADDRESS);

	unsigned long Address[20] = {0 , };
	Kernel::printf("Testing allocation/disallocation sequence... ");
	for(j = 0; j < 10; j++) { // 2GB memory test
		for(i = 0; i < 8; i++) {
			Kernel::MemoryManagement::Free((void*)Address[i]);
		}
		for(i = 0; i < 8; i++) {
			Address[i] = (unsigned long)Kernel::MemoryManagement::Allocate(256*1024*1024);
		}
	}
	NodeManager->MapNode();
	Kernel::printf("Done\n");
}

void *Kernel::MemoryManagement::Allocate(unsigned long Size) {
	unsigned long TotalNodeSize = 0;	// TotalNodeSize : Size of the total node that is going to be used for allocation
	// Load the NodeManager from the local address
	Kernel::MemoryManagement::NodeManager *NodeManager = (Kernel::MemoryManagement::NodeManager *)SystemStructureLocation;
	if(Size == 0) {
		Kernel::printf("Allocation Error #0 : Zero allocated size");
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
		*/
		TotalNodeSize = 0;		// Set the value to 0 so that the <Node Seperation Sequence> can't be executed.
		
		Node->Using = 1;            // Set the using flag to 1
		Node->Signature = MEMORYMANAGEMENT_SIGNATURE;   // Write a signature to mark it's the valid node
		// Next node : Offset + Size of the node + Size of the node structure
		Node->NextNode = (((unsigned long)Node)+Size+sizeof(struct Node));
		((struct Node *)Node->NextNode)->PreviousNode = (unsigned long)Node;
		if((unsigned long)Node == NodeManager->StartAddress) {
			Node->PreviousNode = 0;
		}
	}
	else {
		Node->Using = 1;            			      // Set the using flag to 1
		Node->Signature = MEMORYMANAGEMENT_SIGNATURE;				      // Write a signature to mark it's the valid node
		Node->NextNode = (((unsigned long)Node)+Size+sizeof(struct Node));
		((struct Node *)Node->NextNode)->PreviousNode = (unsigned long)Node;
		if((unsigned long)Node == NodeManager->StartAddress) {
			Node->PreviousNode = 0;
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
		UsableNode = (struct Node *)Node->NextNode;	// UsableNode : Not being used, used for another allocation
		UsableNode->Using = 0;						// Not using(Available)
		UsableNode->Signature = MEMORYMANAGEMENT_SIGNATURE;				// Mark a signature to make it valid
		// NextNode : Address of the current node(UsableNode) + Size of the unused area(TotalNodeSize-Size) + Size of the node structure
		UsableNode->NextNode = (((unsigned long)UsableNode)+(TotalNodeSize-Size)+sizeof(struct Node));
		((struct Node *)UsableNode->NextNode)->PreviousNode = (unsigned long)UsableNode;
	}
#ifdef DEBUG
	Kernel::printf("Updated CurrentAddress : 0x%X\n" , NodeManager->CurrentAddress);
#endif
	// Return the actual available address : Node address + size of the node structure
	return ((void *)((unsigned long)Node+sizeof(struct Node)));
}

void Kernel::MemoryManagement::Free(void *Address) {
	unsigned long CurrentNode;
	unsigned long NextNode;
	unsigned long PreviousNode;
	// Load the NodeManager from the local address
	Kernel::MemoryManagement::NodeManager *NodeManager = (Kernel::MemoryManagement::NodeManager *)SystemStructureLocation;
	if((unsigned long)Address < NodeManager->StartAddress) {
		Kernel::printf("Disallocation Error #0 : Low Memory Address\n");
		return;
	}
	struct Node *Node = (struct Node *)(((unsigned long)Address)-sizeof(struct Node));  // Address of Node : Address - Size of the node structure
	if((Node->Using == 0)||(Node->Signature != MEMORYMANAGEMENT_SIGNATURE)) {			// If Node is not using, or not present, print error and leave.
		Kernel::printf("Disallocation Error #1 : Not Allocated Memory\n");
		return;
	}
	// Allocated Size : Location of the next node - Location of current node
#ifdef DEBUG
	Kernel::printf("Allocated Size : %d\n" , Node->NextNode-((unsigned long)Address));
#endif

	if((Node->PreviousNode != 0x00) && (((struct Node *)Node->PreviousNode)->Using == 0) && (((struct Node *)Node->PreviousNode)->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
#ifdef DEBUG
		Kernel::printf("Previous area is mergable\n");
#endif
		CurrentNode = (unsigned long)Node;
		while((!(((struct Node *)Node->PreviousNode)->Using)) && (Node->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
#ifdef DEBUG
			Kernel::printf("0x%X\n" , Node->PreviousNode);
#endif
			Node = (struct Node *)Node->PreviousNode;
			if(Node == 0) {
				break;
			}
		}
		Node->Using = 0;
		Node->NextNode = ((struct Node *)CurrentNode)->NextNode;
		((struct Node *)Node->NextNode)->PreviousNode = (unsigned long)Node;
		Node->Signature = MEMORYMANAGEMENT_SIGNATURE;
		memset((struct Node *)CurrentNode , 0 , sizeof(struct Node));
	}
	// If next node is usable, and present, the node can be merged.
	// (Why are we merging and seperating the segment? Because, it can reduce the problem of external fragmentation)
	if((((struct Node *)Node->NextNode)->Using == 0) && (((struct Node *)Node->NextNode)->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
#ifdef DEBUG
		Kernel::printf("Allocated area is mergable\n");
#endif
		CurrentNode = (unsigned long)Node;					// CurrentNode : Saves the current node for later
		Node = (struct Node *)Node->NextNode; 				// Save the current node, and move to next node
		while((!(((struct Node *)Node->NextNode)->Using)) && (Node->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
			// Search the nodes that is available for merging, and erase all usable node to merge them
			// until we found already using node, or not present node.
			NextNode = Node->NextNode;						// Save the location of next node, because the node is going to be erased, 
															// and we need the location of it for merging.
			memset(((struct Node *)NextNode) , 0 , sizeof(struct Node));	// Erase the node to allocate something.
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
		asiduhasd
#endif
		memset(Node , 0 , sizeof(struct Node));		  // Erase the node(Set everything to 0)
		NodeManager->CurrentAddress = NodeManager->StartAddress;	// Reset the address so that
		NodeManager->LastFreedAddress = NodeManager->StartAddress;  // Next allocation will be started at StartAddress
	}
}

void Kernel::MemoryManagement::NodeManager::MapNode(void) {
	int i = 0;
	struct Node *Node = (struct Node *)this->StartAddress;
	while(Node->Signature == MEMORYMANAGEMENT_SIGNATURE) {
		Kernel::printf("%02d:%d|%d " , i++ , Node->Using , Node->NextNode-(unsigned long)Node-sizeof(struct Node));
		Node = (struct Node *)Node->NextNode;
	}
	Kernel::printf("\n");
}

void Kernel::MemoryManagement::NodeManager::Initialize(unsigned long StartAddress , unsigned long TotalUsableMemory) {
	this->StartAddress = StartAddress; 				// StartAddress 	 : The location of the memory pool
	this->CurrentAddress = StartAddress;			// CurrentAddress    : The location of current position
	this->LastFreedAddress = 0;						// LastFreedAddress  : The location of lastly freed segment, 0 = There's yet no freed segment
	this->TotalUsableMemory = TotalUsableMemory;	// TotalUsableMemory : Total available memory to use(Size of the memory pool)
}

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

unsigned long Kernel::MemoryManagement::NodeManager::SearchNewNodeLocation(void) {
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
		Node = (struct Node *)Node->NextNode;
	}
	return (unsigned long)Node;
}