#ifndef _PIT_H_
#define _PIT_H_

#include <Kernel.hpp>
#include <EssentialLibrary.hpp>
#include <ExceptionHandlers.hpp>

#define PIT_CONVERT_MS_TO_HZ(MS)  (((MS)*1193182)/1000)
#define PIT_CONVERT_US_TO_HZ(US)  (((US)*1193182)/1000000)
#define PIT_CHANNEL0_DATA         0x40
#define PIT_CHANNEL1_DATA         0x41
#define PIT_CHANNEL2_DATA         0x42
#define PIT_MODE_COMMAND_REGISTER 0x43

#define PIT_COUNTER0 0x00
#define PIT_COUNTER1 0x01
#define PIT_COUNTER2 0x02

//////////////////////////////////////////////////////////////////////////////////////
// <Mode/Command Register Structure (0x43)>                                         //
// +------+----------------------+------------------+------------------+            //
// |      |    Operating Mode    |    Access Mode   |  Select Channel  |            //
// +------+----------------------+------------------+------------------+            //
// 0  |   1          2         3 4                5 6                7              //
//    +-> BCD/Binary Mode                                                           //
// BCD/Binary Mode : 0 : 16bit binary Mode                                          //
//                   1 : Four-Digit BCD Mode                                        //
// Operating Mode  : 0(0b000) : Mode 0 (Interrupt on terminal count)                //
//                   1(0b001) : Mode 1 (Hardware re-triggerable one-shot)           //
//                   2(0b010) : Mode 2 (Rate generator)                             //
//                   3(0b011) : Mode 3 (Square wave generator)                      //
//                   4(0b100) : Mode 4 (Software triggered strobe)                  //
//                   5(0b101) : Mode 5 (Hardware triggered strobe)                  //
//                   6(0b110) : Mode 2 (Same as 0x02, Rate generator)               //
//                   7(0b111) : Mode 3 (Same as 0x03 , Square wave generator)       //
// Access Mode     : 0(0b00)  : Latch count value command                           //
//                   1(0b01)  : Access Mode : Low byte only                         //
//                   2(0b10)  : Access Mode : High byte only                        //
//                   3(0b11)  : Access Mode : Low/High byte                         //
// Select Channel  : 0(0b00)  : Channel 0                                           //
//                   1(0b01)  : Channel 1                                           //
//                   2(0b10)  : Channel 1                                           //
//                   3(0b11)  : Read-back Command                                   //
//////////////////////////////////////////////////////////////////////////////////////

namespace Kernel {
    namespace PIT {
        void Initialize(void);
        unsigned short ReadPITCounter(unsigned char CounterNumber);
        
        unsigned short GetCurrentPITFrequency(void);
        void DelayByPITCount(unsigned int PITCount);
        void DelayMilliseconds(unsigned int Milliseconds);
        void DelayMicroseconds(unsigned int Microseconds);
        
        void InterruptHandler(void);
        void MainInterruptHandler(void);
        
        unsigned long GetTickCount(void);
    }
}


#endif