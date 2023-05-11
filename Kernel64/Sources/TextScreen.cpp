#include <TextScreen.hpp>
#include <KernelSystemStructure.hpp>
#include <EssentialLibrary.hpp>

static TextScreen80x25::TextScreenInformation ScreenInfo;

void TextScreen80x25::Initialize(void) {
    ScreenInfo.VideoMemory = (unsigned char *)TEXTSCREEN_80x25_VIDEOMEMORY;
    ScreenInfo.Width = TEXTSCREEN_80x25_WIDTH;
    ScreenInfo.Height = TEXTSCREEN_80x25_HEIGHT;
    ScreenInfo.BackgroundColor = 0x00;
    ScreenInfo.ForegroundColor = 0x07;
    ClearScreen(0x00 , 0x07);
}

void ClearScreen(unsigned char BackgroundColor , unsigned char ForegroundColor) {
    int i;
    unsigned char Color = ((BackgroundColor & 0x0F) << 4)|(ForegroundColor & 0x0F);
    for(i = 0; i < ScreenInfo.Width*ScreenInfo.Height*2;) {
        ScreenInfo.VideoMemory[i] = ' ';
        ScreenInfo.VideoMemory[i+1] = Color;
        i += 2;
    }
    ScreenInfo.BackgroundColor = BackgroundColor;
    ScreenInfo.ForegroundColor = ForegroundColor;
    ScreenInfo.X = 0;
    ScreenInfo.Y = 0;
}

void PrintString(const char *String) {
    int i;
    int j;
    int Offset;
    for(i = 0; i < strlen(String); i++) {
        Offset = (ScreenInfo.Y*ScreenInfo.Width*2)+ScreenInfo.X*2;
        switch(String[i]) {
            case '\n':
                ScreenInfo.X = 0;
                ScreenInfo.Y++;
                if(ScreenInfo.Y > 24) {
                    memcpy(ScreenInfo.VideoMemory , ScreenInfo.VideoMemory+(ScreenInfo.Width*2) , ScreenInfo.Width*(ScreenInfo.Height-1)*2);
                    for(j = ScreenInfo.Width*(ScreenInfo.Height-1)*2; j < ScreenInfo.Width*ScreenInfo.Height*2; j += 2) {
                        ScreenInfo.VideoMemory[j] = 0x00;
                        ScreenInfo.VideoMemory[j+1] = (ScreenInfo.BackgroundColor << 4)+ScreenInfo.ForegroundColor;
                    }
                    ScreenInfo.Y = 24;
                }
                Offset = (ScreenInfo.Y*ScreenInfo.Width*2)+ScreenInfo.X*2;
                break;
            case '\b':
                ScreenInfo.X -= 1;
                if(ScreenInfo.X < 0) {
                    ScreenInfo.X = 0;
                }
                ScreenInfo.VideoMemory[(ScreenInfo.Y*ScreenInfo.Width*2)+ScreenInfo.X*2] = 0x00;
                break;
            case '\r':
                ScreenInfo.X = 0;
                break;
            case '\t':
                ScreenInfo.X += TEXTSCREEN_TABSIZE;
                break;
            default:
                ScreenInfo.VideoMemory[Offset] = String[i];
                ScreenInfo.VideoMemory[Offset+1] = (ScreenInfo.BackgroundColor << 4)+ScreenInfo.ForegroundColor;
                ScreenInfo.X++;
                if(ScreenInfo.X > 79) {
                    ScreenInfo.X = 0;
                    ScreenInfo.Y++;
                    if(ScreenInfo.Y > 24) { // fix scrolling problem
                        memcpy(ScreenInfo.VideoMemory , ScreenInfo.VideoMemory+(ScreenInfo.Width*2) , ScreenInfo.Width*(ScreenInfo.Height-1)*2);
                        for(j = ScreenInfo.Width*(ScreenInfo.Height-1)*2; j < ScreenInfo.Width*ScreenInfo.Height*2; j += 2) {
                            ScreenInfo.VideoMemory[j] = 0x00;
                            ScreenInfo.VideoMemory[j+1] = (ScreenInfo.BackgroundColor << 4)+ScreenInfo.ForegroundColor;
                        }
                        ScreenInfo.Y = 24;
                    }
                    Offset = (ScreenInfo.Y*ScreenInfo.Width*2)+ScreenInfo.X*2;
                }
                break;
        }
    }
}

void printf(const char *Format , ...) {
    va_list ap;
    char String[512];
    va_start(ap , Format);
    vsprintf(String , Format , ap);
    PrintString(String);
    va_end(ap);
}

void SetPosition(int X , int Y) {
    if(X >= ScreenInfo.Width-1) {
        X = ScreenInfo.Width-1;
    }
    if(Y >= ScreenInfo.Height-1) {
        Y = ScreenInfo.Height-1;
    }
    ScreenInfo.X = X;
    ScreenInfo.Y = Y;
}

void MovePosition(int X , int Y) {
    ScreenInfo.X -= X;
    ScreenInfo.Y -= Y;
    if(ScreenInfo.X < 0) {
        ScreenInfo.X = 0;
    }
    if(ScreenInfo.Y < 0) {
        ScreenInfo.Y = 0;
    }
    if(ScreenInfo.X >= ScreenInfo.Width-1) {
        ScreenInfo.X = ScreenInfo.Width-1;
    }
    if(ScreenInfo.Y >= ScreenInfo.Height-1) {
        ScreenInfo.Y = ScreenInfo.Height-1;
    }
}

void GetScreenInformation(int *X , int *Y , unsigned char *BackgroundColor , unsigned char *ForegroundColor) {
    *X = ScreenInfo.X;
    *Y = ScreenInfo.Y;
    *BackgroundColor = ScreenInfo.BackgroundColor;
    *ForegroundColor = ScreenInfo.ForegroundColor;
}