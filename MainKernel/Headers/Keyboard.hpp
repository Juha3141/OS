#ifndef _KEYBOARD_HPP_
#define _KEYBOARD_HPP_

#include <Kernel.hpp>

namespace Kernel {
    namespace Keyboard {
        void InterruptHandler(void);
        void MainInterruptHandler(void);
    }
}

#endif