#include <SDL3/SDL.h>

export module checkCursor;

import globalVar;
import util;

export Point2 getMouseXY()
{
    float x, y;
    SDL_GetMouseState(&x, &y);
    return { static_cast<int>(x), static_cast<int>(y) };
}

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
        Point2 mouseCoord = getMouseXY();
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
