#ifndef _KEYBOARD_HPP_
#define _KEYBOARD_HPP_

#include <Kernel.hpp>
#include <EssentialLibrary.hpp>
#include <Queue.hpp>

#define KEYBOARD_KEY_ESC            0xC1
#define KEYBOARD_KEY_BACKSPACE      0xC2
#define KEYBOARD_KEY_TAB            0xC3
#define KEYBOARD_KEY_LEFT_CONTROL   0xC4
#define KEYBOARD_KEY_LEFT_SHIFT     0xC5
#define KEYBOARD_KEY_RIGHT_SHIFT    0xC6
#define KEYBOARD_KEY_LEFT_ALT       0xC7
#define KEYBOARD_KEY_CAPSLOCK       0xC8
#define KEYBOARD_KEY_F1             0xC9
#define KEYBOARD_KEY_F2             0xCA
#define KEYBOARD_KEY_F3             0xCB
#define KEYBOARD_KEY_F4             0xCC
#define KEYBOARD_KEY_F5             0xCD
#define KEYBOARD_KEY_F6             0xCE
#define KEYBOARD_KEY_F7             0xCF
#define KEYBOARD_KEY_F8             0xD0
#define KEYBOARD_KEY_F9             0xD1
#define KEYBOARD_KEY_F10            0xD2
#define KEYBOARD_KEY_F11            0xD3
#define KEYBOARD_KEY_F12            0xD4
#define KEYBOARD_KEY_NUMLOCK        0xD5
#define KEYBOARD_KEY_SCROLLLOCK     0xD6
#define KEYBOARD_KEY_HOME           0xD7
#define KEYBOARD_KEY_END            0xD8
#define KEYBOARD_KEY_UP             0xD9
#define KEYBOARD_KEY_DOWN           0xDA
#define KEYBOARD_KEY_RIGHT          0xDB
#define KEYBOARD_KEY_LEFT           0xDC
#define KEYBOARD_KEY_PGUP           0xDE
#define KEYBOARD_KEY_PGDOWN         0xDF
#define KEYBOARD_KEY_INSERT         0xE0

#define KEYBOARD_SCANCODE_CONTROL     0x1D
#define KEYBOARD_SCANCODE_ALT         0x38
#define KEYBOARD_SCANCODE_RIGHT_SHIFT 0x36
#define KEYBOARD_SCANCODE_LEFT_SHIFT  0x2A
#define KEYBOARD_SCANCODE_CAPSLOCK    0x3A
#define KEYBOARD_SCANCODE_NUMLOCK     0x45
#define KEYBOARD_SCANCODE_SUPER       0xDB

// <Special Key Table, macro version>
#define KEYBOARD_KEYLIST_CAPSLOCK     0
#define KEYBOARD_KEYLIST_INSERT       1
#define KEYBOARD_KEYLIST_LEFTCONTROL  2
#define KEYBOARD_KEYLIST_RIGHTCONTROL 3
#define KEYBOARD_KEYLIST_NUMLOCK      4
#define KEYBOARD_KEYLIST_LEFTSHIFT    5
#define KEYBOARD_KEYLIST_RIGHTSHIFT   6
#define KEYBOARD_KEYLIST_LEFTALT      7
#define KEYBOARD_KEYLIST_RIGHTALT     8
#define KEYBOARD_KEYLIST_E            9
#define KEYBOARD_KEYLIST_SUPER        10

namespace Kernel {
    namespace Keyboard {
        class DataManager {
            public:
                void Initialize(void) {
                    memset(SpecialKeys , 0 , 11);
                    ScanCodeQueue.Initialize(2048);
                }
                void InsertDataToQueue(unsigned char ScanCode);
                char ProcessSpecialKeys(unsigned char ScanCode);

                int IsScanCodeQueueEmpty(void);
                unsigned char GetScanCodeQueueData(void);
                Queue<unsigned char>ScanCodeQueue;
                /* 
                 * <Special Key Table>
                 * CapsLock     - Flag : SpecialKeys[0]
                 * Insert       - Flag : SpecialKeys[1]
                 * LeftControl  - Flag : SpecialKeys[2]
                 * RightControl - Flag : SpecialKeys[3]
                 * NumLock      - Flag : SpecialKeys[4]
                 * LeftShift    - Flag : SpecialKeys[5]
                 * RightShift   - Flag : SpecialKeys[6]
                 * LeftAlt      - Flag : SpecialKeys[7]
                 * RightAlt     - Flag : SpecialKeys[8]
                 * E(0x0E)      - Flag : SpecialKeys[9]
                 * Super        - Flag : SpecialKeys[10]
                 */
                // Stores status of the special keys, 1 is pressed, or on, 0 is not pressed or off.
                // Each array element stores each special keys - which is at the above table.
                char SpecialKeys[11];
            private:
                inline void ChangeFlags(unsigned char ScanCode);
        };
        void Initialize(void);
        unsigned char GetASCIIData(void);
        char IsQueueEmpty(void);
        char IsSpecialKeyPressed(int SpecialKeyNumber);
        
        void InterruptHandler(void);
        void MainInterruptHandler(void);
    }
}

#endif