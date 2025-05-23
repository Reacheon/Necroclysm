#include <SDL3/SDL.h>

export module checkCursor;

import globalVar;
import wrapVar;
import util;

export Point2 getTouchXY()
{
    return { static_cast<int>(event.tfinger.x * static_cast<double>(cameraW)),
             static_cast<int>(event.tfinger.y * static_cast<double>(cameraH)) };
}

export bool checkCursor(const SDL_Rect* rect)
{
    switch (option::inputMethod)
    {
    case input::mouse:
    {
        Point2 mouseCoord = { static_cast<int>(getMouseX()), static_cast<int>(getMouseY()) };
        if (mouseCoord.x >= rect->x && mouseCoord.x <= rect->x + rect->w &&
            mouseCoord.y >= rect->y && mouseCoord.y <= rect->y + rect->h)
        {
            return true;
        }
        break;
    }
    case input::touch:
    {
        Point2 touchCoord = getTouchXY();
        if (touchCoord.x >= rect->x && touchCoord.x <= rect->x + rect->w &&
            touchCoord.y >= rect->y && touchCoord.y <= rect->y + rect->h)
        {
            return true;
        }
        break;
    }
    }
    return false;
}
