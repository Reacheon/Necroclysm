#include <SDL3/SDL.h>

module globalVar;

import std;
import util;

float getMouseX()
{
    float px, py;
    SDL_GetMouseState(&px, &py);
    float renderX, renderY;
    SDL_RenderCoordinatesFromWindow(renderer, px, py, &renderX, &renderY);
    return renderX;
}

float getMouseY()
{
    float px, py;
    SDL_GetMouseState(&px, &py);
    float renderX, renderY;
    SDL_RenderCoordinatesFromWindow(renderer, px, py, &renderX, &renderY);
    return renderY;
}

Point2 getAbsMouseGrid()
{
    int cameraGridX, cameraGridY;
    if (cameraX >= 0) cameraGridX = cameraX / 16;
    else cameraGridX = -1 + cameraX / 16;
    if (cameraY >= 0) cameraGridY = cameraY / 16;
    else cameraGridY = -1 + cameraY / 16;

    int camDelX = cameraX - (16 * cameraGridX + 8);
    int camDelY = cameraY - (16 * cameraGridY + 8);

    int revX, revY, revGridX, revGridY;
    if (option::inputMethod == input::touch)
    {
        revX = static_cast<int>(event.tfinger.x * cameraW) - (cameraW / 2);
        revY = static_cast<int>(event.tfinger.y * cameraH) - (cameraH / 2);
    }
    else
    {
        revX = static_cast<int>(getMouseX()) - (cameraW / 2);
        revY = static_cast<int>(getMouseY()) - (cameraH / 2);
    }
    revX += sgn(revX) * (8 * zoomScale) + camDelX;
    revGridX = revX / (16 * zoomScale);
    revY += sgn(revY) * (8 * zoomScale) + camDelY;
    revGridY = revY / (16 * zoomScale);

    return { cameraGridX + revGridX, cameraGridY + revGridY };
}
