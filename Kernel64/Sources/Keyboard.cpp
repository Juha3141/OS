/////////////////////////////////////////////////////////////////////////////
// File "Keyboard.cpp"                                                     //
// Written by : Juha Cho                                                   // 
// Started date : 2022.11.03                                               //
// Description : Manages keyboard data from interrupt, from PS/2 Keyboard. //
// Also controls status of various special key(Like Shift, Ctrl ...)       //
/////////////////////////////////////////////////////////////////////////////

#include <Keyboard.hpp>

static Kernel::Keyboard::DataManager *KeyboardDataManager; // Management structure of the keyboard data

// The map of the PS/2 qwerty keyboard
// The first map is default keyboard map, and the second one is shifted keyboard map, 
// and the third one is capslocked keyboard map, and finally the last one is shifted and capslocked map.
unsigned char ScanCodeInterpreter[4][0x80] = { // normal , shift , capslock , capslock+shift
    {0x00 , KEYBOARD_KEY_ESC , '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , '\b' , 
    '\t' , 'q' , 'w' , 'e' , 'r' , 't' , 'y' , 'u' , 'i' , 'o' , 'p' , '[' , ']' , '\n' , KEYBOARD_KEY_LEFT_CONTROL , 
    'a' , 's' , 'd' , 'f' , 'g' , 'h' , 'j' , 'k' , 'l' , ';' , '\'' , '`' , KEYBOARD_KEY_LEFT_SHIFT , '\\' , 
    'z' , 'x' , 'c' , 'v' , 'b' , 'n' , 'm' , ',' , '.' , '/' , KEYBOARD_KEY_RIGHT_SHIFT , '*' , KEYBOARD_KEY_LEFT_ALT , 
    ' ' , KEYBOARD_KEY_CAPSLOCK , KEYBOARD_KEY_F1 , KEYBOARD_KEY_F2 , KEYBOARD_KEY_F3 , KEYBOARD_KEY_F4 , 
    KEYBOARD_KEY_F5 , KEYBOARD_KEY_F6 , KEYBOARD_KEY_F7 , KEYBOARD_KEY_F8 , KEYBOARD_KEY_F9 , KEYBOARD_KEY_F10 , 
    KEYBOARD_KEY_NUMLOCK , KEYBOARD_KEY_SCROLLLOCK , KEYBOARD_KEY_HOME , KEYBOARD_KEY_UP , KEYBOARD_KEY_PGUP , '-' , KEYBOARD_KEY_LEFT , '5' , KEYBOARD_KEY_RIGHT , '+' , KEYBOARD_KEY_END , KEYBOARD_KEY_DOWN , KEYBOARD_KEY_PGDOWN , KEYBOARD_KEY_INSERT , '.' , 
    0x00 , 0x00 , 0x00 , KEYBOARD_KEY_F11 , KEYBOARD_KEY_F12 , 0x00 , 0x00 , 0x00 , 0x00 , } , 
    {0x00 , KEYBOARD_KEY_ESC , '!' , '@' , '#' , '$' , '%' , '^' , '&' , '*' , '(' , ')' , '_' , '+' , '\b' , 
    '\t' , 'Q' , 'W' , 'E' , 'R' , 'T' , 'Y' , 'U' , 'I' , 'O' , 'P' , '{' , '}' , '\n' , KEYBOARD_KEY_LEFT_CONTROL , 
    'A' , 'S' , 'D' , 'F' , 'G' , 'H' , 'J' , 'K' , 'L' , ':' , '"' , '~' , KEYBOARD_KEY_LEFT_SHIFT , '|' , 
    'Z' , 'X' , 'C' , 'V' , 'B' , 'N' , 'M' , '<' , '>' , '?' , KEYBOARD_KEY_RIGHT_SHIFT , '*' , KEYBOARD_KEY_LEFT_ALT , 
    ' ' , KEYBOARD_KEY_CAPSLOCK , KEYBOARD_KEY_F1 , KEYBOARD_KEY_F2 , KEYBOARD_KEY_F3 , KEYBOARD_KEY_F4 , 
    KEYBOARD_KEY_F5 , KEYBOARD_KEY_F6 , KEYBOARD_KEY_F7 , KEYBOARD_KEY_F8 , KEYBOARD_KEY_F9 , KEYBOARD_KEY_F10 , 
    KEYBOARD_KEY_NUMLOCK , KEYBOARD_KEY_SCROLLLOCK , KEYBOARD_KEY_HOME , KEYBOARD_KEY_UP , KEYBOARD_KEY_PGUP , '-' , KEYBOARD_KEY_LEFT , '5' , KEYBOARD_KEY_RIGHT , '+' , KEYBOARD_KEY_END , KEYBOARD_KEY_DOWN , KEYBOARD_KEY_PGDOWN , KEYBOARD_KEY_INSERT , '.' , 
    0x00 , 0x00 , 0x00 , KEYBOARD_KEY_F11 , KEYBOARD_KEY_F12 , 0x00 , 0x00 , 0x00 , 0x00 , } , 
    {0x00 , KEYBOARD_KEY_ESC , '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , '\b' , 
    '\t' , 'Q' , 'W' , 'E' , 'R' , 'T' , 'Y' , 'U' , 'I' , 'O' , 'P' , '[' , ']' , '\n' , KEYBOARD_KEY_LEFT_CONTROL , 
    'A' , 'S' , 'D' , 'F' , 'G' , 'H' , 'J' , 'K' , 'L' , ';' , '\'' , '`' , KEYBOARD_KEY_LEFT_SHIFT , '\\' , 
    'Z' , 'X' , 'C' , 'V' , 'B' , 'N' , 'M' , ',' , '.' , '/' , KEYBOARD_KEY_RIGHT_SHIFT , '*' , KEYBOARD_KEY_LEFT_ALT , 
    ' ' , KEYBOARD_KEY_CAPSLOCK , KEYBOARD_KEY_F1 , KEYBOARD_KEY_F2 , KEYBOARD_KEY_F3 , KEYBOARD_KEY_F4 , 
    KEYBOARD_KEY_F5 , KEYBOARD_KEY_F6 , KEYBOARD_KEY_F7 , KEYBOARD_KEY_F8 , KEYBOARD_KEY_F9 , KEYBOARD_KEY_F10 , 
    KEYBOARD_KEY_NUMLOCK , KEYBOARD_KEY_SCROLLLOCK , KEYBOARD_KEY_HOME , KEYBOARD_KEY_UP , KEYBOARD_KEY_PGUP , '-' , KEYBOARD_KEY_LEFT , '5' , KEYBOARD_KEY_RIGHT , '+' , KEYBOARD_KEY_END , KEYBOARD_KEY_DOWN , KEYBOARD_KEY_PGDOWN , KEYBOARD_KEY_INSERT , '.' , 
    0x00 , 0x00 , 0x00 , KEYBOARD_KEY_F11 , KEYBOARD_KEY_F12 , 0x00 , 0x00 , 0x00 , 0x00 , } , 
    {0x00 , KEYBOARD_KEY_ESC , '!' , '@' , '#' , '$' , '%' , '^' , '&' , '*' , '(' , ')' , '_' , '+' , '\b' , 
    '\t' , 'q' , 'w' , 'e' , 'r' , 't' , 'y' , 'u' , 'i' , 'o' , 'p' , '{' , '}' , '\n' , KEYBOARD_KEY_LEFT_CONTROL , 
    'a' , 's' , 'd' , 'f' , 'g' , 'h' , 'j' , 'k' , 'l' , ':' , '"' , '~' , KEYBOARD_KEY_LEFT_SHIFT , '|' , 
    'z' , 'x' , 'c' , 'v' , 'b' , 'n' , 'm' , '<' , '>' , '?' , KEYBOARD_KEY_RIGHT_SHIFT , '*' , KEYBOARD_KEY_LEFT_ALT , 
    ' ' , KEYBOARD_KEY_CAPSLOCK , KEYBOARD_KEY_F1 , KEYBOARD_KEY_F2 , KEYBOARD_KEY_F3 , KEYBOARD_KEY_F4 , 
    KEYBOARD_KEY_F5 , KEYBOARD_KEY_F6 , KEYBOARD_KEY_F7 , KEYBOARD_KEY_F8 , KEYBOARD_KEY_F9 , KEYBOARD_KEY_F10 , 
    KEYBOARD_KEY_NUMLOCK , KEYBOARD_KEY_SCROLLLOCK , KEYBOARD_KEY_HOME , KEYBOARD_KEY_UP , KEYBOARD_KEY_PGUP , '-' , KEYBOARD_KEY_LEFT , '5' , KEYBOARD_KEY_RIGHT , '+' , KEYBOARD_KEY_END , KEYBOARD_KEY_DOWN , KEYBOARD_KEY_PGDOWN , KEYBOARD_KEY_INSERT , '.' , 
    0x00 , 0x00 , 0x00 , KEYBOARD_KEY_F11 , KEYBOARD_KEY_F12 , 0x00 , 0x00 , 0x00 , 0x00 , } , 
};

