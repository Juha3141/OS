///////////////////////////////////////////////////////////////////////
// File "Queue.hpp"                                                  //
// Written by : Juha Cho                                             //
// Started Date : 2022.10.27                                         //
// Description : Contains circular queue system for various systems. //
// (Ex : Keyboard data queue , Mouse data queue.)                    //
///////////////////////////////////////////////////////////////////////

#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <MemoryManagement.hpp>

// Circular Queue (FIFO)
template <typename T> struct Queue {
    // CurrentOffset   : Offset which the data is going to be saved, 
    //                   The Offset increases when the data is stored.
    // SelectingOffset : Offset which the data is going to be returned
    //                   The Offset increases when the data is returned.
    // TotalLength     : The length of the queue
    void Initialize(unsigned int QueueSize) {   // Initializes Queue
        CurrentOffset = 0;
        SelectingOffset = 0;
        TotalLength = QueueSize;
        QueueList = (T *)Kernel::MemoryManagement::Allocate(QueueSize*sizeof(T));   // Allocate the data array
    }
    void DeleteQueue(void) {
        Kernel::MemoryManagement::Free(QueueList);  // 
    }
    bool Enqueue(T Data) {                          // Put data to queue
        // Kernel::printf("Enqueue\n");
        // Kernel::printf("Data : 0x%X\n" , Data);
        QueueList[CurrentOffset] = Data;            // Stores data
        CurrentOffset = (CurrentOffset+1)%TotalLength;
        // Kernel::printf("CurrentOffset : %d\n" , CurrentOffset);
        return true;
    }
    bool Dequeue(T *Data) {                               // Remove data from queue
        int i;
        if(IsEmpty() == true) {
            return false;
        }
        // Kernel::printf("Dequeue\n");
        // Kernel::printf("CurrentOffset : %d\n" , CurrentOffset);
        // Kernel::printf("SelectingOffset : %d\n" , SelectingOffset);
        *Data = QueueList[SelectingOffset];
        SelectingOffset = (SelectingOffset+1)%TotalLength;
        // Kernel::printf("NewSelectingOffset : %d\n" , SelectingOffset);
        // Kernel::printf("Data : 0x%X\n" , *Data);
        return true;
    }
    bool IsEmpty(void) {
        if(CurrentOffset == SelectingOffset) {      // If the CurrentOffset and SelectingOffset are same,
            return true;                            // which means all the data were returned, 
        }                                           // It is empty.
        return false;
    }
    T *QueueList;
    int CurrentOffset;
    int SelectingOffset;
    int TotalLength;
};

template <typename T> struct StructureQueue {
    void Initialize(unsigned int QueueSize) {   // Initializes Queue
        CurrentOffset = 0;                      // Initialize variables
        SelectingOffset = 0;
        TotalLength = QueueSize;
        QueueList = (T *)Kernel::MemoryManagement::Allocate(QueueSize*sizeof(T));   // Allocate the data array
    }
    void DeleteQueue(void) {
        Kernel::MemoryManagement::Free(QueueList);
    }
    bool Enqueue(T Data) {                          // Put data to queue
        memcpy(&(QueueList[CurrentOffset]) , &(Data) , sizeof(T));
        CurrentOffset += 1;
        CurrentOffset = (CurrentOffset+1)%TotalLength;
        // Going circular, if we reach the end, 
        // return back to the beginning
        return true;
    }
    bool Dequeue(T *Data) {                         // Remove data from queue
        if(this->IsEmpty() == true) {
            return false;
        }
        memcpy(Data , &(QueueList[SelectingOffset]) , sizeof(T));        // Returns data
        SelectingOffset += 1;
        SelectingOffset = (SelectingOffset+1)%TotalLength;  
        // Going circular, if we reach the end, 
        // return back to the beginning
        return true;
    }
    bool IsEmpty(void) {
        if(CurrentOffset == SelectingOffset) {      // If the CurrentOffset and SelectingOffset are same, 
            return true;                            // which means all the data were returned, 
        }                                           // It is empty.
        return false;
    }
    
    T *QueueList;
    int CurrentOffset;
    int SelectingOffset; 
    int TotalLength;
};

#endif