#include "Keyboard.h"

namespace Engine
{
    Keyboard::Keyboard() = default;
    Keyboard::~Keyboard() = default;

    void Keyboard::KeyPressed(KeyEvent event)
    {
         // Pitushok
         mKeyState[event.Key] = (event.State == KeyEvent::KeyState::Pressed);
    }

    bool Keyboard::IsKeyPressed(KeyCode::Key key)
    {
        return mKeyState[key];
    }
} // namespace Engine