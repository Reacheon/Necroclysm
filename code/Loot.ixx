#include <SDL.h>
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module Loot;

import std;
import util;
import globalVar;
import constVar;
import drawWindowArrow;
import ItemPocket;
import drawItemList;
import checkCursor;
import drawSprite;
import drawText;
import Player;
import World;
import textureVar;
import log;
import ItemStack;
import Msg;
import GUI;
import actFuncSet;
import drawWindow;
import Lst;

export class Loot : public GUI
{
private:
	inline static Loot* ptr = nullptr;
	ItemPocket* lootPtr = nullptr;
	const int lootScrollSize = 6; //�� ��ũ�ѿ� ���� �������� ��
	int lootScroll = 0; //���� ����â�� ��ũ��
	int lootCursor = -1; //���� ����â�� Ŀ��
	int pocketCursor = 0; //���� ����� ���� ���õ� ����, EQUIP�� ������ ������ ������� 0,1,2...
	//��� ��ũ�� �̺�Ʈ���� �������� ��� ���� ��ġ ����� ��ũ��
	int initLootScroll = 0; //��ǽ�ũ���� ���۵Ǳ� ������ ��ũ��
	int initPocketCursor = 0;
	int labelCursor = -1; //��� �� Ŀ��, 0���� 2���� ���, -1�̸� ��Ȱ��ȭ
	Uint32 selectTouchTime = -1;
	sortFlag sortType = sortFlag::null; //0:�⺻_1:���Գ���_2:���Կø�_3:���ǳ���_4:���ǿø�

	SDL_Rect lootBase;
	SDL_Rect lootTitle;
	SDL_Rect lootItem[30];
	SDL_Rect lootItemSelect[30];
	SDL_Rect lootLabel;
	SDL_Rect lootLabelSelect;
	SDL_Rect lootLabelName;
	SDL_Rect lootLabelQuantity;
	SDL_Rect lootArea;
	SDL_Rect lootScrollBox;
	SDL_Rect lootWindow;
	SDL_Rect pocketWeight;
	SDL_Rect pocektVolume;
	SDL_Rect pocketWindow;
	SDL_Rect pocketItem[7];
	SDL_Rect pocketArea;
	SDL_Rect pocketScrollBox;
	SDL_Rect pocketLeft;
	SDL_Rect pocketRight;
	SDL_Rect lootBtn;
	SDL_Rect topWindow;
public:
	Corouter errorFunc();

	Loot(int targetGridX, int targetGridY) : GUI(false)
	{
		prt(L"Loot : �����ڰ� �����Ǿ����ϴ�..\n");
		prt(L"���� loot�� ptr ������ %p�Դϴ�.\n", ptr);
		//errorBox(ptr != nullptr, "More than one Loot instance was generated.");
		ptr = this;
		lootTile[axis::x] = targetGridX;
		lootTile[axis::y] = targetGridY;
		lootTile[axis::z] = Player::ins()->getGridZ();

		changeXY((cameraW / 2) + 17, (cameraH / 2) - 210, false);
		setAniSlipDir(0);

		tabType = tabFlag::closeWin;
		UIType = act::loot;

		lootPtr = World::ins()->getItemPos(lootTile[axis::x], lootTile[axis::y], Player::ins()->getGridZ())->getPocket();
		lootPtr->sortByUnicode();

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);

		prt(L"item�� ũ��� %d�Դϴ�.\n", sizeof(ItemData));

