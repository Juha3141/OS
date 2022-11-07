#include <Graphics/VESABIOSExtensions.hpp>

struct Graphics::VBE::InfoStructure *Graphics::VBE::GetInfoStructure(void) {
    return (struct VBE::InfoStructure *)VBE_INFOSTRUCTURE_ADDRESS;
}

void Graphics::VBE::DrawPixel(int X , int Y , unsigned int Color) {
    struct VBE::InfoStructure *VBEInfoStructure = (struct VBE::InfoStructure *)VBE_INFOSTRUCTURE_ADDRESS;
    unsigned char *VideoMemory = (unsigned char *)VBEInfoStructure->Address;
    VideoMemory[((Y*VBEInfoStructure->Width)+X)*3] = Color & 0xFF;
    VideoMemory[(((Y*VBEInfoStructure->Width)+X)*3)+1] = (Color >> 8) & 0xFF;
    VideoMemory[(((Y*VBEInfoStructure->Width)+X)*3)+2] = (Color >> 16) & 0xFF;
}