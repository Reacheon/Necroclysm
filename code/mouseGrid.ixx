export module mouseGrid;

import globalVar;
import util;
import std;

//카메라에 대한 상대좌표
export std::array<int,2> getRevMouseGrid()
{
	int revX, revY, revGridX, revGridY;
	if (option::inputMethod == input::touch)
	{
		revX = event.tfinger.x * cameraW - (cameraW / 2);
		revY = event.tfinger.y * cameraH - (cameraH / 2);
	}
	else
	{
		revX = event.motion.x - (cameraW / 2);
		revY = event.motion.y - (cameraH / 2);
	}
	revX += sgn(revX) * (8 * zoomScale);
	revGridX = revX / (16 * zoomScale);
	revY += sgn(revY) * (8 * zoomScale);
	revGridY = revY / (16 * zoomScale);

	return { revGridX,revGridY };
}

//현재 마우스의 절대좌표
export Point2 getAbsMouseGrid()
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
		revX = event.tfinger.x * cameraW - (cameraW / 2);
		revY = event.tfinger.y * cameraH - (cameraH / 2);
	}
	else
	{
		revX = event.motion.x - (cameraW / 2);
		revY = event.motion.y - (cameraH / 2);
	}
	revX += sgn(revX) * (8 * zoomScale) + camDelX;
	revGridX = revX / (16 * zoomScale);
	revY += sgn(revY) * (8 * zoomScale) + camDelY;
	revGridY = revY / (16 * zoomScale);

	return { cameraGridX + revGridX, cameraGridY + revGridY };
}