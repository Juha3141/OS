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
template <typename T> class Queue {
    public:
        // CurrentOffset   : Offset which the data is going to be saved, 
        //                   The Offset increases when the data is stored.
        // SelectingOffset : Offset which the data is going to be returned
        //                   The Offset increases when the data is returned.
        // TotalLength     : The length of the queue
        void Initialize(unsigned int QueueSize) {   // Initializes Queue
            CurrentOffset = 0;                      // Initialize variables
            SelectingOffset = 0;
            TotalLength = QueueSize;
            QueueList = (T *)Kernel::MemoryManagement::Allocate(QueueSize*sizeof(T));   // Allocate the data array
        }
        void DeleteQueue(void) {
            Kernel::MemoryManagement::Free(QueueList);  // 
        }
        bool Enqueue(T Data) {                          // Put data to queue
            QueueList[CurrentOffset] = Data;            // Stores data
            CurrentOffset += 1;
            if(CurrentOffset > TotalLength) {           // Going circular, if we reach the end, 
                CurrentOffset = 0;                      // return back to the beginning
            }
            return true;
        }
        T Dequeue(void) {                               // Remove data from queue
            T Data = QueueList[SelectingOffset];        // Returns data
            SelectingOffset += 1;
            if(SelectingOffset > TotalLength) {         // Going circular, if we reach the end, 
                SelectingOffset = 0;                    // return back to the beginning
            }
            return Data;
        }
        bool IsEmpty(void) {
            if(CurrentOffset == SelectingOffset) {      // If the CurrentOffset and SelectingOffset are same, 
                return true;                            // which means all the data were returned, 
            }                                           // It is empty.
            return false;
        }
        bool IsFull(void) {
            if(CurrentOffset == TotalLength) {          // If the CurrentOffset and TotalLength are same, 
                return true;                            // which means all the data filled the array 
            }                                           // It is full.
            return false;
        }
    private:
        T *QueueList;
        int CurrentOffset;
        int SelectingOffset;
        int TotalLength;
};

#endif