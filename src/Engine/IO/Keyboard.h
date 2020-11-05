#pragma once

#include <IO/KeyEvent.h>
#include <io/KeyCodes.h>

#include <bitset>

namespace Engine
{
    class Keyboard
    {
        public:
            Keyboard();
            ~Keyboard();
        
        public:
            void KeyPressed(KeyEvent event);

            bool IsKeyPressed(KeyCode::Key key);

        private:
            std::bitset<0xFF> mKeyState;
    };
}