// Description : Initialize the data manager, and ps/2 keyboard hardware, and unmask the keyboard interrupt.
void Kernel::Keyboard::Initialize(void) {
    unsigned int ID;
    KeyboardDataManager = (Kernel::Keyboard::DataManager *)Kernel::SystemStructure::Allocate(sizeof(Kernel::Keyboard::DataManager));    // Allocate the system structure
    KeyboardDataManager->Initialize();/*
    IO::Write(0x64 , 0xAE); // Send the enable command to status register port
    while(1) {              // If Input buffer state bit in the status register port is 1,
                            // the keyboard yet did not take the data from the buffer.
        if(!(IO::Read(0x64) & 0b10)) {
            break;          // If the bit is not set, that means that the keyboard took the data from the buffer. 
        }
    }
    IO::Write(0x60 , 0xF4); // Send the enable command to input buffer port
    */
    //Kernel::PIC::Unmask(33);        // Unmask the keyboard interrupt
    // Kernel::printf("ACK Signal : 0x%X\n" , IO::Read(0x60));
}

// Description : Handler of the keyboard interrupt
void Kernel::Keyboard::MainInterruptHandler(void) {
    static int i = 0;
    unsigned char ScanCode;
    const unsigned char Spinner[4] = {'-' , '\\' , '|' , '/'};  // Spinner(for debugging purpose)
    unsigned char *VideoMemory = (unsigned char *)0xB8000;
    ScanCode = IO::Read(0x60);                                      // Stores the keyboard scancode data
    if(KeyboardDataManager->ProcessSpecialKeys(ScanCode) == 0) {    // If it's not special key
        if(ScanCode < 0x80) {
            // If the scancode is above 0x80, that means the key has been released, not pressed.
            // Likewise, if the scancode is below 0x80, that means the key has been pressed.
            KeyboardDataManager->InsertDataToQueue(ScanCode);       // the data goes into the keyboard queue.
        }
    }
    VideoMemory[78*2] = Spinner[i];     // Just a basic spinner in the screen
    i++;                                // If the interrupt is called, the spinner spins
    if(i >= 4) {
        i = 0;
    }

    LocalAPIC::SendEOI();
}

