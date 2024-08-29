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


	//���⼭���� ��Ţ ������
	{
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		
		//�÷��̾� ���� ���� ������ �׸���
		SDL_Rect weightGaugeRect = { equipBase.x + 65,equipBase.y + 39,104,9 };
		drawRect(weightGaugeRect, col::white);
		drawFillRect({ weightGaugeRect.x + 2,weightGaugeRect.y + 2,50,5 }, lowCol::green);
		drawSpriteCenter(spr::icon16, 61, weightGaugeRect.x - 47, weightGaugeRect.y + 4);
		setFontSize(10);
		drawText(col2Str(col::white)+ sysStr[163], weightGaugeRect.x - 38, weightGaugeRect.y - 2);//����
		setFontSize(8);
		drawText(col2Str(col::white) + L"32.5 / 92.3 kg", weightGaugeRect.x + 110, weightGaugeRect.y - 1);


		//��Ţ ������ ��ü
		SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
		setFontSize(10);
		drawText(std::to_wstring(equipCursor + 1) + L"/" + std::to_wstring(equipPtr->itemInfo.size()), equipWindow.x + 6, equipWindow.y + equipWindow.h - 16);

		//���� ������ ��ܹ�(���� �̸� ������)
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
		SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
		setFontSize(13);
		drawText(sysStr[15], equipLabel.x + 10, equipLabel.y + 4); //����(��ܹ�)
		drawText(sysStr[16], equipLabel.x + 140, equipLabel.y + 4); //�̸�(��ܹ�)
		drawText(sysStr[24], equipLabel.x + 250, equipLabel.y + 4); //������(��ܹ�)

		//���� ������
		if (GUI::getLastGUI() != this) itemListColorLock = true;
		else  itemListColorLock = false;
		drawItemList(equipPtr, equipArea.x, equipArea.y, EQUIP_ITEM_MAX, equipCursor, equipScroll, isTargetPocket == false);

		if (equipPtr->itemInfo.size() == 0) // ���� �������� ���� ���
		{
			drawTextCenter(col2Str(col::white) + sysStr[90], equipArea.x + equipArea.w / 2, equipArea.y + equipArea.h / 2);
		}

		// ������ ��ũ�� �׸���
		SDL_Rect equipScrollBox = { equipBase.x + 325, equipItemRect[0].y, 2, equipItemRect[EQUIP_ITEM_MAX - 1].y + equipItemRect[EQUIP_ITEM_MAX - 1].h - equipItemRect[0].y };
		drawFillRect(equipScrollBox, { 120,120,120 });
		SDL_Rect inScrollBox = equipScrollBox; // ���� ��ũ�� Ŀ��
		inScrollBox.h = equipScrollBox.h * myMin(1.0, (double)EQUIP_ITEM_MAX / Player::ins()->getEquipPtr()->itemInfo.size());
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

		drawText(col2Str(col::lightGray) + sysStr[107], topWindow.x + 10, topWindow.y + 24 + 18 * 0);//�Ӹ�
		drawText(col2Str(col::lightGray) + sysStr[106], topWindow.x + 10, topWindow.y + 24 + 18 * 1);//����
		drawText(col2Str(col::lightGray) + sysStr[108], topWindow.x + 10, topWindow.y + 24 + 18 * 2);//����
		drawText(col2Str(col::lightGray) + sysStr[109], topWindow.x + 10, topWindow.y + 24 + 18 * 3);//������
		drawText(col2Str(col::lightGray) + sysStr[110], topWindow.x + 10, topWindow.y + 24 + 18 * 4);//�޴ٸ�
		drawText(col2Str(col::lightGray) + sysStr[111], topWindow.x + 10, topWindow.y + 24 + 18 * 5);//�����ٸ�

		SDL_SetRenderDrawColor(renderer, lowCol::orange.r, lowCol::orange.g, lowCol::orange.b, 0xff);

		setFontSize(11);
		drawTextCenter(sysStr[164], topWindow.x + 30 + 54 * 1, topWindow.y + 24 + 18 * -1 + 9);//��������
		drawTextCenter(sysStr[165], topWindow.x + 30 + 54 * 2, topWindow.y + 24 + 18 * -1 + 9);//��������
		drawTextCenter(sysStr[166], topWindow.x + 30 + 54 * 3, topWindow.y + 24 + 18 * -1 + 9);//Ÿ������
		drawTextCenter(sysStr[167], topWindow.x + 30 + 54 * 4, topWindow.y + 24 + 18 * -1 + 9);//���ص�

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
		drawText(col2Str(lowCol::orange)+ sysStr[168], topWindow.x + 290, topWindow.y + 24 + 18 * -1);//���
		drawText(col2Str(lowCol::orange) + sysStr[169], topWindow.x + 290, topWindow.y + 24 + 18 * 0);//ȸ��

		drawText(col2Str(col::lightGray) + sysStr[170], topWindow.x + 290, topWindow.y + 24 + 18 * 1);//ȭ������
		drawText(col2Str(col::lightGray) + sysStr[171], topWindow.x + 290, topWindow.y + 24 + 18 * 2);//�ñ�����
		drawText(col2Str(col::lightGray) + sysStr[172], topWindow.x + 290, topWindow.y + 24 + 18 * 3);//��������
		drawText(col2Str(col::lightGray) + sysStr[173], topWindow.x + 290, topWindow.y + 24 + 18 * 4);//��������
		drawText(col2Str(col::lightGray) + sysStr[174], topWindow.x + 290, topWindow.y + 24 + 18 * 5);//�ν�����

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