export module drawWindowArrow;

import textureVar;
import globalVar;
import drawSprite;

export void drawWindowArrow(int inputX, int inputY, int dir)
{
	int arrowX, arrowY;
	int delta = 0;
	//120
	if (timer::timer600 % 30 < 5) { delta = 0; }
	else if (timer::timer600 % 30 < 10) { delta = 2; }
	else if (timer::timer600 % 30 < 15) { delta = 3; }
	else if (timer::timer600 % 30 < 20) { delta = 2; }
	else { delta = 0; }

	switch (dir)
	{
		default:
			arrowX = inputX + delta;
			arrowY = inputY;
			break;
		case 1:
			arrowX = inputX;
			arrowY = inputY - delta;
			break;
		case 2:
			arrowX = inputX - delta;
			arrowY = inputY;
			break;
		case 3:
			arrowX = inputX;
			arrowY = inputY + delta;
	}
	drawSpriteCenter(spr::windowArrow, dir, arrowX, arrowY);
}