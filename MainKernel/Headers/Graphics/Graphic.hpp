#ifndef _GRAPHIC_HPP_
#define _GRAPHIC_HPP_

#include <Kernel.hpp>
#include <Graphics/VESABIOSExtensions.hpp>

namespace Graphics {
    class Layer {
        public:

        private:
            
    };
    class LayerManager {
        friend class Layer;
        public:
            void Initialize(void);
            
            void AddLayer(void);
    };
}

#endif