		if (inputType == input::keyboard)
		{
			lootCursor = 0;
		}
	}
	~Loot()
	{
		prt(L"Loot : �Ҹ��ڰ� ȣ��Ǿ����ϴ�..\n");
		ptr = nullptr;

		UIType = act::null;
		lootCursor = -1;
		lootScroll = 0;
		for (int i = 0; i < lootPtr->itemInfo.size(); i++) { lootPtr->itemInfo[i].lootSelect = 0; }
		tabType = tabFlag::autoAtk;
		barAct = actSet::null;
		barActCursor = -1;
	}
	static Loot* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		lootBase = { 0,0,335,420 };
		if (center == false)
		{
			lootBase.x += inputX;
			lootBase.y += inputY;
		}
		else
		{
			lootBase.x += inputX - lootBase.w / 2;
			lootBase.y += inputY - lootBase.h / 2;
		}
		lootTitle = { lootBase.x + 102, lootBase.y + 0, 130, 30 };
		lootWindow = { lootBase.x + 0, lootBase.y + 120, 335, 300 };
		lootArea = { lootWindow.x + 10, lootWindow.y + 40,312, 246 };
		for (int i = 0; i < lootItemMax; i++)
		{
			lootItem[i] = { lootArea.x + 42, lootArea.y + 42 * i, 270, 36 };
			lootItemSelect[i] = { lootArea.x, lootArea.y + 42 * i, 36, 36 };
		}
		lootLabel = { lootWindow.x + 10, lootWindow.y + 10, lootWindow.w - 20 , 26 };
		lootLabelSelect = { lootLabel.x, lootLabel.y, 62 , 26 };
		lootLabelName = { lootLabel.x + lootLabelSelect.w, lootLabel.y, 182 , 26 };
		lootLabelQuantity = { lootLabel.x + lootLabelName.w + lootLabelSelect.w, lootLabel.y, 71 , 26 };
		lootScrollBox = { lootWindow.x + 328, lootWindow.y + 40, 2, 42 * lootItemMax };
		pocketWindow = { lootBase.x + 0, lootBase.y + 34, 335, 70 };
		pocketWeight = { pocketWindow.x + 12, pocketWindow.y + 64, 72, 4 };
		pocektVolume = { pocketWindow.x + pocketWindow.w - 12 - 72, pocketWindow.h + 64, 72, 4 };
		pocketItem[0] = { pocketWindow.x + (pocketWindow.w / 2) - 24 - 38 * 3,pocketWindow.y + 11,32,32 };
		pocketItem[1] = { pocketWindow.x + (pocketWindow.w / 2) - 24 - 38 * 2,pocketWindow.y + 11,32,32 };
		pocketItem[2] = { pocketWindow.x + (pocketWindow.w / 2) - 24 - 38,pocketWindow.y + 11,32,32 };
		pocketItem[3] = { pocketWindow.x + (pocketWindow.w / 2) - 24,pocketWindow.y + 11 - 8,48,48 };
		pocketItem[4] = { pocketWindow.x + (pocketWindow.w / 2) - 8 + 38,pocketWindow.y + 11,32,32 };
		pocketItem[5] = { pocketWindow.x + (pocketWindow.w / 2) - 8 + 38 * 2,pocketWindow.y + 11,32,32 };
		pocketItem[6] = { pocketWindow.x + (pocketWindow.w / 2) - 8 + 38 * 3,pocketWindow.y + 11,32,32 };
		pocketLeft = { pocketWindow.x + 4,pocketWindow.y + 6, 24,44 };
		pocketRight = { pocketWindow.x + pocketWindow.w - pocketLeft.w - 4,pocketWindow.y + 6,24,44 };

		lootBtn = { lootWindow.x + lootWindow.w / 2 - 28, lootWindow.y - 2 - 16 + 2, 56, 24 };

		topWindow = { 0, 0, 410,140 };
		topWindow.x = (cameraW / 2) - (topWindow.w / 2);

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - lootBase.w / 2;
			y = inputY - lootBase.h / 2;
		}
	}
	void drawGUI()
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

				drawSpriteCenter(spr::lootBagArrow, 1, lootWindow.x + lootWindow.w / 2, lootWindow.y - 4);

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

				if (inputType == input::keyboard)
				{
					if (state[SDL_SCANCODE_V]) { targetBtnSpr = spr::buttonsPressed; }
					else { targetBtnSpr = spr::buttons; }
					drawSpriteCenter(targetBtnSpr, keyIcon::keyboard_V, lootBtn.x, lootBtn.y + lootBtn.h / 2);
				}
			}
		}


	}
	void clickUpGUI()
	{
		if (checkCursor(&tab) == true)// �ǹڽ�
		{
			executeTab();
			return;
		}
		else if (checkCursor(&lootArea)) //������ Ŭ�� -> ���� ��Ʈ
		{
			//���� �������� Ŭ�������� Ŀ���� �� ���������� �ű�, �ٸ� �� ������ -1�� �ٲ�
			for (int i = 0; i < lootItemMax; i++)
			{
				if (lootPtr->itemInfo.size() - 1 >= i)
				{
					if (checkCursor(&lootItem[i]))
					{
						if (lootCursor != lootScroll + i) //���ο� Ŀ�� ����
						{
							lootCursor = lootScroll + i;
							updateBarAct();
							tabType = tabFlag::back;
						}
						else //Ŀ�� ����
						{
							lootCursor = -1;
							barAct = actSet::null;
							tabType = tabFlag::closeWin;
						}
						return;
					}
				}
			}

			//������ ���� ����Ʈ Ŭ��
			for (int i = 0; i < lootItemMax; i++)
			{
				if (checkCursor(&lootItemSelect[i]))
				{
					if (lootPtr->itemInfo.size() - 1 >= i)
					{
						if (lootPtr->itemInfo[i + lootScroll].lootSelect == 0)
						{
							if (inputType == input::mouse)
							{
								if (event.button.button == SDL_BUTTON_LEFT)
								{
									executeSelectItem(i + lootScroll);
								}
								else if (event.button.button == SDL_BUTTON_RIGHT)
								{
									CORO(executeSelectItemEx(i + lootScroll));
								}
							}
							else if (inputType == input::touch)
							{
								executeSelectItem(i + lootScroll);
							}
						}
						else
						{
							lootPtr->itemInfo[i + lootScroll].lootSelect = 0;
						}
					}
				}
			}
		}
		else if (checkCursor(&lootLabel))
		{
			if (checkCursor(&lootLabelSelect))
			{
				executeSelectAll();
			}
			else if (checkCursor(&lootLabelName))
			{
				CORO(executeSearch());
				//lootPtr->sortPocket(sortFlag::null);
				//lootScroll = 0;
			}
			else if (checkCursor(&lootLabelQuantity))
			{
				executeSort();
			}
		}
		else if (checkCursor(&pocketLeft))
		{
			if (pocketCursor != 0) { pocketCursor--; }
		}
		else if (checkCursor(&pocketRight))
		{
			int numberOfBag = 0;
			ItemPocket* equipPtr = Player::ins()->getEquipPtr();
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
				{
					numberOfBag++;
				}
			}
			if (pocketCursor != numberOfBag - 1) { pocketCursor++; }
		}
		else if (checkCursor(&lootBtn))
		{
			executePickSelect();
		}
		else if (checkCursor(&letterbox)) //��ư�� return ����
		{
			for (int i = 0; i < barAct.size(); i++) // �ϴ� UI ��ġ �̺�Ʈ
			{
				if (checkCursor(&barButton[i]))
				{
					switch (barAct[i])
					{
					case act::pick://�ֱ�
						executePick();
						break;
					case act::equip://���
						executeEquip();
						break;
					case act::wield://���
						CORO(executeWield());
						break;
						//case act::insert:
						//	CORO(executeInsert());
						//	break;
					case act::reloadBulletToMagazine:
					case act::reloadBulletToGun:
						if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::MAGAZINE))
						{
							CORO(actFunc::reloadSelf(actEnv::Loot, lootPtr, lootCursor));
						}
						else if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::AMMO))
						{
							CORO(actFunc::reloadOther(actEnv::Loot, lootPtr, lootCursor));
						}
						else if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::GUN))
						{
							CORO(actFunc::reloadSelf(actEnv::Equip, lootPtr, lootCursor));
						}
						break;
					case act::reloadMagazine:
						//�ѿ��� ����ϴ� ���� źâ���� ����ϴ� ��찡 �ٸ�
						//�ѿ��� ����ϸ� �ڱ� �ڽſ��� ������(self)
						//źâ�� ����ϸ� �ٸ� Ÿ���� �ѿ��� �����
						if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::MAGAZINE))
						{
							CORO(actFunc::reloadOther(actEnv::Loot, lootPtr, lootCursor));
						}
						else
						{
							CORO(actFunc::reloadSelf(actEnv::Loot, lootPtr, lootCursor));
						}
						break;
					case act::unloadMagazine:
					case act::unloadBulletFromMagazine:
					case act::unloadBulletFromGun:
						actFunc::unload(lootPtr, lootCursor);
						break;
					}
				}
			}
		}

		//���� ��� ��쿡�� return�� ���� �������� ��ư �ܸ̿� ���� ���̹Ƿ� Ŀ���� -1�� ����
		{
			lootCursor = -1;
			barAct = actSet::null;
			tabType = tabFlag::closeWin;
		}
	}
	void clickMotionGUI(int dx, int dy)
	{
		if (checkCursor(&lootBase))
		{
			if (click == true)
			{
				int scrollAccelConst = 20; // ���ӻ��, �۾������� ��ũ�� �ӵ��� ������
				lootScroll = initLootScroll + dy / scrollAccelConst;
				if (abs(dy / scrollAccelConst) >= 1)
				{
					deactClickUp = true;
					cursorMotionLock = true;
				}
			}
		}
	}
	void clickDownGUI()
	{
		//������ ���� ����Ʈ Ŭ��
		selectTouchTime = SDL_GetTicks();
		initLootScroll = lootScroll;
		initPocketCursor = pocketCursor;
	}
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step()
	{
		//����Ʈ Ȧ�� �̺�Ʈ
		if (coFunc == nullptr)
		{
			if (selectTouchTime != -1)
			{
				//������ ���� ����Ʈ Ŭ��
				for (int i = 0; i < lootItemMax; i++)
				{
					if (checkCursor(&lootItemSelect[i]))
					{
						if (lootPtr->itemInfo.size() - 1 >= i)
						{
							if (SDL_GetTicks() - selectTouchTime > 800)
							{
								selectTouchTime = -1;
								CORO(executeSelectItemEx(i + lootScroll));
							}
						}
						break;
					}

					if (i == lootItemMax - 1)
					{
						selectTouchTime = -1;
					}
				}
			}
		}

		//�߸��� Ŀ�� ��ġ ����
		if (lootCursor > (int)(lootPtr->itemInfo.size() - 1)) { lootCursor = lootPtr->itemInfo.size() - 1; }

		//�߸��� ��ũ�� ��ġ ����
		if (lootScroll + lootItemMax >= lootPtr->itemInfo.size()) { lootScroll = myMax(0, (int)lootPtr->itemInfo.size() - lootItemMax); }
		else if (lootScroll < 0) { lootScroll = 0; }

		//��Ʈ �������� ������ 0�� �Ǿ��� ��� â�� ����
		if (lootPtr->itemInfo.size() == 0)
		{
			close(aniFlag::null);
			//Ŭ���� ���� �ִϸ��̼��� ������ �ȴ�. �ִϸ��̼��� ��� ����ǰ� �����ؾߵ�
			delete World::ins()->getItemPos(lootTile[axis::x], lootTile[axis::y], Player::ins()->getGridZ());
			return;
		}
	}

	sortFlag getSortType() { return sortType; }
	//�� Ű�� ������ ���� ����
	void executeTab()
	{
		if (lootCursor == -1) //�������� ���� ������ ���� ���
		{
			close(aniFlag::winSlipClose);
		}
		else
		{
			if (inputType != input::keyboard)
			{
				lootCursor = -1;
				barAct = actSet::null;
				tabType = tabFlag::closeWin;
			}
			else
			{
				close(aniFlag::winSlipClose);
			}
		}
	}
	void executeSort()
	{
		switch (sortType)
		{
		default:
			errorBox("undefined sorting in Loot.ixx");
			break;
		case sortFlag::null:
			lootPtr->sortWeightDescend();
			sortType = sortFlag::weightDescend;
			break;
		case sortFlag::weightDescend:
			lootPtr->sortWeightAscend();
			sortType = sortFlag::weightAscend;
			break;
		case sortFlag::weightAscend:
			lootPtr->sortVolumeDescend();
			sortType = sortFlag::volumeDescend;
			break;
		case sortFlag::volumeDescend:
			lootPtr->sortVolumeAscend();
			sortType = sortFlag::volumeAscend;
			break;
		case sortFlag::volumeAscend:
			lootPtr->sortByUnicode();
			sortType = sortFlag::null;
			break;
		}
		lootScroll = 0;
	}
	void executePickSelect()
	{
		//1. ���� Ŀ���� ����Ű�� �������� Array�� �ܿ����ǿ� �÷��̾��� ���� �Ѱ踦 ����
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		ItemPocket* bagPtr = nullptr;
		int bagIndex = -1;
		int bagNumber = 0;

		if (equipPtr->itemInfo.size() == 0)
		{
			updateLog(col2Str(col::white) + sysStr[123]);
			return;
		}

		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			//������ ���
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0) { bagNumber++; }

			//Ŀ���� ����Ű�� ������ �ּҸ� ����
			if (bagNumber - 1 == pocketCursor)
			{
				bagPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
				bagIndex = i;
			}

			//������ ã�� ������ ���
			if (i == equipPtr->itemInfo.size() - 1 && bagPtr == nullptr)
			{
				updateLog(col2Str(col::white) + sysStr[123]);
				return;
			}
		}

		int currentVol = equipPtr->itemInfo[bagIndex].pocketVolume;
		int maxVol = equipPtr->itemInfo[bagIndex].pocketMaxVolume;

		int itemsVol = 0;
		for (int i = 0; i < lootPtr->itemInfo.size(); i++)
		{
			if (lootPtr->itemInfo[i].lootSelect > 0)
			{
				itemsVol += lootPtr->itemInfo[i].lootSelect * lootPtr->itemInfo[i].volume;
			}
		}

		if (maxVol < itemsVol + currentVol)
		{
			updateLog(col2Str(col::white) + sysStr[124]);
			return;
		}
		else
		{
			int fixedLootSize = lootPtr->itemInfo.size();
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (lootPtr->itemInfo[i].lootSelect > 0)
				{
					lootPtr->transferItem(bagPtr, i, lootPtr->itemInfo[i].lootSelect);
				}
			}
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--) { lootPtr->itemInfo[i].lootSelect = 0; }
		}
	}
	void executeSelectAll()
	{
		bool isSelectAll = true;
		for (int i = 0; i < lootPtr->itemInfo.size(); i++)
		{
			if (lootPtr->itemInfo[i].lootSelect != lootPtr->itemInfo[i].number)
			{
				isSelectAll = false;
				break;
			}
		}

		if (isSelectAll == false)
		{
			for (int i = 0; i < lootPtr->itemInfo.size(); i++)
			{
				lootPtr->itemInfo[i].lootSelect = lootPtr->itemInfo[i].number;
			}
		}
		else
		{
			for (int i = 0; i < lootPtr->itemInfo.size(); i++)
			{
				lootPtr->itemInfo[i].lootSelect = 0;
			}
		}
	}
	void executeSelectItem(int index)
	{
		int itemNumber = lootPtr->itemInfo[index].number;
		lootPtr->itemInfo[index].lootSelect = itemNumber;
	}


	void executePocketLeft()
	{
		if (pocketCursor != 0) { pocketCursor--; }
	}
	void executePocketRight()
	{
		int numberOfBag = 0;
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
			{
				numberOfBag++;
			}
		}
		if (pocketCursor != numberOfBag - 1) { pocketCursor++; }
	}
	//act
	void executePick()
	{
		//pick �����ϱ� ���� select ��� ���� �����ؾߵ�
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
			updateLog(col2Str(col::white) + sysStr[123]);
			return;
		}
		else
		{
			//���� ����Ű�� lootCursor�� select�� Ǯ�� ����
			int itemNumber = lootPtr->itemInfo[lootCursor].number;
			lootPtr->itemInfo[lootCursor].lootSelect = itemNumber;
			//���� ����&���� üũ �߰����� �ʾ���
			lootPtr->transferItem((ItemPocket*)equipPtr->itemInfo[pocketList[pocketCursor]].pocketPtr, lootCursor, lootPtr->itemInfo[lootCursor].lootSelect);
			if (inputType == input::keyboard)
			{
				doPopDownHUD = true;
				barActCursor = -1;
			}
		}
	}
	void executeEquip()
	{
		updateLog(col2Str(col::white) + sysStr[125]);
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		int returnIndex = lootPtr->transferItem(Player::ins()->getEquipPtr(), lootCursor, 1);
		equipPtr->itemInfo[returnIndex].equipState = equip::normal;
		Player::ins()->updateStatus();
		if (inputType == input::keyboard)
		{
			doPopDownHUD = true;
			barActCursor = -1;
		}
	}

	void updateBarAct()
	{
		if (lootPtr->itemInfo.size() > 0)
		{
			ItemData targetItem = lootPtr->itemInfo[lootCursor];
			barAct.clear();
			barAct.push_back(act::wield);

			//������Ʈ�� �������� ���� ���
			if (targetItem.checkFlag(itemFlag::GUN))
			{
				//���� �������� źâ�� ���(�Ϲ� ����)
				if (itemDex[targetItem.pocketOnlyItem[0]].checkFlag(itemFlag::MAGAZINE))
				{
					ItemPocket* gunPtr = ((ItemPocket*)targetItem.pocketPtr);

					if (gunPtr->itemInfo.size() == 0)
					{
						barAct.push_back(act::reloadMagazine);
					}
					else
					{
						barAct.push_back(act::unloadMagazine);
					}
				}
				//���� �������� ź�� ���(��������)
				else if (itemDex[targetItem.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
				{
					ItemPocket* gunPtr = ((ItemPocket*)targetItem.pocketPtr);
					//źȯ �и�
					if (gunPtr->itemInfo.size() > 0)
					{
						barAct.push_back(act::unloadBulletFromGun);
					}

					//źȯ ����
					int bulletNumber = 0;
					for (int i = 0; i < gunPtr->itemInfo.size(); i++)
					{
						bulletNumber += gunPtr->itemInfo[i].number;
					}

					if (bulletNumber < targetItem.pocketMaxNumber)
					{
						barAct.push_back(act::reloadBulletToGun);
					}
				}
			}
			//������Ʈ�� �������� źâ�� ���
			else if (targetItem.checkFlag(itemFlag::MAGAZINE))
			{
				barAct.push_back(act::reloadMagazine);

				//źâ ����
				ItemPocket* magazinePtr = ((ItemPocket*)targetItem.pocketPtr);
				if (magazinePtr->itemInfo.size() > 0)
				{
					barAct.push_back(act::unloadBulletFromMagazine);
				}

				//�Ѿ� ����
				int bulletNumber = 0;
				for (int i = 0; i < magazinePtr->itemInfo.size(); i++)
				{
					bulletNumber += magazinePtr->itemInfo[i].number;
				}

				if (bulletNumber < targetItem.pocketMaxNumber)
				{
					barAct.push_back(act::reloadBulletToMagazine);
				}
			}
			//������Ʈ�� �������� źȯ�� ���
			else if (targetItem.checkFlag(itemFlag::AMMO))
			{
				barAct.push_back(act::reloadBulletToGun);
			}

			if (targetItem.checkFlag(itemFlag::CANEQUIP) == true) { barAct.push_back(act::equip); }
		}
	}

	//���ڷ�ƾ �Լ���
	Corouter executeSearch()
	{
		//�̹� �˻� ������ üũ
		for (int i = 0; i < lootPtr->itemInfo.size(); i++)
		{
			if (lootPtr->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))//�̹� �˻� ���� ��� �˻� ���¸� ������
			{
				for (int j = 0; j < lootPtr->itemInfo.size(); j++)
				{
					lootPtr->itemInfo[j].eraseFlag(itemFlag::GRAYFILTER);
				}
				lootPtr->sortByUnicode();
				updateLog(col2Str(col::white) + sysStr[86]);//�˻� ���¸� �����ߴ�.
				co_return;
			}

			if (i == lootPtr->itemInfo.size() - 1)//�˻� ���� �ƴ� ���
			{
				std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//Ȯ��, ���
				new Msg(msgFlag::input, sysStr[27], sysStr[97], choiceVec);//�˻�, �˻��� Ű���带 �Է����ּ���
				lootScroll = 0;
				co_await std::suspend_always();
				if (coAnswer == sysStr[38])
				{
					int matchCount = lootPtr->searchTxt(exInputText);
					for (int i = 0; i < lootPtr->itemInfo.size(); i++) lootPtr->itemInfo[i].addFlag(itemFlag::GRAYFILTER);
					for (int i = 0; i < matchCount; i++) lootPtr->itemInfo[i].eraseFlag(itemFlag::GRAYFILTER);
				}
				else {}
			}
		}
	}

	Corouter executeSelectItemEx(int index)
	{
		//�Է��� �޽��� �ڽ� ����
		std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//Ȯ��, ���
		new Msg(msgFlag::input, sysStr[40], sysStr[39], choiceVec);//������ ����, �󸶳�?
		co_await std::suspend_always();

		int inputSelectNumber = wtoi(exInputText.c_str()); // ����Ʈ �ڽ��� �־��� ����(�÷��̾ �Է��� ��)
		if (inputSelectNumber > lootPtr->itemInfo[index].number) //���� ���� �ִ� ���ں��� ���� ���� �Է����� ���
		{
			inputSelectNumber = lootPtr->itemInfo[index].number; //Select�� ���� �ִ����� ����
		}
		lootPtr->itemInfo[index].lootSelect = inputSelectNumber;
	}


	Corouter executeWield()
	{
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::TWOHANDED)) //�������� ���
		{
			bool isWield = false;
			ItemPocket* drop = new ItemPocket(storageType::null);
			for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (equipPtr->itemInfo[i].equipState == equip::left || equipPtr->itemInfo[i].equipState == equip::right || equipPtr->itemInfo[i].equipState == equip::both)
				{
					equipPtr->transferItem(drop, i, 1);
					isWield = true;
				}
			}
			if (isWield == true) { Player::ins()->drop(drop); }
			else { delete drop; }
			int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
			equipPtr->itemInfo[returnIndex].equipState = equip::both; //���
			equipPtr->sortEquip();
			updateLog(L"#FFFFFF�������� �����.");
		}
		else
		{
			bool hasLeft = false;
			bool hasRight = false;
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				switch (equipPtr->itemInfo[i].equipState)
				{
				case equip::left:
					hasLeft = true;
					break;
				case equip::right:
					hasRight = true;
					break;
				case equip::both:
					hasLeft = true;
					hasRight = true;
					break;
				}
			}

			if (hasLeft == true && hasRight == true)
			{
				int fixedLootCursor = lootCursor;//Msg�� ������ lootCursor�� -1�� �ʱ�ȭ�Ǳ⿡ �̸� ���������� ����

				//�޼�, ������
				std::vector<std::wstring> choiceVec = { sysStr[49], sysStr[50] };
				//����, ��� �տ� ���?
				new Msg(msgFlag::normal, sysStr[98], sysStr[99], choiceVec);
				co_await std::suspend_always();

				int handDir = 0;
				if (coAnswer == sysStr[49])//�޼�
				{
					handDir = equip::left;
				}
				else//������
				{
					handDir = equip::right;
				}

				//�޼� ������ ������
				ItemPocket* drop = new ItemPocket(storageType::null);
				for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
				{
					if (equipPtr->itemInfo[i].equipState == handDir)
					{
						equipPtr->transferItem(drop, i, 1);
						break;
					}
				}
				//��� ������ ������
				for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
				{
					if (equipPtr->itemInfo[i].equipState == equip::both)
					{
						equipPtr->transferItem(drop, i, 1);
						break;
					}
				}
				Player::ins()->drop(drop);

				int returnIndex = lootPtr->transferItem(equipPtr, fixedLootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = handDir;
				equipPtr->sortEquip();
			}
			else if (hasLeft == false && hasRight == false)
			{
				int fixedLootCursor = lootCursor;//Msg�� ������ lootCursor�� -1�� �ʱ�ȭ�Ǳ⿡ �̸� ���������� ����

				//�޼�, ������
				std::vector<std::wstring> choiceVec = { sysStr[49], sysStr[50] };
				//����, ��� �տ� ���?
				new Msg(msgFlag::normal, sysStr[98], sysStr[99], choiceVec);
				co_await std::suspend_always();

				int handDir = 0;
				if (coAnswer == sysStr[49])//�޼�
				{
					handDir = equip::left;
				}
				else//������
				{
					handDir = equip::right;
				}

				int returnIndex = lootPtr->transferItem(equipPtr, fixedLootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = handDir;
				equipPtr->sortEquip();
			}
			else if (hasLeft == false && hasRight == true)//�޼տ� ���
			{
				int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = equip::left;
				equipPtr->sortEquip();
				updateLog(L"#FFFFFF�������� �����.");
			}
			else//�����տ� ���
			{
				int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = equip::right;
				equipPtr->sortEquip();
				updateLog(L"#FFFFFF�������� �����.");
			}
		}
		Player::ins()->updateStatus();
		if (inputType == input::keyboard)
		{

			doPopDownHUD = true;
			barActCursor = -1;
		}
	}


	Corouter executeInsert()//��ź : �Ѿ˿� ���, �� źȯ�� ���� �� �ִ� źâ ����Ʈ�� ǥ���ϰ� �ű⿡ ����
	{
		int targetLootCursor = lootCursor;
		std::vector<std::wstring> pocketList;
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0 || equipPtr->itemInfo[i].pocketMaxNumber > 0)
			{
				if (equipPtr->itemInfo[i].pocketMaxNumber > 0) //���� �������� ���� ���
				{
					if (std::find(equipPtr->itemInfo[i].pocketOnlyItem.begin(), equipPtr->itemInfo[i].pocketOnlyItem.end(), lootPtr->itemInfo[targetLootCursor].itemCode) == equipPtr->itemInfo[i].pocketOnlyItem.end())
					{
						//��ƮĿ���� �ش� �������� ���� �� ���� �����̸� continue
						continue;
					}
				}

				pocketList.push_back(equipPtr->itemInfo[i].name);
			}
		}

		if (pocketList.size() == 0) //�������� ������ ã�� ������ ���
		{
			//�� �������� �������� ������ ����.
			updateLog(col2Str(col::white) + sysStr[96]);
			co_return;
		}

		////////////////////////////////////////////////////////////////////

		new Lst(sysStr[95], sysStr[94], pocketList);//�ֱ�, ���� ������ �������ּ���.
		co_await std::suspend_always();

		////////////////////////////////////////////////////////////////////

		int counter = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0 || equipPtr->itemInfo[i].pocketMaxNumber > 0)
			{
				if (equipPtr->itemInfo[i].pocketMaxNumber > 0) //���� �������� ���� ���
				{
					if (std::find(equipPtr->itemInfo[i].pocketOnlyItem.begin(), equipPtr->itemInfo[i].pocketOnlyItem.end(), lootPtr->itemInfo[targetLootCursor].itemCode) == equipPtr->itemInfo[i].pocketOnlyItem.end())
					{
						//��ƮĿ���� �ش� �������� ���� �� ���� �����̸� continue
						continue;
					}
				}

				if (counter == wtoi(coAnswer.c_str()))
				{

					int transferNumber;
					if (lootPtr->itemInfo[targetLootCursor].lootSelect != 0)
					{
						transferNumber = lootPtr->itemInfo[targetLootCursor].lootSelect;
						lootPtr->itemInfo[targetLootCursor].lootSelect = 0;
					}
					else { transferNumber = lootPtr->itemInfo[targetLootCursor].number; }

					//�� �� ������?
					lootPtr->transferItem
					(
						(ItemPocket*)equipPtr->itemInfo[i].pocketPtr,
						targetLootCursor,
						transferNumber
					);

					co_return;
				}

				counter++;
			}
		}
	}
};
