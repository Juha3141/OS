#include <Keyboard.hpp>

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

static Kernel::Keyboard::DataManager *KeyboardDataManager;

void Kernel::Keyboard::Initialize(void) {
    KeyboardDataManager = (Kernel::Keyboard::DataManager *)Kernel::MemoryManagement::Allocate(sizeof(Kernel::Keyboard::DataManager));
    KeyboardDataManager->Initialize();
    Kernel::printf("sizeof(Kernel::Keyboard::DataManager) = %d\n" , sizeof(Kernel::Keyboard::DataManager));
    IO::Write(0x64 , 0xAE);
    IO::Write(0x60 , 0xF4);

    if(IO::Read(0x60) == 0xFA) {
        Kernel::printf("0xFA received\n");
    }
    Kernel::printf("SpecialKeys Location : 0x%X\n" , (unsigned long)KeyboardDataManager->SpecialKeys);
    PIC::Unmask(33);
}

void Kernel::Keyboard::MainInterruptHandler(void) {
    unsigned char Data;
    unsigned char *VideoMemory = (unsigned char *)0xB8000;
    Data = IO::Read(0x60);
    if(KeyboardDataManager->ProcessSpecialKeys(Data) == 0) {
        if(Data < 0x80) {
            KeyboardDataManager->InsertDataToQueue(Data);
        }
    }
    VideoMemory[78*2] = Data;

    PIC::SendEOI(33);
}

void Kernel::Keyboard::DataManager::InsertDataToQueue(unsigned char ScanCode) {
    unsigned char Mode = 0;
    if((SpecialKeys[KEYBOARD_KEYLIST_RIGHTSHIFT] ==  1)||(SpecialKeys[KEYBOARD_KEYLIST_LEFTSHIFT] ==  1)) {
        Mode += 1;
    }
    if(SpecialKeys[KEYBOARD_KEYLIST_CAPSLOCK] ==  1) {
        Mode += 2;
    }
    this->ScanCodeQueue.Enqueue(ScanCodeInterpreter[Mode][ScanCode]);
}


int Kernel::Keyboard::DataManager::IsScanCodeQueueEmpty(void) {
    int Value;
    // To-do : Create MutEx
    Value = ScanCodeQueue.IsEmpty();
    return Value;
}


unsigned char Kernel::Keyboard::DataManager::GetScanCodeQueueData(void) { 
    // To-do : Create MutEx
    unsigned char Data;
    Data = ScanCodeQueue.Dequeue();
    return Data;
}

unsigned char Kernel::Keyboard::GetASCIIData(void) {
    unsigned char Data;
    while(1) {
        if(KeyboardDataManager->IsScanCodeQueueEmpty() != 1) {
            break;
        }
    }
    Data = KeyboardDataManager->GetScanCodeQueueData();
    return Data;
}

static int GetKeyListIndex(unsigned char ScanCode , char IsRight) {
    int i;
    unsigned char SpecialKeyScancodeList[2][11] = { 
        {KEYBOARD_SCANCODE_CAPSLOCK , 
        0 , 
        KEYBOARD_SCANCODE_CONTROL , 
        0 , 
        KEYBOARD_SCANCODE_NUMLOCK , 
        KEYBOARD_SCANCODE_LEFT_SHIFT , 
        KEYBOARD_SCANCODE_RIGHT_SHIFT , 
        KEYBOARD_SCANCODE_ALT , 
        0 , 
        0 , 
        KEYBOARD_SCANCODE_SUPER} , 
        {KEYBOARD_SCANCODE_CAPSLOCK , 
        0 , 
        0 , 
        KEYBOARD_SCANCODE_CONTROL , 
        KEYBOARD_SCANCODE_NUMLOCK , 
        KEYBOARD_SCANCODE_LEFT_SHIFT , 
        KEYBOARD_SCANCODE_RIGHT_SHIFT , 
        0 , 
        KEYBOARD_SCANCODE_ALT , 
        0 , 
        KEYBOARD_SCANCODE_SUPER}
    };
    if(ScanCode == 0) {
        return 0;
    }
    for(i = 0; i < 11; i++) {
        if((ScanCode == SpecialKeyScancodeList[IsRight][i])||(ScanCode == SpecialKeyScancodeList[IsRight][i]+0x80)) {
            return i;
        }
    }
    return -1;
}

char Kernel::Keyboard::DataManager::ProcessSpecialKeys(unsigned char ScanCode) {
    int i;
    int IsRight = 0;
    if(ScanCode == 0xE0) {
        SpecialKeys[KEYBOARD_KEYLIST_E] = 1;
        return 1;
    }
    if(SpecialKeys[KEYBOARD_KEYLIST_E] == 1) {
        SpecialKeys[KEYBOARD_KEYLIST_E] = 0;
        IsRight = 1;
    }
    /*
    if(ScanCode > 0x80) {
        if((i = GetKeyListIndex(ScanCode-0x80 , IsRight)) == 0) {
            return 0;
        }
        SpecialKeys[i] = 0;
        return 1;
    }
    else {*/
        if((i = GetKeyListIndex(ScanCode , IsRight)) == -1) {
            return 0;
        }
        if(i == KEYBOARD_KEYLIST_CAPSLOCK) {
            if(ScanCode < 0x80) {
                SpecialKeys[i] = (SpecialKeys[i] == 1) ? 0 : 1;
            }
            return 1;
        }
        SpecialKeys[i] = ((ScanCode < 0x80) ? 1 : 0);
        return 1;
}

char Kernel::Keyboard::IsQueueEmpty(void) {
    return KeyboardDataManager->ScanCodeQueue.IsEmpty();
}

char Kernel::Keyboard::IsSpecialKeyPressed(int SpecialKeyNumber) {
    return KeyboardDataManager->SpecialKeys[SpecialKeyNumber];
}