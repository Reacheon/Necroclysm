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
		//이큅 윈도우 본체
		//drawStadium(equipWindow.x, equipWindow.y, equipWindow.w, equipWindow.h, { 0,0,0 }, 150, 5);
		SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
		setFontSize(10);
		drawText(std::to_wstring(equipCursor + 1) + L"/" + std::to_wstring(equipPtr->itemInfo.size()), equipWindow.x + 6, equipWindow.y + equipWindow.h - 16);

		//우측 아이템 상단바(선택 이름 물리량)
		drawStadium(equipLabel.x, equipLabel.y, equipLabel.w, equipLabel.h, { 0,0,0 }, 150, 5);
		if (checkCursor(&equipLabelSelect))
		{
			drawStadium(equipLabelSelect.x, equipLabelSelect.y, equipLabelSelect.w, equipLabelSelect.h, lowCol::blue, 150, 5);
		}
		else if (checkCursor(&equipLabelName))
		{
			drawStadium(equipLabelName.x, equipLabelName.y, equipLabelName.w, equipLabelName.h, lowCol::blue, 150, 5);
		}
		else if (checkCursor(&equipLabelQuantity))
		{
			drawStadium(equipLabelQuantity.x, equipLabelQuantity.y, equipLabelQuantity.w, equipLabelQuantity.h, lowCol::blue, 150, 5);
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
		drawFillRect(equipScrollBox, { 120,120,120 });
		SDL_Rect inScrollBox = { equipWindow.x + 328, equipWindow.y + 40, 2, 42 * equipItemMax }; // 내부 스크롤 커서
		inScrollBox.h = equipScrollBox.h * myMin(1.0, (double)equipItemMax / equipPtr->itemInfo.size());
		inScrollBox.y = equipScrollBox.y + equipScrollBox.h * ((float)equipScroll / (float)equipPtr->itemInfo.size());
		drawFillRect(inScrollBox, col::white);
	}



	if (aniUSet.find(this) == aniUSet.end())
	{
		drawStadium(topWindow.x, topWindow.y, topWindow.w, topWindow.h, { 0,0,0 }, 180, 5);

		setFontSize(14);

		SDL_SetRenderDrawColor(renderer, col::lightGray.r, col::lightGray.g, col::lightGray.b, 0xff);

		drawText(L"HEAD", topWindow.x + 10, topWindow.y + 24 + 18 * 0);
		drawText(L"TORSO", topWindow.x + 10, topWindow.y + 24 + 18 * 1);
		drawText(L"LARM", topWindow.x + 10, topWindow.y + 24 + 18 * 2);
		drawText(L"RARM", topWindow.x + 10, topWindow.y + 24 + 18 * 3);
		drawText(L"LLEG", topWindow.x + 10, topWindow.y + 24 + 18 * 4);
		drawText(L"RLEG", topWindow.x + 10, topWindow.y + 24 + 18 * 5);

		SDL_SetRenderDrawColor(renderer, lowCol::orange.r, lowCol::orange.g, lowCol::orange.b, 0xff);

		drawTextCenter(L"rPierce", topWindow.x + 30 + 54 * 1, topWindow.y + 24 + 18 * -1 + 9);
		drawTextCenter(L"rCut", topWindow.x + 30 + 54 * 2, topWindow.y + 24 + 18 * -1 + 9);
		drawTextCenter(L"rBash", topWindow.x + 30 + 54 * 3, topWindow.y + 24 + 18 * -1 + 9);
		drawTextCenter(L"Enc", topWindow.x + 30 + 54 * 4, topWindow.y + 24 + 18 * -1 + 9);

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


		SDL_SetRenderDrawColor(renderer, lowCol::orange.r, lowCol::orange.g, lowCol::orange.b, 0xff);

		drawText(L"Shield", topWindow.x + 290, topWindow.y + 24 + 18 * -1);
		drawText(L"Evade", topWindow.x + 290, topWindow.y + 24 + 18 * 0);

		SDL_SetRenderDrawColor(renderer, col::lightGray.r, col::lightGray.g, col::lightGray.b, 0xff);

		drawText(L"rFire", topWindow.x + 290, topWindow.y + 24 + 18 * 1);
		drawText(L"rCold", topWindow.x + 290, topWindow.y + 24 + 18 * 2);
		drawText(L"rElec", topWindow.x + 290, topWindow.y + 24 + 18 * 3);
		drawText(L"rNeg", topWindow.x + 290, topWindow.y + 24 + 18 * 4);
		drawText(L"rCorr", topWindow.x + 290, topWindow.y + 24 + 18 * 5);

		SDL_SetRenderDrawColor(renderer, col::white.r, col::white.g, col::white.b, 0xff);

		int SH = Player::ins()->getSH();
		int EV = Player::ins()->getEV();

		drawTextCenter(std::to_wstring(SH), topWindow.x + 290 + 70 + 20, topWindow.y + 24 + 18 * -1 + 9);
		drawTextCenter(std::to_wstring(EV), topWindow.x + 290 + 70 + 20, topWindow.y + 24 + 18 * 0 + 9);

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

		//우측 포켓창 그리기
		if (pocketCursor >= 0)
		{
			const Uint8* state = SDL_GetKeyboardState(NULL);
			Sprite* targetBtnSpr = nullptr;

			drawWindow(&lootBase, sysStr[10], 1);

			//가방이 몇 개 있는지 체크
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
				//가방을 가지고 있지 않다.
				SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
				setFontSize(16);
				drawTextCenter(sysStr[19], pocketWindow.x + pocketWindow.w / 2, pocketWindow.y + 35);
			}
			else
			{
				//포켓 1~3번째 칸 그리기
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

				//포켓 4번째 칸
				setZoom(3.0);
				drawStadium(pocketItem[3].x, pocketItem[3].y, pocketItem[3].w, pocketItem[3].h, lowCol::blue, 200, 5);
				drawSpriteCenter(spr::itemset, equipPtr->itemInfo[pocketList[pocketCursor]].sprIndex, pocketItem[3].x + (pocketItem[3].w / 2), pocketItem[3].y + (pocketItem[3].h / 2));

				//포켓 5~7번째 칸 
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


				//포켓 질량 게이지
				SDL_Rect weightBar = { pocketWindow.x + 12, pocketWindow.y + 64, 72, 4 };
				drawRect(weightBar, col::white);

				SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
				setFontSize(10);
				drawTextCenter(L"132.9/99.9 KG", weightBar.x + (weightBar.w / 2), weightBar.y - 8);

				//루팅 주머니 부피 게이지
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
				drawTextCenter(L"테스트 아이템", pocketWindow.x + pocketWindow.w / 2, pocketWindow.y + pocketWindow.h - 10);




				{
					SDL_Color leftBtnColor;
					if (checkCursor(&pocketLeft))
					{
						if (click == true) { leftBtnColor = lowCol::deepBlue; }
						else { leftBtnColor = lowCol::blue; }
					}
					else { leftBtnColor = lowCol::black; }
					drawStadium(pocketLeft.x, pocketLeft.y, pocketLeft.w, pocketLeft.h, leftBtnColor, 200, 5);
					drawSpriteCenter(spr::windowArrow, 2, pocketWindow.x + 16, pocketWindow.y + 27);

					if (inputType == input::keyboard)
					{
						if (state[SDL_SCANCODE_LSHIFT]) { targetBtnSpr = spr::buttonsPressed; }
						else { targetBtnSpr = spr::buttons; }
						drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_LShift, pocketLeft.x + pocketLeft.w / 2, pocketLeft.y + pocketRight.h / 2);
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
					drawSpriteCenter(spr::windowArrow, 0, pocketWindow.x + pocketWindow.w - 16, pocketWindow.y + 27);

					if (inputType == input::keyboard)
					{
						if (state[SDL_SCANCODE_RSHIFT]) { targetBtnSpr = spr::buttonsPressed; }
						else { targetBtnSpr = spr::buttons; }
						drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_RShift, pocketRight.x + pocketRight.w / 2, pocketRight.y + pocketRight.h / 2);
					}

					if (pocketCursor == numberOfBag - 1) { drawStadium(pocketRight.x, pocketRight.y, pocketRight.w, pocketRight.h, rightBtnColor, 200, 5); }
				}
			}

			//여기서부턴 루팅 윈도우
			{
				//루팅 윈도우 본체
				//drawStadium(lootWindow.x, lootWindow.y, lootWindow.w, lootWindow.h, { 0,0,0 }, 150, 5);
				SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
				setFontSize(10);
				drawText(std::to_wstring(lootCursor + 1) + L"/" + std::to_wstring(lootPtr->itemInfo.size()), lootWindow.x + 6, lootWindow.y + lootWindow.h - 16);

				//우측 아이템 상단바 라벨(선택 이름 물리량)
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
				drawTextCenter(sysStr[15], lootLabel.x + 32, lootLabel.y + 12); //선택(상단바)

				{ //이름(상단바)
					std::wstring tailStr = L"";
					int grayNumber = 0;
					int whiteNumber = 0;
					//이미 검색 중인지 체크
					for (int i = 0; i < lootPtr->itemInfo.size(); i++)
					{
						if (lootPtr->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))//이미 검색 중일 경우 검색 상태를 해제함
						{
							grayNumber++;
						}
					}

					if (grayNumber > 0)
					{
						whiteNumber = lootPtr->itemInfo.size() - grayNumber;
						tailStr = L"(" + std::to_wstring(whiteNumber) + sysStr[87] + L")";// n개 아이템 검색됨
					}

					drawTextCenter(sysStr[16] + tailStr, lootLabel.x + 152, lootLabel.y + 12);
				}

				switch (getSortType())
				{
				default:
					drawTextCenter(sysStr[24], lootLabel.x + 280, lootLabel.y + 12); //물리량(상단바)
					break;
				case sortFlag::weightDescend:
					drawTextCenter(sysStr[45], lootLabel.x + 280, lootLabel.y + 12); //물리량(상단바)
					break;
				case sortFlag::weightAscend:
					drawTextCenter(sysStr[46], lootLabel.x + 280, lootLabel.y + 12); //물리량(상단바)
					break;
				case sortFlag::volumeDescend:
					drawTextCenter(sysStr[47], lootLabel.x + 280, lootLabel.y + 12); //물리량(상단바)
					break;
				case sortFlag::volumeAscend:
					drawTextCenter(sysStr[48], lootLabel.x + 280, lootLabel.y + 12); //물리량(상단바)
					break;
				}


				//개별 아이템
				drawItemList(lootPtr, lootArea.x, lootArea.y, lootItemMax, lootCursor, lootScroll, true);

				if (lootPtr->itemInfo.size() == 0) // 만약 아이템이 없을 경우
				{
					setFontSize(13);
					drawTextCenter(col2Str(col::white) + sysStr[89], lootArea.x + lootArea.w / 2, lootArea.y + lootArea.h / 2);
				}

				// 아이템 스크롤 그리기
				drawFillRect(lootScrollBox, { 120,120,120 });
				SDL_Rect inScrollBox = { lootWindow.x + 328, lootWindow.y + 40, 2, 42 * lootItemMax }; // 내부 스크롤 커서
				inScrollBox.h = lootScrollBox.h * myMin(1.0, (double)lootItemMax / lootPtr->itemInfo.size());
				inScrollBox.y = lootScrollBox.y + lootScrollBox.h * ((float)lootScroll / (float)lootPtr->itemInfo.size());
				if (inScrollBox.y + inScrollBox.h > lootScrollBox.y + lootScrollBox.h) { inScrollBox.y = lootScrollBox.y + lootScrollBox.h - inScrollBox.h; }
				drawFillRect(inScrollBox, col::white);

				//루팅(버리기)버튼 그리기
				{
					SDL_Rect bridgeRect = { pocketWindow.x + pocketWindow.w / 2 - 36, pocketWindow.y + pocketWindow.h, 72,10 };
					drawFillRect(bridgeRect, col::black, 150);

					SDL_Color lootBtnColor;
					if (checkCursor(&lootBtn))
					{
						if (click == true) { lootBtnColor = lowCol::deepBlue; }
						else { lootBtnColor = lowCol::blue; }
					}
					else { lootBtnColor = lowCol::black; }

					drawFillRect(lootBtn, lootBtnColor, 200);

					drawRect(lootBtn, { 0x57,0x57,0x57 });

					setZoom(0.7);
					drawSpriteCenter(spr::icon32, 0, lootWindow.x + lootWindow.w / 2, lootWindow.y - 2);
					setZoom(1.0);

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

					if (lootPtr->itemInfo.size() == 0)
					{
						drawStadium(lootBtn.x, lootBtn.y, lootBtn.w, lootBtn.h, lootBtnColor, 200, 5);
					}

					if (inputType == input::keyboard)
					{
						if (state[SDL_SCANCODE_V]) { targetBtnSpr = spr::buttonsPressed; }
						else { targetBtnSpr = spr::buttons; }
						drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_V, lootBtn.x, lootBtn.y + lootBtn.h / 2);
					}
				}
			}
		}
	}
}