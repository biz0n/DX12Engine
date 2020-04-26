#pragma once

#include "App.h"
#include "Events.h"

#include <memory>

class IGame : public std::enable_shared_from_this<IGame>
{
public:
    IGame(App *app);
    virtual ~IGame();

    virtual bool Initialize() = 0;

    virtual void Update(const Timer &time) = 0;

    virtual void Draw(const Timer &time) = 0;

    virtual void Deinitialize() = 0;

    virtual void Resize(int32 width, int32 height) {}

    virtual void KeyPressed(KeyEvent keyEvent) {}

protected:
    App &Graphics() { return *mApp; }

private:
    App *mApp;
};