#pragma once

#include <KeyCodes.h>

namespace Engine
{

    class KeyEvent
    {
    public:
        enum KeyState
        {
            Released = 0,
            Pressed = 1
        };

        KeyEvent(KeyCode::Key key, KeyState state) : Key(key),
                                                     State(state)
        {
        }

        KeyCode::Key Key;
        KeyState State;
    };

} // namespace Engine