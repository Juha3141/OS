#include <Graphics/Graphic.hpp>

static Graphics::LayerManager *GraphicLayerManager;

void Graphics::Initialize(void) {
    unsigned int ID;
    GraphicLayerManager = (Graphics::LayerManager *)Kernel::SystemStructure::Allocate(sizeof(Graphics::LayerManager));
    GraphicLayerManager->Initialize();
}

void Graphics::Layer::Initialize(int InitialX , int InitialY , int Width , int Height) {
    Coordinates.X1 = InitialX;
    Coordinates.X2 = InitialX+Width;
    Coordinates.Y1 = InitialY;
    Coordinates.Y2 = InitialY+Height;

    VideoMemory = (unsigned int *)Kernel::MemoryManagement::Allocate(Width*Height*sizeof(unsigned int));
}

void Graphics::Layer::Register(void) {
    this->ID = GraphicLayerManager->AddLayer(this);
}

void Graphics::Layer::Move(int X , int Y) {
    struct Rectangle OldCoordinates;
    int Width = Coordinates.X2-Coordinates.X1;
    int Height = Coordinates.Y2-Coordinates.Y1;
    memcpy(&(OldCoordinates) , &(Coordinates) , sizeof(struct Rectangle));
    Coordinates.X1 = X;
    Coordinates.X2 = X+Width;
    Coordinates.Y1 = Y;
    Coordinates.Y2 = Y+Height;
    Movement.NewX = X;
    Movement.NewY = Y;
    GraphicLayerManager->UpdateArea(OldCoordinates);
}

void Graphics::Layer::DrawPixel(int X , int Y , unsigned int Color) {
    if(X >= Coordinates.X2-Coordinates.X1) {
        X = Coordinates.X2-Coordinates.X1-1;
    }
    if(Y >= Coordinates.Y2-Coordinates.Y1) {
        Y = Coordinates.Y2-Coordinates.Y1-1;
    }
    VideoMemory[(Y*(Coordinates.X2-Coordinates.X1))+X] = Color;
}

void Graphics::Layer::DrawRectangle(int X1 , int Y1 , int X2 , int Y2 , unsigned int Color) {
    int X;
    int Y;
    for(Y = Y1; Y <= Y2; Y++) {
        for(X = X1; X <= X2; X++) {
            this->DrawPixel(X , Y , Color);
        }
    }
}

void Graphics::Layer::DrawText(int X , int Y , unsigned int Color , const char *Format , ...) {
	int i;
	int j;
	int k;
	va_list ap;
	int CurrentX;
	int CurrentY;
	unsigned char BitMask;
	int BitMaskStartAddress;
	char String[512];
    struct VBE::InfoStructure *VBEInfoStructure = (struct VBE::InfoStructure *)VBE_INFOSTRUCTURE_ADDRESS;
    unsigned char *VideoMemory = (unsigned char *)VBEInfoStructure->Address;
	unsigned char *Consolas_8x16_Bold = (unsigned char *)0x9C00;
	const int Width = 8;
	const int Height = 16;
	int HeightBackup;
	va_start(ap , Format);

	vsprintf(String , Format , ap);
	CurrentX = X;
	for(k = 0; k < strlen(String); k++) {
		CurrentY = Y;
		BitMaskStartAddress = String[k]*Height;
		for(j = 0; j < Height; j++) {
			BitMask = Consolas_8x16_Bold[BitMaskStartAddress++];
			for(i = 0; i < Width; i++) {
				if(BitMask & (0x01 << (Width-i-1))) {
					DrawPixel(CurrentX+i , CurrentY , Color);
				}
			}
			CurrentY += 1;
		}
		CurrentX += Width;
	}

	va_end(ap);
}

void Graphics::LayerManager::Initialize(void) {
    VideoMemory = (unsigned char *)VBE::GetInfoStructure()->Address;
    ScreenWidth = VBE::GetInfoStructure()->Width;
    ScreenHeight = VBE::GetInfoStructure()->Height;
    MaxLayerIndex = 0;
    Layers = (unsigned long *)Kernel::MemoryManagement::Allocate(sizeof(unsigned long)*GRAPHICS_LAYERCOUNT);
}

unsigned long Graphics::LayerManager::AddLayer(Layer *Layer) {
    Layers[MaxLayerIndex] = (unsigned long)Layer;
    return (MaxLayerIndex++);
}

void Graphics::LayerManager::UpdateLayer(unsigned long LayerID) { // !!!
    int i;
    int X;
    int Y;
    int Width;
    for(i = 0; i < MaxLayerIndex; i++) {
        Width = ((Layer *)Layers[i])->Coordinates.X2-((Layer *)Layers[i])->Coordinates.X1;
        for(Y = ((Layer *)Layers[i])->Coordinates.Y1; Y < ((Layer *)Layers[i])->Coordinates.Y2; Y++) {
            for(X = ((Layer *)Layers[i])->Coordinates.X1; X < ((Layer *)Layers[i])->Coordinates.X2; X++) {
                VBE::DrawPixel(X , Y , ((Layer *)Layers[i])->VideoMemory[((Y-((Layer *)Layers[i])->Coordinates.Y1)*(Width))+(X-((Layer *)Layers[i])->Coordinates.X1)]);
            }
        }
    }
}

