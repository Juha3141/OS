#ifndef _TEXTSCREEN_HPP_
#define _TEXTSCREEN_HPP_

#define TEXTSCREEN_80x25_VIDEOMEMORY 0xB8000
#define TEXTSCREEN_80x25_WIDTH       80
#define TEXTSCREEN_80x25_HEIGHT      25
#define TEXTSCREEN_TABSIZE           4

namespace TextScreen80x25 {
    struct TextScreenInformation {
        unsigned char *VideoMemory;
        int Width;
        int Height;
        unsigned char BackgroundColor;
        unsigned char ForegroundColor;

        int X;
        int Y;
    };
    void Initialize(void);
}
void ClearScreen(unsigned char BackgroundColor , unsigned char ForegroundColor);
void printf(const char *Format , ...);
void PrintString(const char *String);

void SetPosition(int X , int Y);
void MovePosition(int RelativeX , int RelativeY);
void GetScreenInformation(int *X , int *Y , unsigned char *BackgroundColor , unsigned char *ForegroundColor);
unsigned char GetCharacter(int X , int Y);

#endif