void Kernel::Keyboard::DataManager::InsertDataToQueue(unsigned char ScanCode) {
    unsigned char Mode = 0;
    if((SpecialKeys[KEYBOARD_KEYLIST_RIGHTSHIFT] ==  1)||(SpecialKeys[KEYBOARD_KEYLIST_LEFTSHIFT] ==  1)) { // If shift keyboard is being pressed, 
        Mode += 1;  // Increase the mode to 1
    }
    if(SpecialKeys[KEYBOARD_KEYLIST_CAPSLOCK] ==  1) {
        Mode += 2;  // Increase the mode to 2, if shift key is also pressed, then the mode is going to be 3(capslock+shift), 
                    // or, if shift key is not pressed, then the mode is going to be 2(only capslock)
    }
    this->ScanCodeQueue.Enqueue(ScanCodeInterpreter[Mode][ScanCode]);   // Put the data interpreted by the keyboard map
}

int Kernel::Keyboard::GetASCIIData(void) {
    int Data;
    while(1) {
        if(KeyboardDataManager->ScanCodeQueue.Dequeue(&(Data)) == true) {
            return Data;
        }
    }
}

// Description : To-do
static int GetKeyListIndex(unsigned char ScanCode , char IsRight) {
    int i;
    // IsRight : if it's 0 : Use left side table
    //                   1 : Use right side table
    // I actually made a table to set the flag more easily.
    unsigned char SpecialKeyScancodeList[2][11] = {
        // Table for left side special keys
        {KEYBOARD_SCANCODE_CAPSLOCK ,           // Index 0  : Capslock
        0 ,                                     // Index 1  : Insert(unused)
        KEYBOARD_SCANCODE_CONTROL ,             // Index 2  : Left Control
        0 ,                                     // Index 3  : Right Control(unused)
        KEYBOARD_SCANCODE_NUMLOCK ,             // Index 4  : Numlock
        KEYBOARD_SCANCODE_LEFT_SHIFT ,          // Index 5  : Left shift
        KEYBOARD_SCANCODE_RIGHT_SHIFT ,         // Index 6  : Right shift
        KEYBOARD_SCANCODE_ALT ,                 // Index 7  : Left Alt
        0 ,                                     // Index 8  : Right Alt(unused)
        0 ,                                     // Index 9  : E(unused)
        KEYBOARD_SCANCODE_SUPER} ,              // Index 10 : Super(Window key)
        // Table for right side special keys
        {KEYBOARD_SCANCODE_CAPSLOCK ,           // Index 0  : Capslock
        0 ,                                     // Index 1  : Insert(unused)
        0 ,                                     // Index 2  : Left Control(unused)
        KEYBOARD_SCANCODE_CONTROL ,             // Index 3  : Right Control
        KEYBOARD_SCANCODE_NUMLOCK ,             // Index 4  : Numlock
        KEYBOARD_SCANCODE_LEFT_SHIFT ,          // Index 5  : Left shift
        KEYBOARD_SCANCODE_RIGHT_SHIFT ,         // Index 6  : Right shift
        0 ,                                     // Index 7  : Left Alt(unused)
        KEYBOARD_SCANCODE_ALT ,                 // Index 8  : Right Alt
        0 ,                                     // Index 9  : E(unused)
        KEYBOARD_SCANCODE_SUPER}                // Index 10 : Super(Window key)
    };
    if(ScanCode == 0) {         // If it's invalid scancode,
        return -1;              // just return -1
    }
    for(i = 0; i < 11; i++) {   // Compare with the entire table
        // If the "pressed" scancode is identical to the following, as well as the "released" scancode,
        // return the index. The index is going to be used to set the flags, which in the SpecialKeys.
        if((ScanCode == SpecialKeyScancodeList[IsRight][i])||(ScanCode == SpecialKeyScancodeList[IsRight][i]+0x80)) {
            return i;           // Return the index
        }
    }
    return -1;                  // The scancode was not the special key.
}

