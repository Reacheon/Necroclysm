#include <SDL.h>

export module checkCursor;

import globalVar;
import util;
/**
 *  @brief 입력한 좌표가 해당 사각형에 들어가는지 반환합니다.
 *  @param rect 체크할 사각형, 버튼 등을 넣으면 된다.
 *  @return 성공할 경우 true, 실패하면 false 반환
 */

export Point2 getMouseXY()
{
	int x, y;
	SDL_GetMouseState(&x, &y);
	return {x, y };
}

export Point2 getTouchXY()
{
	return { (int)(event.tfinger.x * (double)cameraW), (int)(event.tfinger.y * (double)cameraH) };
}

export bool checkCursor(const SDL_Rect* rect)
{
	switch (option::inputMethod)
	{
		case input::mouse:
		{
			Point2 mouseCoord = getMouseXY();
			if (mouseCoord.x >= rect->x && mouseCoord.x <= rect->x + rect->w)
			{
				if (mouseCoord.y >= rect->y && mouseCoord.y <= rect->y + rect->h)
				{
					return true;
				}
			}
			break;
		}
		case input::touch:
		{
			Point2 touchCoord = getTouchXY();
			if (touchCoord.x >= rect->x && touchCoord.x <= rect->x + rect->w)
			{
				if (touchCoord.y >= rect->y && touchCoord.y <= rect->y + rect->h)
				{
					return true;
				}
			}
			break;
		}
	}

	return false;
}
