#include <SDL.h>

import Loot;
import globalVar;
import constVar;
import textureVar;
import util;
import Sprite;
import Player;
import drawWindow;
import drawSprite;
import checkCursor;
import drawText;
import drawItemList;

void Loot::drawGUI()
{
	if (getStateDraw() == false) { return; }

	const Uint8* state = SDL_GetKeyboardState(NULL);
	Sprite* targetBtnSpr = nullptr;

	drawWindow(&lootBase, sysStr[10], 1);

	//����
	{
		//drawStadium(pocketWindow.x, pocketWindow.y, pocketWindow.w, pocketWindow.h, { 0,0,0 }, 150, 5);

		//������ �� �� �ִ��� üũ
		std::vector<int> pocketList;
		int numberOfBag = 0;
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
			{
				pocketList.push_back(i);
				numberOfBag++;
			}
		}

		if (numberOfBag == 0)
		{
			//������ ������ ���� �ʴ�.
			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
			setFontSize(16);
			drawTextCenter(sysStr[19], pocketWindow.x + pocketWindow.w / 2, pocketWindow.y + 35);
		}
		else
		{
			//���� 1~3��° ĭ �׸���
			if (pocketCursor != 0)
			{

				setZoom(2.0);
				drawStadium(pocketItem[2].x, pocketItem[2].y, pocketItem[2].w, pocketItem[2].h, { 0,0,0 }, 200, 5);
				drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 1]].sprIndex, pocketItem[2].x + (pocketItem[2].w / 2), pocketItem[2].y + (pocketItem[2].h / 2));
				if (pocketCursor != 1)
				{
					drawStadium(pocketItem[1].x, pocketItem[1].y, pocketItem[1].w, pocketItem[1].h, { 0,0,0 }, 200, 5);
					drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 2]].sprIndex, pocketItem[1].x + (pocketItem[1].w / 2), pocketItem[1].y + (pocketItem[1].h / 2));
					if (pocketCursor != 2)
					{
						drawStadium(pocketItem[0].x, pocketItem[0].y, pocketItem[0].w, pocketItem[0].h, { 0,0,0 }, 200, 5);
						drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor - 3]].sprIndex, pocketItem[0].x + (pocketItem[0].w / 2), pocketItem[0].y + (pocketItem[0].h / 2));
					}
				}
			}

			//���� 4��° ĭ
			setZoom(3.0);
			drawStadium(pocketItem[3].x, pocketItem[3].y, pocketItem[3].w, pocketItem[3].h, lowCol::blue, 200, 5);
			drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor]].sprIndex, pocketItem[3].x + (pocketItem[3].w / 2), pocketItem[3].y + (pocketItem[3].h / 2));

			//���� 5~7��° ĭ 
			if (pocketCursor != numberOfBag - 1)
			{
				setZoom(2.0);
				drawStadium(pocketItem[4].x, pocketItem[4].y, pocketItem[4].w, pocketItem[4].h, { 0,0,0 }, 200, 5);
				drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 1]].sprIndex, pocketItem[4].x + (pocketItem[4].w / 2), pocketItem[4].y + (pocketItem[4].h / 2));
				if (pocketCursor != numberOfBag - 2)
				{
					drawStadium(pocketItem[5].x, pocketItem[5].y, pocketItem[5].w, pocketItem[5].h, { 0,0,0 }, 200, 5);
					drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 2]].sprIndex, pocketItem[5].x + (pocketItem[5].w / 2), pocketItem[5].y + (pocketItem[5].h / 2));
					if (pocketCursor != numberOfBag - 3)
					{
						drawStadium(pocketItem[6].x, pocketItem[6].y, pocketItem[6].w, pocketItem[6].h, { 0,0,0 }, 200, 5);
						drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor + 3]].sprIndex, pocketItem[6].x + (pocketItem[6].w / 2), pocketItem[6].y + (pocketItem[6].h / 2));
					}
				}
			}
			setZoom(1.0);


			//���� ���� ������
			SDL_Rect weightBar = { pocketWindow.x + 12, pocketWindow.y + 64, 72, 4 };
			drawRect(weightBar, col::white);

			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
			setFontSize(10);
			drawTextCenter(L"132.9/99.9 KG", weightBar.x + (weightBar.w / 2), weightBar.y - 8);

			//���� �ָӴ� ���� ������
			SDL_Rect volumeBar = { pocketWindow.x + pocketWindow.w - 12 - 72, pocketWindow.y + 64, 72, 4 };
			drawRect(volumeBar, col::white);
			SDL_Rect volumeGauge = { volumeBar.x + 1, volumeBar.y + 1, volumeBar.w - 2, 2 };
			int maxVolume = equipPtr->itemInfo[pocketList[pocketCursor]].pocketMaxVolume;
			int currentVolume = equipPtr->itemInfo[pocketList[pocketCursor]].pocketVolume;
			volumeGauge.w = (volumeBar.w - 2) * ((float)currentVolume / (float)maxVolume);
			drawFillRect(volumeGauge, lowCol::green);

			std::wstring volumeStr = decimalCutter(currentVolume / 1000.0, 2) + L"/" + decimalCutter(maxVolume / 1000.0, 2) + L" L";
			SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
			setFontSize(10);
			drawTextCenter(volumeStr, volumeBar.x + (volumeBar.w / 2), volumeBar.y - 8);
			setFontSize(13);
			drawTextCenter(L"�׽�Ʈ ������", pocketWindow.x + pocketWindow.w / 2, pocketWindow.y + pocketWindow.h - 10);



			//���� �¿� ���� ��ư
			{
				SDL_Color leftBtnColor;
				if (checkCursor(&pocketLeft))
				{
					if (click == true) { leftBtnColor = lowCol::deepBlue; }
					else { leftBtnColor = lowCol::blue; }
				}
				else { leftBtnColor = lowCol::black; }
				drawStadium(pocketLeft.x, pocketLeft.y, pocketLeft.w, pocketLeft.h, leftBtnColor, 200, 5);
				

				if (inputType == input::keyboard)
				{
					if (state[SDL_SCANCODE_LSHIFT]) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_LShift, pocketLeft.x + pocketLeft.w / 2, pocketLeft.y + pocketRight.h / 2);
				}
				else if (inputType == input::gamepad)
				{
					if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER)) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_L1, pocketLeft.x + pocketLeft.w / 2, pocketLeft.y + pocketRight.h / 2);
				}
				else
				{
					drawSpriteCenter(spr::windowArrow, 2, pocketWindow.x + 16, pocketWindow.y + 27);
				}

				if (pocketCursor == 0) { drawStadium(pocketLeft.x, pocketLeft.y, pocketLeft.w, pocketLeft.h, leftBtnColor, 200, 5); }

			}
			{
				SDL_Color rightBtnColor;
				if (checkCursor(&pocketRight))
				{
					if (click == true) { rightBtnColor = lowCol::deepBlue; }
					else { rightBtnColor = lowCol::blue; }
				}
				else { rightBtnColor = lowCol::black; }
				drawStadium(pocketRight.x, pocketRight.y, pocketRight.w, pocketRight.h, rightBtnColor, 200, 5);

				if (inputType == input::keyboard)
				{
					if (state[SDL_SCANCODE_RSHIFT]) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_RShift, pocketRight.x + pocketRight.w / 2, pocketRight.y + pocketRight.h / 2);
				}
				else if (inputType == input::gamepad)
				{
					if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_R1, pocketRight.x + pocketRight.w / 2, pocketRight.y + pocketRight.h / 2);
				}
				else
				{
					drawSpriteCenter(spr::windowArrow, 0, pocketWindow.x + pocketWindow.w - 16, pocketWindow.y + 27);
				}

				if (pocketCursor == numberOfBag - 1) { drawStadium(pocketRight.x, pocketRight.y, pocketRight.w, pocketRight.h, rightBtnColor, 200, 5); }
			}
		}
	}



	//���⼭���� ���� ������
	{
		//���� ������ ��ü
		//drawStadium(lootWindow.x, lootWindow.y, lootWindow.w, lootWindow.h, { 0,0,0 }, 150, 5);
		SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
		setFontSize(10);
		drawText(std::to_wstring(lootCursor + 1) + L"/" + std::to_wstring(lootPtr->itemInfo.size()), lootWindow.x + 6, lootWindow.y + lootWindow.h - 16);

		//���� ������ ��ܹ� ��(���� �̸� ������)
		drawStadium(lootLabel.x, lootLabel.y, lootLabel.w, lootLabel.h, { 0,0,0 }, 150, 5);
		if (checkCursor(&lootLabelSelect) || labelCursor == 0)
		{
			SDL_Color btnColor;
			if (click == true) { btnColor = lowCol::deepBlue; }
			else { btnColor = lowCol::blue; }
			drawStadium(lootLabelSelect.x, lootLabelSelect.y, lootLabelSelect.w, lootLabelSelect.h, btnColor, 150, 5);
		}
		else if (checkCursor(&lootLabelName) || labelCursor == 1)
		{
			SDL_Color btnColor;
			if (click == true) { btnColor = lowCol::deepBlue; }
			else { btnColor = lowCol::blue; }
			drawStadium(lootLabelName.x, lootLabelName.y, lootLabelName.w, lootLabelName.h, btnColor, 150, 5);
		}
		else if (checkCursor(&lootLabelQuantity) || labelCursor == 2)
		{
			SDL_Color btnColor;
			if (click == true) { btnColor = lowCol::deepBlue; }
			else { btnColor = lowCol::blue; }
			drawStadium(lootLabelQuantity.x, lootLabelQuantity.y, lootLabelQuantity.w, lootLabelQuantity.h, btnColor, 150, 5);
		}

		SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
		setFontSize(13);
		drawTextCenter(sysStr[15], lootLabel.x + 32, lootLabel.y + 12); //����(��ܹ�)

		{ //�̸�(��ܹ�)
			std::wstring tailStr = L"";
			int grayNumber = 0;
			int whiteNumber = 0;
			//�̹� �˻� ������ üũ
			for (int i = 0; i < lootPtr->itemInfo.size(); i++)
			{
				if (lootPtr->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))//�̹� �˻� ���� ��� �˻� ���¸� ������
				{
					grayNumber++;
				}
			}

			if (grayNumber > 0)
			{
				whiteNumber = lootPtr->itemInfo.size() - grayNumber;
				tailStr = L"(" + std::to_wstring(whiteNumber) + sysStr[87] + L")";// n�� ������ �˻���
			}

			drawTextCenter(sysStr[16] + tailStr, lootLabel.x + 152, lootLabel.y + 12);
		}

		switch (getSortType())
		{
		default:
			drawTextCenter(sysStr[24], lootLabel.x + 280, lootLabel.y + 12); //������(��ܹ�)
			break;
		case sortFlag::weightDescend:
			drawTextCenter(sysStr[45], lootLabel.x + 280, lootLabel.y + 12); //������(��ܹ�)
			break;
		case sortFlag::weightAscend:
			drawTextCenter(sysStr[46], lootLabel.x + 280, lootLabel.y + 12); //������(��ܹ�)
			break;
		case sortFlag::volumeDescend:
			drawTextCenter(sysStr[47], lootLabel.x + 280, lootLabel.y + 12); //������(��ܹ�)
			break;
		case sortFlag::volumeAscend:
			drawTextCenter(sysStr[48], lootLabel.x + 280, lootLabel.y + 12); //������(��ܹ�)
			break;
		}


		//���� ������
		drawItemList(lootPtr, lootArea.x, lootArea.y, lootItemMax, lootCursor, lootScroll, true);

		// ������ ��ũ�� �׸���
		drawFillRect(lootScrollBox, { 120,120,120 });
		SDL_Rect inScrollBox = { lootWindow.x + 328, lootWindow.y + 40, 2, 42 * lootItemMax }; // ���� ��ũ�� Ŀ��
		inScrollBox.h = lootScrollBox.h * myMin(1.0, (double)lootItemMax / lootPtr->itemInfo.size());
		inScrollBox.y = lootScrollBox.y + lootScrollBox.h * ((float)lootScroll / (float)lootPtr->itemInfo.size());
		if (inScrollBox.y + inScrollBox.h > lootScrollBox.y + lootScrollBox.h) { inScrollBox.y = lootScrollBox.y + lootScrollBox.h - inScrollBox.h; }
		drawFillRect(inScrollBox, col::white);

		//���ù�ư �׸���
		{
			SDL_Color lootBtnColor;
			if (checkCursor(&lootBtn))
			{
				if (click == true) { lootBtnColor = lowCol::deepBlue; }
				else { lootBtnColor = lowCol::blue; }
			}
			else { lootBtnColor = lowCol::black; }

			drawFillRect(lootBtn, lootBtnColor, 200);
			drawRect(lootBtn, { 0x57, 0x57, 0x57 });

			

			if (inputType == input::keyboard)
			{
				if (state[SDL_SCANCODE_V]) { targetBtnSpr = spr::buttonsPressed; }
				else { targetBtnSpr = spr::buttons; }
				drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_V, lootBtn.x + lootBtn.w / 2, lootBtn.y + lootBtn.h / 2);
			}
			else if (inputType == input::gamepad)
			{
				if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y)) { targetBtnSpr = spr::buttonsPressed; }
				else { targetBtnSpr = spr::buttons; }
				drawSpriteCenter(targetBtnSpr, keyIcon::duelSense_TRI, lootBtn.x + lootBtn.w / 2, lootBtn.y + lootBtn.h / 2);
			}
			else
			{
				drawSpriteCenter(spr::lootBagArrow, 1, lootWindow.x + lootWindow.w / 2, lootWindow.y - 4);
			}

			for (int i = 0; i < lootPtr->itemInfo.size(); i++)
			{
				if (lootPtr->itemInfo[i].lootSelect > 0)
				{
					break;
				}
				if (i == lootPtr->itemInfo.size() - 1)
				{
					drawStadium(lootBtn.x, lootBtn.y, lootBtn.w, lootBtn.h, lootBtnColor, 200, 5);
				}
			}
		}
	}


}