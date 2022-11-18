#include <Graphics/Graphic.hpp>

static Graphics::LayerManager *GraphicLayerManager;

void Graphics::Initialize(void) {
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
    this->Elevation = GraphicLayerManager->AddLayer(this);
}

void Graphics::Layer::Move(int X , int Y) {
    /*
    
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
    GraphicLayerManager->UpdateArea(Coordinates);
    */
    GraphicLayerManager->Move(this , X , Y);
}

void Graphics::Layer::DrawPixel(int X , int Y , unsigned int Color) {
    if((X >= Coordinates.X2-Coordinates.X1)||(Y >= Coordinates.Y2-Coordinates.Y1)) {
        return;
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
	static char String[256];
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
					this->DrawPixel(CurrentX+i , CurrentY , Color);
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

void Graphics::LayerManager::Move(Layer *TargetLayer , int NewX , int NewY) {
    int i;
    int X;
    int Y;
    int X1;
    int Y1;
    int X2;
    int Y2;
    int Width;
    int Height;
    int LayerWidth = TargetLayer->Coordinates.X2-TargetLayer->Coordinates.X1;
    int LayerHeight = TargetLayer->Coordinates.Y2-TargetLayer->Coordinates.Y1;
    struct Rectangle OldCoordinates;
    struct Rectangle OverlappedArea;
    struct Rectangle ExtraOverlappedArea1;
    struct Rectangle ExtraOverlappedArea2;
    int NotDraw = false;
    memcpy(&(OldCoordinates) , &(TargetLayer->Coordinates) , sizeof(struct Rectangle));
    TargetLayer->Coordinates.X1 = NewX;
    TargetLayer->Coordinates.Y1 = NewY;
    TargetLayer->Coordinates.X2 = NewX+LayerWidth;
    TargetLayer->Coordinates.Y2 = NewY+LayerHeight;
    // OverlappedArea : Overlapped area of OldCoordinates and CurrentCoordintes
    if(GetOverlappedArea(OldCoordinates , TargetLayer->Coordinates , &(OverlappedArea)) == false) {
        OverlappedArea.X1 = -1;
        OverlappedArea.Y1 = -1;
        OverlappedArea.X2 = -1;
        OverlappedArea.Y2 = -1;
    }
    if(TargetLayer->Elevation+1 != MaxLayerIndex) {
        // If it's not the top layer, and it's hidden by higher layers
        // both Current coordinates and old coordinates
        // Do not draw the layers
        // Check all higher layers
        // If the rectangles are in the higher layer, it doesn't need an update
        for(i = TargetLayer->Elevation+1; i < MaxLayerIndex; i++) {
            if((IsRectangleInside(TargetLayer->Coordinates , ((Layer *)Layers[i])->Coordinates) == 1)
            && (IsRectangleInside(OldCoordinates , ((Layer *)Layers[i])->Coordinates) == 1)) {
                return;
            }
        }
    }
    for(Y = TargetLayer->Coordinates.Y1; Y <= TargetLayer->Coordinates.Y2; Y++) {
        for(X = TargetLayer->Coordinates.X1; X <= TargetLayer->Coordinates.X2; X++) {
            NotDraw = false;
            for(int j = TargetLayer->Elevation+1; j < MaxLayerIndex; j++) {
                if(IsCoordinateInside(X , Y , ((Layer *)Layers[j])->Coordinates) == true) {
                    NotDraw = true;
                    continue;
                }
            }
            if(NotDraw == true) {
                continue;
            }
            VBE::DrawPixel(X , Y , TargetLayer->VideoMemory[((Y-TargetLayer->Coordinates.Y1)*(LayerWidth))+(X-TargetLayer->Coordinates.X1)]);
        }
    }
    // update old coordinates : Problem
    for(i = 0; i < TargetLayer->Elevation; i++) {
        Width = ((Layer *)Layers[i])->Coordinates.X2-((Layer *)Layers[i])->Coordinates.X1;
        Height = ((Layer *)Layers[i])->Coordinates.Y2-((Layer *)Layers[i])->Coordinates.Y1;
        if((Width <= LayerWidth) && (Height <= LayerHeight)) {
            NotDraw = false;
            for(int j = i+1; j < MaxLayerIndex; j++) {
                if(IsRectangleInside(((Layer *)Layers[i])->Coordinates , ((Layer *)Layers[j])->Coordinates) == true) {
                    NotDraw = true;
                    continue;
                }
            }
            if(NotDraw == true) {
                continue;
            }
            if((IsRectangleInside(((Layer *)Layers[i])->Coordinates , OldCoordinates) == 1)||(IsRectangleInside(((Layer *)Layers[i])->Coordinates , TargetLayer->Coordinates) == 1)) {
                memset(&(ExtraOverlappedArea1) , 0 , sizeof(struct Rectangle));
                memset(&(ExtraOverlappedArea2) , 0 , sizeof(struct Rectangle));
            }
            else {
                GetOverlappedArea(((Layer *)Layers[i])->Coordinates , OldCoordinates , &(ExtraOverlappedArea1));
                GetOverlappedArea(((Layer *)Layers[i])->Coordinates , TargetLayer->Coordinates , &(ExtraOverlappedArea2));
            }
            for(Y = ((Layer *)Layers[i])->Coordinates.Y1; Y <= ((Layer *)Layers[i])->Coordinates.Y2; Y++) {
                for(X = ((Layer *)Layers[i])->Coordinates.X1; X <= ((Layer *)Layers[i])->Coordinates.X2; X++) {
                    if((IsCoordinateInside(X , Y , ExtraOverlappedArea1) == true)||(IsCoordinateInside(X , Y , ExtraOverlappedArea2) == true)) {
                        continue;
                    }
                    VBE::DrawPixel(X , Y , ((Layer *)Layers[i])->VideoMemory[((Y-(((Layer *)Layers[i])->Coordinates.Y1))*Width)+(X-(((Layer *)Layers[i])->Coordinates.X1))]);
                }
            }
            continue;
        }
        for(Y = OldCoordinates.Y1; Y <= OldCoordinates.Y2; Y++) {
            for(X = OldCoordinates.X1; X <= OldCoordinates.X2; X++) {
                if(((Y-((Layer *)Layers[i])->Coordinates.Y1) < 0)||((X-((Layer *)Layers[i])->Coordinates.X1) < 0)) {
                    continue;
                }
                NotDraw = false;
                for(int j = i+1; j < MaxLayerIndex; j++) {
                    if(IsCoordinateInside(X , Y , ((Layer *)Layers[j])->Coordinates) == true) {
                        NotDraw = true;
                        continue;
                    }
                }
                if(NotDraw == true) {
                    continue;
                }
                VBE::DrawPixel(X , Y , ((Layer *)Layers[i])->VideoMemory[((Y-(((Layer *)Layers[i])->Coordinates.Y1))*Width)+(X-(((Layer *)Layers[i])->Coordinates.X1))]);
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
    int IsInside = false;
    int IsNotDrawable = false;
    struct Rectangle OverlappingArea;
    struct Rectangle OverlappingArea2;
    for(i = 0; i < MaxLayerIndex; i++) {
        GetOverlappedArea(Area , ((Layer *)Layers[i])->Coordinates , &(OverlappingArea));

        GetOverlappedArea(((Layer *)Layers[i])->Coordinates , ((Layer *)Layers[i+1])->Coordinates , &(OverlappingArea2));
        if(IsRectangleInside(((Layer *)Layers[i])->Coordinates , ((Layer *)Layers[i+1])->Coordinates) == 1) {
            continue;
        }
        for(j = i+2; j < MaxLayerIndex; j++) {
            GetOverlappedArea(OverlappingArea2 , ((Layer *)Layers[j])->Coordinates , &(OverlappingArea2));
            if(IsRectangleInside(((Layer *)Layers[i])->Coordinates , ((Layer *)Layers[j])->Coordinates) == 1) {
                IsNotDrawable = true;
                break;
            }
        }
        if(IsNotDrawable == true) {
            continue;
        }

        IsInside = IsRectangleInside(Area , ((Layer *)Layers[i])->Coordinates);
        Width = ((Layer *)Layers[i])->Coordinates.X2-((Layer *)Layers[i])->Coordinates.X1;
        
        X1 = (IsInside != 0) ? OverlappingArea.X1 : ((Layer *)Layers[i])->Coordinates.X1;
        X2 = (IsInside != 0) ? OverlappingArea.X2 : ((Layer *)Layers[i])->Coordinates.X2;
        Y1 = (IsInside != 0) ? OverlappingArea.Y1 : ((Layer *)Layers[i])->Coordinates.Y1;
        Y2 = (IsInside != 0) ? OverlappingArea.Y2 : ((Layer *)Layers[i])->Coordinates.Y2;
        /*
        VBE::DrawText(10 , 10+(i*16*6) , 0xFFFFFF , 0xFF0000 , "Area : [(%d,%d) (%d,%d)]" , Area.X1 , Area.Y1 , Area.X2 , Area.Y2);
        VBE::DrawText(10 , 10+(16*1)+(i*16*6) , 0xFFFFFF , 0xFF0000 , "L%d   : [(%d,%d) (%d,%d)]" , i , ((Layer *)Layers[i])->Coordinates.X1 , ((Layer *)Layers[i])->Coordinates.Y1 , ((Layer *)Layers[i])->Coordinates.X2 , ((Layer *)Layers[i])->Coordinates.Y2);
        VBE::DrawText(10 , 10+(16*2)+(i*16*6) , 0xFFFFFF , 0xFF0000 , "1    : [(%d,%d) (%d,%d)]" , OverlappingArea.X1 , OverlappingArea.Y1 , OverlappingArea.X2 , OverlappingArea.Y2);
        VBE::DrawText(10 , 10+(16*3)+(i*16*6) , 0xFFFFFF , 0xFF0000 , "2    : [(%d,%d) (%d,%d)]" , OverlappingArea2.X1 , OverlappingArea2.Y1 , OverlappingArea2.X2 , OverlappingArea2.Y2);
        VBE::DrawText(10 , 10+(16*4)+(i*16*6) , 0xFFFFFF , 0xFF0000 , "upd  : [(%d,%d) (%d,%d)]" , X1 , Y1 , X2 , Y2);
        */
        for(Y = Y1; Y < Y2; Y++) {
            for(X = X1; X < X2; X++) {
				if(IsCoordinateInside(X , Y , Area) == false) {
					continue;
				}
                VBE::DrawPixel(X , Y , ((Layer *)Layers[i])->VideoMemory[((Y-(((Layer *)Layers[i])->Coordinates.Y1))*Width)+(X-(((Layer *)Layers[i])->Coordinates.X1))]);
            }
        }/*
        VBE::DrawRectangle(OverlappingArea2.X1 , OverlappingArea2.Y1 , OverlappingArea2.X2 , OverlappingArea2.Y1+1 , 0xFF00FF);
        VBE::DrawRectangle(OverlappingArea2.X1 , OverlappingArea2.Y1 , OverlappingArea2.X1+1 , OverlappingArea2.Y2 , 0xFF00FF);
        VBE::DrawRectangle(OverlappingArea2.X1-1 , OverlappingArea2.Y2 , OverlappingArea2.X2 , OverlappingArea2.Y2 , 0xFF00FF);
        VBE::DrawRectangle(OverlappingArea2.X2-1 , OverlappingArea2.Y1 , OverlappingArea2.X2 , OverlappingArea2.Y2 , 0xFF00FF);*/
	}
}

void Graphics::UpdateLayer(const Layer Layer) {
    GraphicLayerManager->UpdateArea(Layer.Coordinates);
}

bool Graphics::LayerManager::IsLayerOverlapped(const struct Rectangle Rectangle1 , const struct Rectangle Rectangle2) {
    // to-do list
    if(IsRectangleInside(Rectangle1 , Rectangle2) != 0) {
        return true;
    }
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
    && ((Rectangle1.X2 >= Rectangle2.X1) && (Rectangle1.X2 <= Rectangle2.X2))
    && ((Rectangle1.Y1 >= Rectangle2.Y1) && (Rectangle1.Y1 <= Rectangle2.Y2))
    && ((Rectangle1.Y2 >= Rectangle2.Y1) && (Rectangle1.Y2 <= Rectangle2.Y2))) {         // Is A is in B
        return 1;   // 1 = A is inside B
    }
    if(((Rectangle2.X1 >= Rectangle1.X1) && (Rectangle2.X1 <= Rectangle1.X2))
    && ((Rectangle2.X2 >= Rectangle1.X1) && (Rectangle2.X2 <= Rectangle1.X2))
    && ((Rectangle2.Y1 >= Rectangle1.Y1) && (Rectangle2.Y1 <= Rectangle1.Y2))
    && ((Rectangle2.Y2 >= Rectangle1.Y1) && (Rectangle2.Y2 <= Rectangle1.Y2))) {        // Is Rectangle2 is in Rectangle1 in terms of X position
        return 2;   // 2 = B is inside A
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
    if(IsRectangleInside(Rectangle1 , Rectangle2) == 1) {
        OverlappingArea->X1 = Rectangle1.X1;
        OverlappingArea->Y1 = Rectangle1.Y1;
        OverlappingArea->X2 = Rectangle1.X2;
        OverlappingArea->Y2 = Rectangle1.Y2;
        return true;
    }
    if(IsRectangleInside(Rectangle1 , Rectangle2) == 2) {
        OverlappingArea->X1 = Rectangle2.X1;
        OverlappingArea->Y1 = Rectangle2.Y1;
        OverlappingArea->X2 = Rectangle2.X2;
        OverlappingArea->Y2 = Rectangle2.Y2;
        return true;
    }
    OverlappingArea->X1 = MAX(Rectangle1.X1 , Rectangle2.X1);
    OverlappingArea->Y1 = MAX(Rectangle1.Y1 , Rectangle2.Y1);
    OverlappingArea->X2 = MIN(Rectangle1.X2 , Rectangle2.X2);
    OverlappingArea->Y2 = MIN(Rectangle1.Y2 , Rectangle2.Y2);
    return true;
}

bool Graphics::LayerManager::IsRectangleSame(const Rectangle Rectangle1 , const Rectangle Rectangle2) {
    if((Rectangle1.X1 == Rectangle2.X1)
    && (Rectangle1.Y1 == Rectangle2.Y1)
    && (Rectangle1.X2 == Rectangle2.X2)
    && (Rectangle1.Y2 == Rectangle2.Y2)) {
        return true;
    }
    return false;
}