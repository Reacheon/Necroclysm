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

static int delayR2 = 0;

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

		changeXY(cameraW - 335, (cameraH / 2) - 210, false);
		setAniSlipDir(0);

		tabType = tabFlag::closeWin;
		UIType = act::loot;

		lootPtr = World::ins()->getItemPos(lootTile[axis::x], lootTile[axis::y], Player::ins()->getGridZ())->getPocket();
		lootPtr->sortByUnicode();

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);

		prt(L"item�� ũ��� %d�Դϴ�.\n", sizeof(ItemData));

		if (inputType == input::keyboard || inputType == input::gamepad)
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
	void drawGUI();
	void clickUpGUI();
	void clickMotionGUI(int dx, int dy);
	void clickDownGUI();
	void gamepadBtnDown();
	void gamepadBtnMotion();
	void gamepadBtnUp();
	void step()
	{
		if (SDL_NumJoysticks() > 0)
		{
			if (delayR2 <= 0 && SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 1000)
			{
				prt(L"���� ����Ǿ���.\n");
				executeTab();
				delayR2 = 20;
			}
			else delayR2--;
		}

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
