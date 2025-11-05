#include <SDL3/SDL.h>

import Equip;
import util;
import globalVar;
import wrapVar;
import constVar;
import textureVar;
import Player;
import drawText;
import checkCursor;
import drawWindow;
import drawItemList;
import drawSprite;
import ItemData;
import CoordSelect;

void Equip::drawGUI()
{
	if (getStateDraw() == false) { return; }
	if (CoordSelect::ins() != nullptr) return;


	drawWindow(&equipBase, sysStr[332], 94);


	//여기서부턴 이큅 윈도우
	{
		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		
		//플레이어 무게 제한 게이지 그리기
		SDL_Rect weightGaugeRect = { equipBase.x + 65,equipBase.y + 39,104,9 };
		drawRect(weightGaugeRect, col::white);
		drawFillRect(SDL_Rect{ weightGaugeRect.x + 2,weightGaugeRect.y + 2,50,5 }, lowCol::green);
		drawSpriteCenter(spr::icon16, 61, weightGaugeRect.x - 47, weightGaugeRect.y + 4);
		setFontSize(10);
		drawText(sysStr[163], weightGaugeRect.x - 38, weightGaugeRect.y - 2);//무게
		setFontSize(8);
		drawText(L"32.5 / 92.3 kg", weightGaugeRect.x + 110, weightGaugeRect.y - 1);


		//이큅 윈도우 본체
		setFontSize(10);
		drawText(std::to_wstring(equipCursor + 1) + L"/" + std::to_wstring(equipPtr->itemInfo.size()), equipWindow.x + 6, equipWindow.y + equipWindow.h - 8);

		//우측 아이템 상단바(선택 이름 물리량)
		drawStadium(equipLabel.x, equipLabel.y, equipLabel.w, equipLabel.h, { 0,0,0 }, 183, 5);
		if (GUI::getLastGUI() == this)
		{
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
		}
		setFontSize(12);
		drawText(sysStr[15], equipLabel.x + 10, equipLabel.y + 4); //선택(상단바)
		drawText(sysStr[16], equipLabel.x + 140, equipLabel.y + 4); //이름(상단바)
		drawText(sysStr[24], equipLabel.x + 260, equipLabel.y + 4); //무리량(상단바)

		//개별 아이템
		if (GUI::getLastGUI() != this) itemListColorLock = true;
		else  itemListColorLock = false;
		drawItemList(equipPtr, equipArea.x, equipArea.y, EQUIP_ITEM_MAX, equipCursor, equipScroll, isTargetPocket == false);

		if (equipPtr->itemInfo.size() == 0) // 만약 아이템이 없을 경우
		{
			drawTextCenter(sysStr[90], equipArea.x + equipArea.w / 2, equipArea.y + equipArea.h / 2);
		}

		// 아이템 스크롤 그리기
		SDL_Rect equipScrollBox = { equipBase.x + 325, equipItemRect[0].y, 2, equipItemRect[EQUIP_ITEM_MAX - 1].y + equipItemRect[EQUIP_ITEM_MAX - 1].h - equipItemRect[0].y };
		drawFillRect(equipScrollBox, { 120,120,120 });
		SDL_Rect inScrollBox = equipScrollBox; // 내부 스크롤 커서
		inScrollBox.h = equipScrollBox.h * myMin(1.0, (double)EQUIP_ITEM_MAX / PlayerPtr->getEquipPtr()->itemInfo.size());
		inScrollBox.y = equipScrollBox.y + equipScrollBox.h * ((float)equipScroll / (float)PlayerPtr->getEquipPtr()->itemInfo.size());
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

		setFont(fontType::pixel);
		setFontSize(12);

		drawText(sysStr[107], topWindow.x + 10, topWindow.y + 24 + 18 * 0, col::lightGray);//머리
		drawText(sysStr[106], topWindow.x + 10, topWindow.y + 24 + 18 * 1, col::lightGray);//몸통
		drawText(sysStr[108], topWindow.x + 10, topWindow.y + 24 + 18 * 2, col::lightGray);//왼팔
		drawText(sysStr[109], topWindow.x + 10, topWindow.y + 24 + 18 * 3, col::lightGray);//오른팔
		drawText(sysStr[110], topWindow.x + 10, topWindow.y + 24 + 18 * 4, col::lightGray);//왼다리
		drawText(sysStr[111], topWindow.x + 10, topWindow.y + 24 + 18 * 5, col::lightGray);//오른다리


		setFontSize(11);
		setFont(fontType::pixel);
		drawTextCenter(sysStr[164], topWindow.x + 30 + 54 * 1, topWindow.y + 24 + 18 * -1 + 9, col::orange);//관통저항
		drawTextCenter(sysStr[165], topWindow.x + 30 + 54 * 2, topWindow.y + 24 + 18 * -1 + 9, col::orange);//참격저항
		drawTextCenter(sysStr[166], topWindow.x + 30 + 54 * 3, topWindow.y + 24 + 18 * -1 + 9, col::orange);//타격저항
		drawTextCenter(sysStr[167], topWindow.x + 30 + 54 * 4, topWindow.y + 24 + 18 * -1 + 9, col::orange);//방해도

		for (int i = 0; i < 6; i++)
		{
			int rPierce, rCut, rBash, enc, maxEnc;
			enum class BodyPart
			{
				Head = 0,
				Torso = 1,
				LeftArm = 2,
				RightArm = 3,
				LeftLeg = 4,
				RightLeg = 5
			};

			switch (static_cast<BodyPart>(i))
			{
			default:
			case BodyPart::Head:
				rPierce = PlayerPtr->getResPierceHead();
				rCut = PlayerPtr->getResCutHead();
				rBash = PlayerPtr->getResBashHead();
				enc = PlayerPtr->getEncHead();
				break;
			case BodyPart::Torso:
				rPierce = PlayerPtr->getResPierceTorso();
				rCut = PlayerPtr->getResCutTorso();
				rBash = PlayerPtr->getResBashTorso();
				enc = PlayerPtr->getEncTorso();
				break;
			case BodyPart::LeftArm:
				rPierce = PlayerPtr->getResPierceLArm();
				rCut = PlayerPtr->getResCutLArm();
				rBash = PlayerPtr->getResBashLArm();
				enc = PlayerPtr->getEncLArm();
				break;
			case BodyPart::RightArm:
				rPierce = PlayerPtr->getResPierceRArm();
				rCut = PlayerPtr->getResCutRArm();
				rBash = PlayerPtr->getResBashRArm();
				enc = PlayerPtr->getEncRArm();
				break;
			case BodyPart::LeftLeg:
				rPierce = PlayerPtr->getResPierceLLeg();
				rCut = PlayerPtr->getResCutLLeg();
				rBash = PlayerPtr->getResBashLLeg();
				enc = PlayerPtr->getEncLLeg();
				break;
			case BodyPart::RightLeg:
				rPierce = PlayerPtr->getResPierceRLeg();
				rCut = PlayerPtr->getResCutRLeg();
				rBash = PlayerPtr->getResBashRLeg();
				enc = PlayerPtr->getEncRLeg();
				break;
			}


			maxEnc = MAX_ENC;

			drawTextCenter(std::to_wstring(rPierce), topWindow.x + 30 + 54 * 1, topWindow.y + 24 + 18 * i + 9);
			drawTextCenter(std::to_wstring(rCut), topWindow.x + 30 + 54 * 2, topWindow.y + 24 + 18 * i + 9);
			drawTextCenter(std::to_wstring(rBash), topWindow.x + 30 + 54 * 3, topWindow.y + 24 + 18 * i + 9);
			drawTextCenter(std::to_wstring(enc) + L" / " + std::to_wstring(maxEnc), topWindow.x + 30 + 54 * 4, topWindow.y + 24 + 18 * i + 9, col::lightGray);

			
		}


		setFontSize(12);
		setFont(fontType::pixel);
		drawText(sysStr[168], topWindow.x + 290, topWindow.y + 24 + 18 * -1, lowCol::orange); // 방어
		drawText(sysStr[169], topWindow.x + 290, topWindow.y + 24 + 18 * 0, lowCol::orange); // 회피

		drawText(sysStr[170], topWindow.x + 290, topWindow.y + 24 + 18 * 1, col::lightGray); // 화염저항
		drawText(sysStr[171], topWindow.x + 290, topWindow.y + 24 + 18 * 2, col::lightGray); // 냉기저항
		drawText(sysStr[172], topWindow.x + 290, topWindow.y + 24 + 18 * 3, col::lightGray); // 전기저항
		drawText(sysStr[173], topWindow.x + 290, topWindow.y + 24 + 18 * 4, col::lightGray); // 피폭저항
		drawText(sysStr[174], topWindow.x + 290, topWindow.y + 24 + 18 * 5, col::lightGray); // 부식저항

		int SH = PlayerPtr->getSH();
		int EV = PlayerPtr->getEV();

		drawTextCenter(std::to_wstring(SH),
			topWindow.x + 290 + 70 + 20,
			topWindow.y + 24 + 18 * -1 + 9,
			col::white);
		drawTextCenter(std::to_wstring(EV),
			topWindow.x + 290 + 70 + 20,
			topWindow.y + 24 + 18 * 0 + 9,
			col::white);

		int rFire = PlayerPtr->entityInfo.rFire;
		int rCold = PlayerPtr->entityInfo.rCold;
		int rElec = PlayerPtr->entityInfo.rElec;
		int rCorr = PlayerPtr->entityInfo.rCorr;
		int rRad = PlayerPtr->entityInfo.rRad;

		drawText(L"Lv." + std::to_wstring(rFire),
			topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 1, lowCol::green);
		drawText(L"Lv." + std::to_wstring(rCold),
			topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 2, lowCol::green);
		drawText(L"Lv." + std::to_wstring(rElec),
			topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 3, lowCol::green);
		drawText(L"Lv." + std::to_wstring(rCorr),
			topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 4, lowCol::green);
		drawText(L"Lv." + std::to_wstring(rRad),
			topWindow.x + 290 + 70, topWindow.y + 24 + 18 * 5, lowCol::green);
	}

}