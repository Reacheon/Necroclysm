#include <SDL.h>

export module checkCursor;

import globalVar;
/**
 *  @brief 입력한 좌표가 해당 사각형에 들어가는지 반환합니다.
 *  @param rect 체크할 사각형, 버튼 등을 넣으면 된다.
 *  @return 성공할 경우 true, 실패하면 false 반환
 */
export bool checkCursor(const SDL_Rect* rect)
{
	switch (inputType)
	{
		case input::mouse:
		{
			int mouseX = event.motion.x;
			int mouseY = event.motion.y;
			if (mouseX >= rect->x && mouseX <= rect->x + rect->w)
			{
				if (mouseY >= rect->y && mouseY <= rect->y + rect->h)
				{
					return true;
				}
			}
			break;
		}
		case input::touch:
		{
			float touchX = event.tfinger.x * cameraW;
			float touchY = event.tfinger.y * cameraH;
			if (touchX >= rect->x && touchX <= rect->x + rect->w)
			{
				if (touchY >= rect->y && touchY <= rect->y + rect->h)
				{
					return true;
				}
			}
			break;
		}
		case input::keyboard:{return false;}
	}

	return false;
}
