#pragma once

#include <IO/KeyCodes.h>

namespace Engine
{
    class KeyEvent
    {
    public:
        enum class KeyState
        {
            Released = 0,
            Pressed = 1
        };

        KeyEvent(KeyCode::Key key, KeyState state) : key(key),
                                                     state(state)
        {
        }

        KeyCode::Key key;
        KeyState state;
    };

} // namespace Engine