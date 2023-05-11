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

void operator delete(void *Pointer) {
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
	struct Node *UsableNode;			// UsableNode : The node that is seperated
	if(Node->NextNode != 0) {			// If the next node is present, set TotalNodeSize to the size of searched node.
		TotalNodeSize = Node->NextNode-((unsigned long)Node)-sizeof(struct Node);	// TotalNodeSize : size of the current node
	}
	if(Node == 0) { // If we have to create new node, create new node at the end of the segments
		Node = NodeManager->CreateNewNode(Size , Alignment);
		if(Node == 0x00) {
			return 0x00;
		}
		TotalNodeSize = 0;								// Set the value to 0 so that the <Node Seperation Sequence> can't be executed.
	}
	else {	// Using already existing node
		if(Alignment != NO_ALIGN) {
			// Search New alignable location
			// printf("Alignment : %d\n" , Alignment);
			Node = (struct Node *)NodeManager->SearchAlignedNode(Size , Alignment);
			// printf("Found aligned, new one : 0x%X\n" , Node);
			if(Node == 0) {
				Node = NodeManager->CreateNewNode(Size , Alignment);
			}
		}
		NodeManager->WriteNodeData(Node , 1 , Size , (((unsigned long)Node)+Size+sizeof(struct Node)));
		Node = NodeManager->AdjustNode(Node);
		NodeManager->CurrentAddress = Node->NextNode; // Update CurrentAddress
	}
	// <Node Seperation Sequence>
	if(TotalNodeSize > Size) {	// If the size of the given segment is bigger than needed,
								// Seperate the bigger segment to needed size, and leave the left segment available
		UsableNode = (struct Node *)Node->NextNode;						// UsableNode : Not being used, used for another allocation
		NodeManager->WriteNodeData(UsableNode , 0 , Size , (((unsigned long)UsableNode)+(TotalNodeSize-Size)+sizeof(struct Node)));
	}
	// Return the actual available address : Node address + size of the node structure
	for(Address = ((unsigned long)Node+sizeof(struct Node)); Address < ((unsigned long)Node+sizeof(struct Node))+Size; Address += 8) {
		*((unsigned long *)Address) = 0x00;
	}
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

void MemoryManagement::Free(void *Address) {
	unsigned long CurrentNode;
	unsigned long NextNode;
	unsigned long PreviousNode;
	// Load the NodeManager from the local address
	MemoryManagement::NodeManager *NodeManager = (MemoryManagement::NodeManager *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
	if((unsigned long)Address < NodeManager->StartAddress) {
		printf("Deallocation Error #0 : Low Memory Address\n");
		return;
	}
	struct Node *Node = (struct Node *)(((unsigned long)Address)-sizeof(struct Node));  // Address of Node : Address - Size of the node structure
	if((Node->Using == 0)||(Node->Signature != MEMORYMANAGEMENT_SIGNATURE)) {			// If Node is not using, or not present, print error and leave.
		printf("Deallocation Error #1 : Not Allocated Memory\n");
		return;
	}
	// Allocated Size : Location of the next node - Location of current node
	// If the previous node is usable, and present, the node can be merged.
	// (Why are we merging and seperating the segment? Because, it can reduce the problem of external fragmentation)
	if((Node->PreviousNode != 0x00) && (((struct Node *)Node->PreviousNode)->Using == 0) && (((struct Node *)Node->PreviousNode)->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
		CurrentNode = (unsigned long)Node;				// CurrentNode : Saves the current node for later
		while((!(((struct Node *)Node->PreviousNode)->Using)) && (Node->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
			// Search the nodes that is available for merging, and erase all usable node to make a free space 
			// -> until we find already using node, or not present node.

			Node = (struct Node *)Node->PreviousNode; 	// Head to previous node 
			if(Node == 0) {							  	// If current node is zero, that means we reached the beginning of the memory.
				break;
			}
		}
		NodeManager->WriteNodeData(Node , 0 , 0 , ((struct Node *)CurrentNode)->NextNode);
		memset((struct Node *)CurrentNode , 0 , sizeof(struct Node));			// Erase the last current node
	}
	// If next node is usable, and present, the node can be merged.
	if((((struct Node *)Node->NextNode)->Using == 0) && (((struct Node *)Node->NextNode)->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
		CurrentNode = (unsigned long)Node;					// CurrentNode : Saves the current node for later
		Node = (struct Node *)Node->NextNode; 				// Save the current node, and move to next node
		while((!(((struct Node *)Node->NextNode)->Using)) && (Node->Signature == MEMORYMANAGEMENT_SIGNATURE)) {
			NextNode = Node->NextNode;						// Save the location of next node, because the node is going to be erased, 
															// and we need the location of it for merging.
			Node = (struct Node *)Node->NextNode; 			// Go to the next node and keep search
		}
		// Done erasing : Modify the next node location to the end of the node(It's going to be using node).
		NodeManager->WriteNodeData(((struct Node *)CurrentNode) , 0 , 0 , NextNode); // Free the node
		NodeManager->LastFreedAddress = CurrentNode;		// Modify LastFreedAddress to find node slightly more efficient.
	}
	else { // Not mergable, just set the flag to usable and modify LastFreedAddress.
		Node->Using = 0;
		NodeManager->LastFreedAddress = (unsigned long)Node;
	}
	// If the first node is usable, and there is no next nodes, then the node will be removed.
	// But, if the first node is being used, or there is next nodes, then the node is not going to be removed.
	Node = (struct Node *)NodeManager->StartAddress;
	if((Node->Using == 0) && (((struct Node *)Node->NextNode)->Signature != MEMORYMANAGEMENT_SIGNATURE)) { // If it sooths the condition,
		memset(Node , 0 , sizeof(struct Node));		  // Erase the node(Set everything to 0)
		NodeManager->CurrentAddress = NodeManager->StartAddress;	// Reset the address so that
		NodeManager->LastFreedAddress = NodeManager->StartAddress;  // Next allocation will be started at StartAddress
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
	struct Node *Node = (struct Node *)this->StartAddress;	// Start from StartAddress, 
	while(Node->Signature == MEMORYMANAGEMENT_SIGNATURE) {
		printf("%02d:%d|%d " , i++ , Node->Using , Node->NextNode-(unsigned long)Node-sizeof(struct Node));
		Node = (struct Node *)Node->NextNode;				// Print the information of the node, until the node runs out.
	}
	printf("\n");
}

// Description : Initializes the variables
void MemoryManagement::NodeManager::Initialize(unsigned long StartAddress , unsigned long TotalUsableMemory , QuerySystemAddressMap *E820 , int E820EntryCount) {
	int i;
	this->StartAddress = StartAddress; 				// StartAddress 	 : The location of the memory pool
	this->CurrentAddress = StartAddress;			// CurrentAddress    : The location of current position
	this->LastFreedAddress = 0;						// LastFreedAddress  : The location of lastly freed segment, 0 = There's yet no freed segment
	this->TotalUsableMemory = TotalUsableMemory;	// TotalUsableMemory : Total available memory to use(Size of the memory pool)
	printf("Unusable Memories List Location : 0x%X\n" , UnusableMemories);
	for(i = UnusableMemoryEntryCount = 0; i < E820EntryCount; i++) {
		if(E820[i].Type != MEMORYMANAGEMENT_E820_USABLE) { // If memory is not usable, put it into unusable memories list
			memcpy(&(UnusableMemories[UnusableMemoryEntryCount]) , &(E820[i]) , sizeof(QuerySystemAddressMap));
			UnusableMemoryEntryCount++;
		}
	}
}

// Description : Search the node that is bigger than the given size from the argument
unsigned long MemoryManagement::NodeManager::SearchReasonableNode(unsigned long Size) {
	struct Node *Node;
	if(this->CurrentAddress == this->StartAddress) {
		// printf("Returning Start of the memory\n");
		// If current address is start of the memory,
		return this->StartAddress;  // return the start address.
	}
	Node = (struct Node *)((this->LastFreedAddress == 0) ? this->CurrentAddress : this->LastFreedAddress);
	while(Node->Signature == MEMORYMANAGEMENT_SIGNATURE) {
		if((Node->Using == 0) && (GetNodeSize(Node) >= Size)) {
			// printf("Free Node Found : At 0x%08X, Size : %d\n" , Node , (Node->NextNode-(unsigned long)Node-sizeof(struct Node)));
			return (unsigned long)Node;
		}
		Node = (struct Node *)Node->NextNode;
	}
	return 0; // No node available, need to create new node
}

unsigned long MemoryManagement::NodeManager::SearchAlignedNode(unsigned long Size , MemoryManagement::ALIGNMENT Alignment) {
	struct Node *Node;
	unsigned long AlignedAddress;
	Node = (struct Node *)((this->LastFreedAddress == 0) ? this->CurrentAddress : this->LastFreedAddress);
	while(Node->Signature == MEMORYMANAGEMENT_SIGNATURE) {
		if(Node->Using == 0) {
			AlignedAddress = AlignAddress(((unsigned long)Node)+sizeof(struct Node) , Alignment);
			if(AlignedAddress >= Node->NextNode) {
				continue;
			}
			if(Node->Size >= Size) {
				// printf("Found node , alignable : 0x%X->0x%X\n" , Node , AlignedAddress);
				// printf("Required Size : %d\n" , Size);
				// printf("Calculated Size(A,I) : %d\n" , (((Node->NextNode-AlignedAddress)-sizeof(struct Node))));
				return (unsigned long)(AlignedAddress-sizeof(struct Node));
			}
		}
		Node = (struct Node *)Node->NextNode;
	}
	return 0; // No node available, need to create new node
}

// Description : Search the location for new node
unsigned long MemoryManagement::NodeManager::SearchNewNodeLocation(void) {
	struct Node *Node;
	if(this->CurrentAddress == this->StartAddress) {
		// printf("Returning Start of the memory\n");
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

struct MemoryManagement::Node *MemoryManagement::NodeManager::CreateNewNode(unsigned long Size , MemoryManagement::ALIGNMENT Alignment) {
	MemoryManagement::NodeManager *NodeManager = (MemoryManagement::NodeManager *)MEMORYMANAGEMENT_MEMORY_STARTADDRESS;
	struct Node *Node = (struct Node *)SearchNewNodeLocation();	// New node address : CurrentAddress
	Node = AlignNode(Node , Alignment);
	if(Alignment != NO_ALIGN) {
		// printf("Aligned Node  : 0x%X\n" , Node);
		// printf("Start Address : 0x%X\n" , ((unsigned long)Node)+sizeof(struct Node));
	}
	// Next node : Offset + Size of the node + Size of the node structure
	WriteNodeData(Node , 1 , Size , (((unsigned long)Node)+Size+sizeof(struct Node)));
	Node = NodeManager->AdjustNode(Node);
	Node->NextNode = (((unsigned long)Node)+Size+sizeof(struct Node));
	return Node;
}

void MemoryManagement::NodeManager::WriteNodeData(struct Node *Node , unsigned char Using , unsigned long Size , unsigned long NextNode , unsigned long PreviousNode) {
	Node->Using = Using;
	if(PreviousNode != 0xFFFFFFFFFFFFFFFF) {
		Node->PreviousNode = PreviousNode;
	}
	if(NextNode != 0xFFFFFFFFFFFFFFFF) {
		Node->NextNode = NextNode;
	}
	if(Size != 0x00) {
		Node->Size = Size;
	}
	if((unsigned long)Node == this->StartAddress) {					// For the first node, the previous node shouldn't be exist.
		Node->PreviousNode = 0;												// Set PreviousNode to zero
	}
	((struct Node *)Node->NextNode)->PreviousNode = (unsigned long)Node;
	Node->Signature = MEMORYMANAGEMENT_SIGNATURE;   // Write a signature to mark that it's a valid node
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

struct MemoryManagement::Node *MemoryManagement::NodeManager::AdjustNode(struct MemoryManagement::Node *Node) {
	int i;
	int ViolatedMemoryCount;
	unsigned long StartAddress = 0;
	unsigned long EndAddress;
	QuerySystemAddressMap ViolatedMemoryList[UnusableMemoryEntryCount];
	memset(ViolatedMemoryList , 0 , sizeof(ViolatedMemoryList));
	//printf("Node->Size : %d\n" , Node->Size);
	if((ViolatedMemoryCount = IsNodeInUnusableMemory(Node , ViolatedMemoryList)) == 0) {
		//printf("Node is not in reserved memory\n");
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
	while((Address & (Alignment-1)) != 0x00) {
		Address += 1;
	}
	return Address;
}

struct MemoryManagement::Node *MemoryManagement::AlignNode(struct MemoryManagement::Node *Node , MemoryManagement::ALIGNMENT Alignment) {
	unsigned long AlignedAddress = 0;   // PreviousNodeAddress : Previous Node Address before aligning to 4K
	AlignedAddress = AlignAddress((((unsigned long)Node)+sizeof(struct Node)) , Alignment); // Get the aligned address
	if(Alignment != NO_ALIGN) {
		((struct Node *)(AlignedAddress-sizeof(struct Node)))->PreviousNode = Node->PreviousNode; // Write previous node information to new aligned node
		Node = (struct Node *)(AlignedAddress-sizeof(struct Node));							      // Relocate node to aligned address
		((struct Node *)Node->PreviousNode)->NextNode = (AlignedAddress-sizeof(struct Node));	  // Rewrite new node information(new aligned one)
		// printf("Node start address     : 0x%X\n" , Node);
	}
	return Node;
}

unsigned long MemoryManagement::GetNodeSize(struct MemoryManagement::Node *Node) {
	return ((Node->NextNode-((unsigned long)Node))-sizeof(struct Node));
} 

bool MemoryManagement::IsMemoryInside(unsigned long Source , unsigned long SourceLength , unsigned long Target , unsigned long TargetLength) {
	if(((Source <= Target) && (Target <= (Source+SourceLength)))
	|| ((Source <= (Target+TargetLength)) && ((Target+TargetLength) <= Source+SourceLength))) {
		return true;
	}
	return false;
}