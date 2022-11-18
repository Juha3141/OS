#ifndef _GRAPHIC_HPP_
#define _GRAPHIC_HPP_

#include <Kernel.hpp>
#include <Graphics/VESABIOSExtensions.hpp>

#define GRAPHICS_LAYERCOUNT 512

namespace Graphics {
    struct Rectangle {
        int X1;
        int Y1;
        int X2;
        int Y2;
    };
    struct Movement {
        int OldX;
        int OldY;
        int NewX;
        int NewY;
    };
    class Layer {
        friend class LayerManager;
        public:
            void Initialize(int InitialX , int InitialY , int Width , int Height);
            void Register(void);

            void Move(int X , int Y);

            void DrawPixel(int X , int Y , unsigned int Color);
            void DrawRectangle(int X1 , int Y1 , int X2 , int Y2 , unsigned int Color);
            void DrawText(int X , int Y , unsigned int Color , const char *Format , ...);

            unsigned long Elevation; // Elevation, and index
            
            struct Rectangle Coordinates;
        private:
            unsigned int *VideoMemory;
            
            struct Movement Movement;

            int Modified;
    };
    class LayerManager {
        friend class Layer;
        public:
            void Initialize(void);

            unsigned long AddLayer(Layer *Layer);
            void RemoveLayer(unsigned long LayerID);
            void UpdateArea(const struct Rectangle Area);

            void Move(Layer *Layer , int NewX , int NewY);
        private:
            bool IsLayerOverlapped(const struct Rectangle Rectangle1 , const struct Rectangle Rectangle2);
            int IsRectangleInside(const struct Rectangle Rectangle1 , const struct Rectangle Rectangle2);
            bool IsCoordinateInside(int X , int Y , const struct Rectangle Rectangle);
            bool GetOverlappedArea(const struct Rectangle Rectangle1 , const struct Rectangle Rectangle2 , struct Rectangle *OverlappingArea);
            bool IsRectangleSame(const struct Rectangle Rectangle1 , const struct Rectangle Rectangle2);

            unsigned char *VideoMemory;
            int ScreenWidth;
            int ScreenHeight;

            unsigned long *Layers;
            int LayerCount = 0;
            int MaxLayerIndex = 0;
    };
    void Initialize(void);

    void UpdateLayer(const Layer Layer);
    void UpdateAllLayers(void);

    void MoveToTop(Layer *Layer);
}

#endif