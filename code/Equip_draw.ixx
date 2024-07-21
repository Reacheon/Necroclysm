#include <SDL.h>

import Equip;
import util;
import globalVar;
import constVar;
import textureVar;
import Player;
import drawText;
import checkCursor;
import drawWindow;
import drawItemList;
import drawSprite;

void Equip::drawGUI()
{
	if (getStateDraw() == false) { return; }

	drawWindow(&equipBase, sysStr[13], 2);


	//여기서부턴 이큅 윈도우
	{
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		
		//플레이어 무게 제한 게이지 그리기

		SDL_Rect weightGaugeRect = { equipBase.x + 55,equipBase.y + 39,104,9 };
		drawRect(weightGaugeRect, col::white);
		drawFillRect({ weightGaugeRect.x + 2,weightGaugeRect.y + 2,50,5 }, lowCol::green);
		drawSpriteCenter(spr::icon16, 61, weightGaugeRect.x - 37, weightGaugeRect.y + 4);
		setFontSize(10);
		drawText(col2Str(col::white)+ L"무게", weightGaugeRect.x - 28, weightGaugeRect.y - 2);
		setFontSize(8);
		drawText(col2Str(col::white) + L"32.5 / 92.3 kg", weightGaugeRect.x + 110, weightGaugeRect.y - 1);

		//이큅 윈도우 본체
		SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
		setFontSize(10);
		drawText(std::to_wstring(equipCursor + 1) + L"/" + std::to_wstring(equipPtr->itemInfo.size()), equipWindow.x + 6, equipWindow.y + equipWindow.h - 16);

		//우측 아이템 상단바(선택 이름 물리량)
		drawStadium(equipLabel.x, equipLabel.y, equipLabel.w, equipLabel.h, { 0,0,0 }, 183, 5);
		if (checkCursor(&equipLabelSelect))
		{
			drawStadium(equipLabelSelect.x, equipLabelSelect.y, equipLabelSelect.w, equipLabelSelect.h, lowCol::blue, 183, 5);
		}
		else if (checkCursor(&equipLabelName))
		{
			drawStadium(equipLabelName.x, equipLabelName.y, equipLabelName.w, equipLabelName.h, lowCol::blue, 183, 5);
		}
		else if (checkCursor(&equipLabelQuantity))
		{
			drawStadium(equipLabelQuantity.x, equipLabelQuantity.y, equipLabelQuantity.w, equipLabelQuantity.h, lowCol::blue, 183, 5);
		}
		SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
		setFontSize(13);
		drawText(sysStr[15], equipLabel.x + 10, equipLabel.y + 4); //선택(상단바)
		drawText(sysStr[16], equipLabel.x + 140, equipLabel.y + 4); //이름(상단바)
		drawText(sysStr[24], equipLabel.x + 250, equipLabel.y + 4); //무리량(상단바)

		//개별 아이템
		drawItemList(equipPtr, equipArea.x, equipArea.y, equipItemMax, equipCursor, equipScroll, isTargetPocket == false);

		if (equipPtr->itemInfo.size() == 0) // 만약 아이템이 없을 경우
		{
			drawTextCenter(col2Str(col::white) + sysStr[90], equipArea.x + equipArea.w / 2, equipArea.y + equipArea.h / 2);
		}

		// 아이템 스크롤 그리기
		SDL_Rect equipScrollBox = { equipBase.x + 325, equipItemRect[0].y, 2, equipItemRect[equipItemMax - 1].y + equipItemRect[equipItemMax - 1].h - equipItemRect[0].y };
		drawFillRect(equipScrollBox, { 120,120,120 });
		SDL_Rect inScrollBox = equipScrollBox; // 내부 스크롤 커서
		inScrollBox.h = equipScrollBox.h * myMin(1.0, (double)equipItemMax / Player::ins()->getEquipPtr()->itemInfo.size());
		inScrollBox.y = equipScrollBox.y + equipScrollBox.h * ((float)equipScroll / (float)Player::ins()->getEquipPtr()->itemInfo.size());
		if (inScrollBox.y + inScrollBox.h > equipScrollBox.y + equipScrollBox.h) { inScrollBox.y = equipScrollBox.y + equipScrollBox.h - inScrollBox.h; }
		drawFillRect(inScrollBox, col::white);


		if (GUI::getActiveGUIList()[GUI::getActiveGUIList().size() - 1] != this)
		{
			drawFillRect(equipBase, col::black, 150);
		}
	}



	if (aniUSet.find(this) == aniUSet.end())
	{
		drawStadium(topWindow.x, topWindow.y, topWindow.w, topWindow.h, { 0,0,0 }, 180, 5);

		setFontSize(14);

		SDL_SetRenderDrawColor(renderer, col::lightGray.r, col::lightGray.g, col::lightGray.b, 0xff);

		drawText(col2Str(col::lightGray)+L"머리", topWindow.x + 10, topWindow.y + 24 + 18 * 0);
		drawText(col2Str(col::lightGray) + L"상체", topWindow.x + 10, topWindow.y + 24 + 18 * 1);
		drawText(col2Str(col::lightGray) + L"왼팔", topWindow.x + 10, topWindow.y + 24 + 18 * 2);
		drawText(col2Str(col::lightGray) + L"오른팔", topWindow.x + 10, topWindow.y + 24 + 18 * 3);
		drawText(col2Str(col::lightGray) + L"왼다리", topWindow.x + 10, topWindow.y + 24 + 18 * 4);
		drawText(col2Str(col::lightGray) + L"오른다리", topWindow.x + 10, topWindow.y + 24 + 18 * 5);

		SDL_SetRenderDrawColor(renderer, lowCol::orange.r, lowCol::orange.g, lowCol::orange.b, 0xff);

		setFontSize(11);
		drawTextCenter(L"관통저항", topWindow.x + 30 + 54 * 1, topWindow.y + 24 + 18 * -1 + 9);
		drawTextCenter(L"참격저항", topWindow.x + 30 + 54 * 2, topWindow.y + 24 + 18 * -1 + 9);
		drawTextCenter(L"타격저항", topWindow.x + 30 + 54 * 3, topWindow.y + 24 + 18 * -1 + 9);
		drawTextCenter(L"방해도", topWindow.x + 30 + 54 * 4, topWindow.y + 24 + 18 * -1 + 9);

		for (int i = 0; i < 6; i++)
		{
			int rPierce, rCut, rBash, enc, maxEnc;
			int targetPart = partType::head;
			switch (i)
			{
			default:
				targetPart = partType::head;
				break;
			case 1:
				targetPart = partType::torso;
				break;
			case 2:
				targetPart = partType::larm;
				break;
			case 3:
				targetPart = partType::rarm;
				break;
			case 4:
				targetPart = partType::lleg;
				break;
			case 5:
				targetPart = partType::rleg;
				break;
			}

			rPierce = Player::ins()->getRPierce(targetPart);
			rCut = Player::ins()->getRCut(targetPart);
			rBash = Player::ins()->getRBash(targetPart);
			enc = Player::ins()->getEnc(targetPart);
			maxEnc = 30;

			SDL_SetRenderDrawColor(renderer, col::white.r, col::white.g, col::white.b, 0xff);
			drawTextCenter(std::to_wstring(rPierce), topWindow.x + 30 + 54 * 1, topWindow.y + 24 + 18 * i + 9);
			drawTextCenter(std::to_wstring(rCut), topWindow.x + 30 + 54 * 2, topWindow.y + 24 + 18 * i + 9);
			drawTextCenter(std::to_wstring(rBash), topWindow.x + 30 + 54 * 3, topWindow.y + 24 + 18 * i + 9);
			SDL_SetRenderDrawColor(renderer, col::lightGray.r, col::lightGray.g, col::lightGray.b, 0xff);
			drawTextCenter(std::to_wstring(enc) + L"/" + std::to_wstring(maxEnc), topWindow.x + 30 + 54 * 4, topWindow.y + 24 + 18 * i + 9);
		}


		setFontSize(14);
		drawText(col2Str(lowCol::orange)+L"방어", topWindow.x + 290, topWindow.y + 24 + 18 * -1);
		drawText(col2Str(lowCol::orange) + L"회피", topWindow.x + 290, topWindow.y + 24 + 18 * 0);

		drawText(col2Str(col::lightGray)+L"화염저항", topWindow.x + 290, topWindow.y + 24 + 18 * 1);
		drawText(col2Str(col::lightGray) + L"냉기저항", topWindow.x + 290, topWindow.y + 24 + 18 * 2);
		drawText(col2Str(col::lightGray) + L"전기저항", topWindow.x + 290, topWindow.y + 24 + 18 * 3);
		drawText(col2Str(col::lightGray) + L"피폭저항", topWindow.x + 290, topWindow.y + 24 + 18 * 4);
		drawText(col2Str(col::lightGray) + L"부식저항", topWindow.x + 290, topWindow.y + 24 + 18 * 5);

		int SH = Player::ins()->getSH();
		int EV = Player::ins()->getEV();

		drawTextCenter(col2Str(col::white) + std::to_wstring(SH), topWindow.x + 290 + 70 + 20, topWindow.y + 24 + 18 * -1 + 9);
		drawTextCenter(col2Str(col::white) + std::to_wstring(EV), topWindow.x + 290 + 70 + 20, topWindow.y + 24 + 18 * 0 + 9);

		SDL_SetRenderDrawColor(renderer, lowCol::green.r, lowCol::green.g, lowCol::green.b, 0xff);

		int rFire = Player::ins()->entityInfo.rFire;
		int rCold = Player::ins()->entityInfo.rCold;
		int rElec = Player::ins()->entityInfo.rElec;
		int rCorr = Player::ins()->entityInfo.rCorr;
		int rRad = Player::ins()->entityInfo.rRad;

		drawText(L"Lv." + std::to_wstring(rFire), topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 1);
		drawText(L"Lv." + std::to_wstring(rCold), topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 2);
		drawText(L"Lv." + std::to_wstring(rElec), topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 3);
		drawText(L"Lv." + std::to_wstring(rCorr), topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 4);
		drawText(L"Lv." + std::to_wstring(rRad), topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 5);
	}

}