// Description : Check if the scancode is a special key, and if it is, then change the flag of the special keys.
// Return 1 if the scancode is a special key, and return 0 if it isn't.
char Kernel::Keyboard::DataManager::ProcessSpecialKeys(unsigned char ScanCode) {
    int i;
    int IsRight = 0;        // Is the key is right sided?
    if(ScanCode == 0xE0) {  // If a scancode is transmitted with 0xE0, then the key is on the right side of the keyboard.
        SpecialKeys[KEYBOARD_KEYLIST_E] = 1;    // Set the E flag to 1
        return 1;
    }
    if(SpecialKeys[KEYBOARD_KEYLIST_E] == 1) {  // If the scancode is on the right side, 
        SpecialKeys[KEYBOARD_KEYLIST_E] = 0;    // Toggle the E value, 
        IsRight = 1;                            // and change the IsRight to 1
    }
    if((i = GetKeyListIndex(ScanCode , IsRight)) == -1) {
        return 0;           // If we failed to search the index of the scancode, return 0
    }
    if(i == KEYBOARD_KEYLIST_CAPSLOCK) {    // Capslock is exclusive, because it needs to be toggled, unlike the other special keys.
        if(ScanCode < 0x80) {
            SpecialKeys[i] = (SpecialKeys[i] == 1) ? 0 : 1; // Toggle the flag.
        }
        return 1;
    }
    SpecialKeys[i] = ((ScanCode < 0x80) ? 1 : 0);           // Set the flag, depending on the status of the key(pressed(<0x80)/released(>0x80))
    return 1;                                               // Job well done.
}

char Kernel::Keyboard::IsSpecialKeyPressed(int SpecialKeyNumber) {
    return KeyboardDataManager->SpecialKeys[SpecialKeyNumber];
}