void Graphics::LayerManager::UpdateArea(const struct Rectangle Area) { // !!!
    int i;
    int j;
    int X;
    int Y;
    int X1;
    int Y1;
    int X2;
    int Y2;
    int Width;
    int Height;
    int IsInside;
    struct Rectangle OverlappingArea;
    struct Rectangle OverlappingArea2;
    for(i = 0; i < MaxLayerIndex; i++) {
        if(GetOverlappedArea(Area , ((Layer *)Layers[i])->Coordinates , &(OverlappingArea)) == false) {
            continue;
        }
        
        IsInside = GetOverlappedArea(Area , ((Layer *)Layers[i])->Coordinates , &(OverlappingArea2));
        if(IsInside == false) {
            memcpy(&(OverlappingArea2) , &(Area) , sizeof(struct Rectangle));
        }
        for(j = i+1; j < MaxLayerIndex; j++) {
            GetOverlappedArea(OverlappingArea2 , ((Layer *)Layers[j])->Coordinates , &(OverlappingArea2));
        }
        IsInside = IsRectangleInside(Area , ((Layer *)Layers[i])->Coordinates);
        Width = ((Layer *)Layers[i])->Coordinates.X2-((Layer *)Layers[i])->Coordinates.X1;
        
        X1 = (IsInside != 0) ? OverlappingArea.X1 : ((Layer *)Layers[i])->Coordinates.X1;
        Y1 = (IsInside != 0) ? OverlappingArea.Y1 : ((Layer *)Layers[i])->Coordinates.Y1;
        X2 = (IsInside != 0) ? OverlappingArea.X2 : ((Layer *)Layers[i])->Coordinates.X2;
        Y2 = (IsInside != 0) ? OverlappingArea.Y2 : ((Layer *)Layers[i])->Coordinates.Y2;
        for(Y = Y1; Y < Y2; Y++) {
            for(X = X1; X < X2; X++) {
                VBE::DrawPixel(X , Y , ((Layer *)Layers[i])->VideoMemory[((Y-(((Layer *)Layers[i])->Coordinates.Y1))*Width)+(X-(((Layer *)Layers[i])->Coordinates.X1))]);
            }
        }
    }
}

void Graphics::UpdateLayer(Layer *Layer) {
    GraphicLayerManager->UpdateLayer(Layer->ID);
}

bool Graphics::LayerManager::IsLayerOverlapped(const struct Rectangle Rectangle1 , const struct Rectangle Rectangle2) {
    // to-do list
    if(((Rectangle2.Y1 >= Rectangle1.Y1) && (Rectangle2.Y1 <= Rectangle1.Y2))
     ||((Rectangle1.Y1 >= Rectangle2.Y1) && (Rectangle1.Y1 <= Rectangle2.Y2))) {
        if(((Rectangle2.X1 >= Rectangle1.X1) && (Rectangle2.X1 <= Rectangle1.X2))
         ||((Rectangle1.X1 >= Rectangle2.X1) && (Rectangle1.X1 <= Rectangle2.X2))) {
            return true;
        }
    }
		
    if(((Rectangle2.X1 >= Rectangle1.X1) && (Rectangle2.X1 <= Rectangle1.X2))
     ||((Rectangle1.X1 >= Rectangle2.X1) && (Rectangle1.X1 <= Rectangle2.X2))) {
	    if(((Rectangle2.Y1 >= Rectangle1.Y1) && (Rectangle2.Y1 <= Rectangle1.Y2))
        ||((Rectangle1.Y1 >= Rectangle2.Y1) && (Rectangle1.Y1 <= Rectangle2.Y2))) {
		    return true;
        }
    }
    return false;
}

int Graphics::LayerManager::IsRectangleInside(const struct Rectangle Rectangle1 , const struct Rectangle Rectangle2) {
    if(((Rectangle1.X1 >= Rectangle2.X1) && (Rectangle1.X1 <= Rectangle2.X2))
    && ((Rectangle1.Y1 >= Rectangle2.Y1) && (Rectangle1.Y1 <= Rectangle2.Y2))) {        // Is A is in B
        return 1;   // 1 = A is inside B
    }
    if(((Rectangle2.X1 >= Rectangle1.X1) && (Rectangle2.X2 <= Rectangle1.X2))
    && ((Rectangle2.Y1 >= Rectangle1.Y1) && (Rectangle2.Y2 <= Rectangle1.Y2))) {        // Is Rectangle2 is in Rectangle1 in terms of X position
        return 2;
    }
    return 0;
}

bool Graphics::LayerManager::IsCoordinateInside(int X , int Y , const struct Rectangle Rectangle) {
    if(((X >= Rectangle.X1) && (X <= Rectangle.X2)) && ((Y >= Rectangle.Y1) && (Y <= Rectangle.Y2))) {
        return true;
    }
    return false;
}

bool Graphics::LayerManager::GetOverlappedArea(const struct Rectangle Rectangle1 , const struct Rectangle Rectangle2 , struct Rectangle *OverlappingArea) {
    if(IsLayerOverlapped(Rectangle1 , Rectangle2) == false) {
        return false;
    }
    OverlappingArea->X1 = MAX(Rectangle1.X1 , Rectangle2.X1);
    OverlappingArea->Y1 = MAX(Rectangle1.Y1 , Rectangle2.Y1);
    OverlappingArea->X2 = MIN(Rectangle2.X2 , Rectangle2.X2);
    OverlappingArea->Y2 = MIN(Rectangle2.Y2 , Rectangle2.Y2);
    return true;
}