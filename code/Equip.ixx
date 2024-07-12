#include <SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();


export module Equip;

import std;
import util;
import Player;
import drawText;
import globalVar;
import textureVar;
import ItemPocket;
import checkCursor;
import drawSprite;
import drawItemList;
import drawWindowArrow;
import GUI;
import log;
import Msg;
import actFuncSet;
import drawWindow;
import CoordSelect;
import Lst;

export class Equip : public GUI
{
private:
	inline static Equip* ptr = nullptr;
	ItemPocket* equipPtr = Player::ins()->getEquipPtr();

	int equipScroll = 0; //좌측 장비창의 스크롤
	int equipCursor = -1; //좌측 장비창의 커서
	const int equipScrollSize = 8;

	ItemPocket* lootPtr = nullptr;
	const int lootScrollSize = 6; //한 스크롤에 들어가는 아이템의 수
	int lootScroll = 0; //우측 루팅창의 스크롤
	int lootCursor = -1; //우측 루팅창의 커서
	int pocketCursor = -1; //우측 상단의 현재 선택된 가방, EQUIP의 가방의 위부터 순서대로 0,1,2...
	//모션 스크롤 이벤트에서 기준으로 잡는 최초 터치 당시의 스크롤
	int initLootScroll = 0;
	int initPocketCursor = 0;
	int labelCursor = -1; //상단 라벨 커서, 0부터 2까지 기능, -1이면 비활성화
	Uint32 selectTouchTime = -1;

	sortFlag sortType = sortFlag::null; //0:기본_1:무게내림_2:무게올림_3:부피내림_4:부피올림

	bool isTargetPocket = false; //우측의 포켓창이 조작 타겟일 경우

	SDL_Rect equipBase;
	SDL_Rect equipTitle;
	SDL_Rect equipItem[30];
	SDL_Rect equipItemSelect[30];
	SDL_Rect equipLabel;
	SDL_Rect equipLabelSelect;
	SDL_Rect equipLabelName;
	SDL_Rect equipLabelQuantity;
	SDL_Rect equipArea;
	SDL_Rect equipScrollBox;
	SDL_Rect equipWindow;

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

	SDL_Rect topWindow;//상단에 표시되는 저항이나 방어 상성, 아이템의 설명
public:
	Equip() : GUI(false)
	{
		errorBox(ptr != nullptr, "More than equip instance was generated.");
		ptr = this;

		changeXY((cameraW / 2) - 352, (cameraH / 2) - 210, false);
		setAniSlipDir(4);

		//barAct = actSet::null;
		tabType = tabFlag::closeWin;
		UIType = act::equip;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winSlipOpen);

		if (inputType == input::keyboard)
		{
			equipCursor = 0;
		}
	}
	~Equip()
	{
		prt(L"Equip : 소멸자가 호출되었습니다..\n");
		ptr = nullptr;

		UIType = act::null;
		equipCursor = -1;
		equipScroll = 0;
		barAct = actSet::null;
		tabType = tabFlag::autoAtk;
	}
	static Equip* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		equipBase = { 0,0,335,420 };
		if (center == false)
		{
			equipBase.x += inputX;
			equipBase.y += inputY;
		}
		else
		{
			equipBase.x += inputX - equipBase.w / 2;
			equipBase.y += inputY - equipBase.h / 2;
		}

		equipTitle = { equipBase.x + 103, equipBase.y, 130, 30 };
		equipWindow = { equipBase.x, equipBase.y + 30, 335, 380 };
		equipLabel = { equipWindow.x + 10, equipWindow.y + 10, equipWindow.w - 20 , 26 };
		equipLabelSelect = { equipLabel.x, equipLabel.y, 62 , 26 };
		equipLabelName = { equipLabel.x + equipLabelSelect.w, equipLabel.y, 182 , 26 };
		equipLabelQuantity = { equipLabel.x + equipLabelName.w + equipLabelSelect.w, equipLabel.y, 71 , 26 };
		equipArea = { equipWindow.x + 10, equipWindow.y + 40,312, 42 * 8 - 6 };
		for (int i = 0; i < equipItemMax; i++)
		{
			equipItem[i] = { equipArea.x + 42, equipArea.y + 42 * i, 270, 36 };
			equipItemSelect[i] = { equipArea.x, equipArea.y + 42 * i, 36, 36 };
		}
		equipScrollBox = { equipWindow.x + 328, equipWindow.y + 40, 2, 42 * equipItemMax };

		//////////////////////////////////////////////////////////////////////////////////////////////////

		int lootPivotX = (cameraW / 2) + 17;
		int lootPivotY = (cameraH / 2) - 210;
		lootBase = { 0,0,335,420 };
		if (center == false)
		{
			lootBase.x += lootPivotX;
			lootBase.y += lootPivotY;
		}
		else
		{
			lootBase.x += lootPivotX - lootBase.w / 2;
			lootBase.y += lootPivotY - lootBase.h / 2;
		}
		lootTitle = { lootBase.x + 102, lootBase.y + 0, 130, 30 };
		lootWindow = { lootBase.x + 0, lootBase.y + 120, 335, 300 };
		lootArea = { lootWindow.x + 10, lootWindow.y + 40,312, 246 };
		for (int i = 0; i < lootItemMax; i++)
		{
			lootItem[i] = { lootArea.x + 42, lootArea.y + 42 * i, 270, 36 };
			lootItemSelect[i] = { lootArea.x, lootArea.y + 42 * i, 236, 36 };
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
			x = inputX - equipBase.w / 2;
			y = inputY - equipBase.h / 2;
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
		//셀렉트 홀드 이벤트
		if (coFunc == nullptr)
		{
			if (selectTouchTime != -1)
			{
				//아이템 좌측 셀렉트 클릭
				for (int i = 0; i < lootItemMax; i++)
				{
					if (checkCursor(&lootItemSelect[i]))
					{
						if (lootPtr->itemInfo.size() - 1 >= i)
						{
							if (SDL_GetTicks() - selectTouchTime > 800)
							{
								selectTouchTime = -1;
								CORO(executeSelectItemEx(pocketCursor, i + lootScroll));
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
	}
	sortFlag getSortType() { return sortType; }

	void executeTab()
	{
		if (pocketCursor == -1)
		{
			if (equipCursor == -1) //아이템을 선택 중이지 않을 경우
			{
				close(aniFlag::winSlipClose);
			}
			else
			{
				if (inputType != input::keyboard)
				{
					//select 아이템이 하나라도 있을 경우 전부 제거
					equipScroll = 0;
					equipCursor = -1;
					for (int i = 0; i < equipPtr->itemInfo.size(); i++) { equipPtr->itemInfo[i].lootSelect = 0; }
					barAct = actSet::null;
					tabType = tabFlag::closeWin;
				}
				else
				{
					close(aniFlag::winSlipClose);
				}
			}
		}
		else//가방 닫기
		{
			isTargetPocket = false;
			labelCursor = -1;
			pocketCursor = -1;
		}
	}
	//탭 키를 눌렀을 때의 연산
	void executeSort()
	{
		switch (sortType)
		{
			default:
				errorBox("undefined sorting in Equip.ixx");
				break;
			case sortFlag::null:
				lootPtr->sortWeightDescend();;
				break;
			case sortFlag::weightDescend:
				lootPtr->sortWeightDescend();
				break;
			case sortFlag::weightAscend:
				lootPtr->sortWeightAscend();
				break;
			case sortFlag::volumeDescend:
				lootPtr->sortVolumeDescend();
				break;
			case sortFlag::volumeAscend:
				lootPtr->sortVolumeAscend();
				break;
		}
		lootScroll = 0;
	}

	void executePickDrop()
	{
		if (lootPtr->itemInfo.size() > 0)
		{
			//선택된 아이템이 하나도 없으면 종료
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (lootPtr->itemInfo[i].lootSelect > 0) { break; }

				if (i == 0) { return; }
			}

			ItemPocket* drop = new ItemPocket(storageType::null);
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--)
			{
				if (lootPtr->itemInfo[i].lootSelect > 0)
				{
					lootPtr->transferItem(drop, i, lootPtr->itemInfo[i].lootSelect);
				}
			}
			for (int i = lootPtr->itemInfo.size() - 1; i >= 0; i--) { lootPtr->itemInfo[i].lootSelect = 0; }
			Player::ins()->drop(drop);
			Player::ins()->updateStatus();
			updateLog(col2Str(col::white) + sysStr[126]);//아이템을 버렸다.
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
		if (pocketCursor != 0)
		{
			pocketCursor--;

			lootCursor = 0;
			lootScroll = 0;
			labelCursor = -1;

			//포켓커서 변경에 따른 pocketPtr 변경
			int pocketStack = 0;
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
				{
					if (pocketCursor == pocketStack)
					{
						lootPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
						break;
					}
					else
					{
						pocketStack++;
					}
				}
			}
		}


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
		if (pocketCursor != numberOfBag - 1)
		{
			pocketCursor++;

			lootCursor = 0;
			lootScroll = 0;
			labelCursor = -1;

			//포켓커서 변경에 따른 pocketPtr 변경
			int pocketStack = 0;
			for (int i = 0; i < equipPtr->itemInfo.size(); i++)
			{
				if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
				{
					if (pocketCursor == pocketStack)
					{
						lootPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
						break;
					}
					else
					{
						pocketStack++;
					}
				}
			}
		}


	}
	//act
	void executeEquip()
	{
		updateLog(col2Str(col::white) + sysStr[125]);//아이템을 장착했다.
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

	void executeDroping()
	{
		ItemPocket* drop = new ItemPocket(storageType::null);
		if (isTargetPocket == false)
		{
			equipPtr->transferItem(drop, equipCursor, 1);
		}
		else
		{
			lootPtr->transferItem(drop, lootCursor, 1);
		}
		Player::ins()->drop(drop);
		Player::ins()->updateStatus();
		if (inputType == input::keyboard)
			doPopDownHUD = true;
		{
			barActCursor = -1;
		}
		updateLog(col2Str(col::white) + sysStr[126]);
	}
	void executeOpen()
	{
		lootPtr = (ItemPocket*)equipPtr->itemInfo[equipCursor].pocketPtr;
		pocketCursor = 0;
		isTargetPocket = true;
		for (int j = 0; j < equipPtr->itemInfo.size(); j++)
		{
			if (equipPtr->itemInfo[j].pocketMaxVolume > 0)
			{
				if (j == equipCursor)
				{
					break;
				}
				else
				{
					pocketCursor++;
				}
			}
		}

		if (inputType == input::keyboard)
		{
			lootCursor = 0;
			doPopDownHUD = true;
			barActCursor = -1;
		}
	}
	void updateBarAct()
	{
		if (equipCursor != -1)
		{
			if (equipPtr->itemInfo.size() > 0)
			{
				ItemData targetItem = equipPtr->itemInfo[equipCursor];
				barAct.clear();
				if (targetItem.pocketMaxVolume > 0) { barAct.push_back(act::open); }//가방 종류일 경우 open 추가
				if (targetItem.pocketOnlyItem.size() > 0) { barAct.push_back(act::reload); }//전용 아이템 있을 경우 reload 추가
				barAct.push_back(act::droping);//droping은 항상 추가
				barAct.push_back(act::throwing);//throwing도 항상 추가

				//업데이트할 아이템이 총일 경우
				if (targetItem.checkFlag(itemFlag::GUN))
				{
					//전용 아이템이 탄창일 경우(일반 소총)
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
					//전용 아이템이 탄일 경우(리볼버류)
					else if (itemDex[targetItem.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
					{
						ItemPocket* gunPtr = ((ItemPocket*)targetItem.pocketPtr);
						//탄환 분리
						if (gunPtr->itemInfo.size() > 0)
						{
							barAct.push_back(act::unloadBulletFromGun);
						}

						//탄환 장전
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
				//업데이트할 아이템이 탄창일 경우
				else if (targetItem.checkFlag(itemFlag::MAGAZINE))
				{
					barAct.push_back(act::reloadMagazine);

					//탄창 장전
					ItemPocket* magazinePtr = ((ItemPocket*)targetItem.pocketPtr);
					if (magazinePtr->itemInfo.size() > 0)
					{
						barAct.push_back(act::unloadBulletFromMagazine);
					}

					//총알 장전
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
				//업데이트할 아이템이 탄환일 경우
				else if (targetItem.checkFlag(itemFlag::AMMO))
				{
					barAct.push_back(act::reloadBulletToGun);
				}
			}
		}
		else if (lootCursor != -1)
		{
			if (lootPtr->itemInfo.size() > 0)
			{
				ItemData targetItem = lootPtr->itemInfo[lootCursor];
				barAct.clear();
				barAct.push_back(act::wield);
				if (targetItem.checkFlag(itemFlag::CANEQUIP) == true) { barAct.push_back(act::equip); }
				barAct.push_back(act::throwing);//throwing도 항상 추가
			}
		}
		else
		{
			errorBox("Equip instance : updateBarAct error occurs, Both lootCursor and equipCursor have the same value -1");
		}
	}

	Corouter executeSearch()
	{
		//이미 검색 중인지 체크
		for (int i = 0; i < lootPtr->itemInfo.size(); i++)
		{
			if (lootPtr->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))//이미 검색 중일 경우 검색 상태를 해제함
			{
				for (int j = 0; j < lootPtr->itemInfo.size(); j++)
				{
					lootPtr->itemInfo[j].eraseFlag(itemFlag::GRAYFILTER);
				}
				lootPtr->sortByUnicode();
				updateLog(col2Str(col::white) + sysStr[86]);//검색 상태를 해제했다.
				co_return;
			}

			if (i == lootPtr->itemInfo.size() - 1)//검색 중이 아닐 경우
			{
				std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
				new Msg(msgFlag::input, sysStr[27], sysStr[97], choiceVec);//검색, 검색할 키워드를 입력해주세요
				lootScroll = 0;
				co_await std::suspend_always();
				if (coAnswer == sysStr[38]) //검색 후 확인 버튼
				{
					int matchCount = lootPtr->searchTxt(exInputText);
					for (int i = 0; i < lootPtr->itemInfo.size(); i++) lootPtr->itemInfo[i].addFlag(itemFlag::GRAYFILTER);
					for (int i = 0; i < matchCount; i++) lootPtr->itemInfo[i].eraseFlag(itemFlag::GRAYFILTER);
				}
				else {}
			}
		}
	}

	Corouter executeSelectItemEx(int pocketCursor, int lootCursor)
	{
		//입력형 메시지 박스 열기
		std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
		new Msg(msgFlag::input, sysStr[40], sysStr[39], choiceVec);//아이템 선택, 얼마나?
		co_await std::suspend_always();

		//포켓커서에 따른 pocketPtr 변경
		ItemPocket* lootPtr = nullptr;
		int pocketStack = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)
			{
				if (pocketCursor == pocketStack)
				{
					lootPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
					break;
				}
				else
				{
					pocketStack++;
				}
			}
		}

		int inputSelectNumber = wtoi(exInputText.c_str()); // 셀렉트 박스에 넣어질 숫자(플레이어가 입력한 값)
		if (inputSelectNumber > lootPtr->itemInfo[lootCursor].number) //만약 실제 있는 숫자보다 많은 값을 입력했을 경우
		{
			inputSelectNumber = lootPtr->itemInfo[lootCursor].number; //Select의 값을 최댓값으로 맞춤
		}
		lootPtr->itemInfo[lootCursor].lootSelect = inputSelectNumber;
	}

	Corouter executeWield()
	{
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		if (lootPtr->itemInfo[lootCursor].checkFlag(itemFlag::TWOHANDED)) //양손장비일 경우
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
			equipPtr->itemInfo[returnIndex].equipState = equip::both; //양손
			equipPtr->sortEquip();
			updateLog(L"#FFFFFF아이템을 들었다.");
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
				int fixedLootCursor = lootCursor;//Msg가 켜지면 lootCursor가 -1로 초기화되기에 미리 지역변수로 저장

				//왼손, 오른손
				std::vector<std::wstring> choiceVec = { sysStr[49], sysStr[50] };
				//선택, 어느 손에 들까?
				new Msg(msgFlag::normal, sysStr[98], sysStr[99], choiceVec);
				co_await std::suspend_always();

				int handDir = 0;
				switch (wtoi(coAnswer.c_str()))
				{
					case 0:
						handDir = equip::left;
						break;
					case 1:
						handDir = equip::right;
						break;
				}

				//왼손 아이템 떨구기
				ItemPocket* drop = new ItemPocket(storageType::null);
				for (int i = equipPtr->itemInfo.size() - 1; i >= 0; i--)
				{
					if (equipPtr->itemInfo[i].equipState == handDir)
					{
						equipPtr->transferItem(drop, i, 1);
						break;
					}
				}
				//양손 아이템 떨구기
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
				int fixedLootCursor = lootCursor;//Msg가 켜지면 lootCursor가 -1로 초기화되기에 미리 지역변수로 저장

				//왼손, 오른손
				std::vector<std::wstring> choiceVec = { sysStr[49], sysStr[50] };
				//선택, 어느 손에 들까?
				new Msg(msgFlag::normal, sysStr[98], sysStr[99], choiceVec);
				co_await std::suspend_always();

				int handDir = 0;
				switch (wtoi(coAnswer.c_str()))
				{
					case 0:
						handDir = equip::left;
						break;
					case 1:
						handDir = equip::right;
						break;
				}
				int returnIndex = lootPtr->transferItem(equipPtr, fixedLootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = handDir;
				equipPtr->sortEquip();
			}
			else if (hasLeft == false && hasRight == true)//왼손에 들기
			{
				int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = equip::left;
				equipPtr->sortEquip();
				updateLog(L"#FFFFFF아이템을 들었다.");
			}
			else//오른손에 들기
			{
				int returnIndex = lootPtr->transferItem(equipPtr, lootCursor, 1);
				equipPtr->itemInfo[returnIndex].equipState = equip::right;
				equipPtr->sortEquip();
				updateLog(L"#FFFFFF아이템을 들었다.");
			}
		}
		Player::ins()->updateStatus();
		if (inputType == input::keyboard)
		{
			doPopDownHUD = true;
			barActCursor = -1;
		}
	}

	Corouter executeReload()//장전 : 타겟아이템(탄창이나 총)에 넣을 수 있는 탄환을 넣는다.
	{
		int targetEquipCursor = equipCursor;
		std::vector<std::wstring> bulletList;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (i == equipCursor) { continue; }

			//1층 : 현재 장비 중인 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
			std::vector<unsigned short> onlyItem = equipPtr->itemInfo[equipCursor].pocketOnlyItem;
			//넣을 수 있는 아이템인 경우
			if (std::find(onlyItem.begin(), onlyItem.end(), equipPtr->itemInfo[i].itemCode) != onlyItem.end())
			{
				bulletList.push_back(equipPtr->itemInfo[i].name);
				break;
			}

			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)//가방은 1단계까지만 뒤져봄
			{
				ItemPocket* pocketPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
				for (int pocketItr = 0; pocketItr < pocketPtr->itemInfo.size(); pocketItr++)
				{
					//2층 : 현재 이 포켓의 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
					std::vector<unsigned short> onlyItem = equipPtr->itemInfo[equipCursor].pocketOnlyItem;
					//넣을 수 있는 아이템인 경우
					if (std::find(onlyItem.begin(), onlyItem.end(), pocketPtr->itemInfo[pocketItr].itemCode) != onlyItem.end())
					{
						bulletList.push_back(pocketPtr->itemInfo[pocketItr].name);
						break;
					}
				}
			}
		}

		if (bulletList.size() == 0) //넣을만한 포켓을 찾지 못했을 경우
		{
			//이 아이템을 넣을만한 포켓이 없다.
			updateLog(col2Str(col::white) + sysStr[96]);
			co_return;
		}

		////////////////////////////////////////////////////////////////////

		new Lst(sysStr[95], sysStr[94], bulletList);//넣기, 넣을 포켓을 선택해주세요.
		co_await std::suspend_always();

		////////////////////////////////////////////////////////////////////

		int counter = 0;
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (i == targetEquipCursor) { continue; }

			//1층 : 현재 장비 중인 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
			std::vector<unsigned short> onlyItem = equipPtr->itemInfo[targetEquipCursor].pocketOnlyItem;
			//넣을 수 있는 아이템인 경우
			if (std::find(onlyItem.begin(), onlyItem.end(), equipPtr->itemInfo[i].itemCode) != onlyItem.end())
			{
				if (counter == wtoi(coAnswer.c_str()))//일치하는 아이템일 경우(1층)
				{
					if (equipPtr->itemInfo[targetEquipCursor].checkFlag(itemFlag::GUN))
					{
						if (equipPtr->itemInfo[i].checkFlag(itemFlag::AMMO))
						{
							//(%bullet) (%number)개를(을) (%gun)에 장전했다!

							std::wstring str1 = replaceStr(sysStr[103], L"(%bullet)", equipPtr->itemInfo[i].name);
							std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(equipPtr->itemInfo[i].number));
							std::wstring str3 = replaceStr(str2, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
							updateLog(col2Str(col::white) + str3);
						}
						else
						{
							//(%magazine)를(을) (%gun)에 장전했다!
							std::wstring str1 = replaceStr(sysStr[101], L"(%magazine)", equipPtr->itemInfo[i].name);
							std::wstring str2 = replaceStr(str1, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
							updateLog(col2Str(col::white) + str2);
						}
					}
					else
					{
						//(%bullet) (%number)개를(을) (%magazine)에 장전했다!
						std::wstring str1 = replaceStr(sysStr[102], L"(%bullet)", equipPtr->itemInfo[i].name);
						std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(equipPtr->itemInfo[i].number));
						std::wstring str3 = replaceStr(str2, L"(%magazine)", equipPtr->itemInfo[targetEquipCursor].name);
						updateLog(col2Str(col::white) + str3);
					}

					equipPtr->transferItem
					(
						(ItemPocket*)equipPtr->itemInfo[targetEquipCursor].pocketPtr,
						i,
						equipPtr->itemInfo[i].number
					);
					co_return;
				}
				counter++;
				break;
			}

			if (equipPtr->itemInfo[i].pocketMaxVolume > 0)//가방은 1단계까지만 뒤져봄
			{
				ItemPocket* pocketPtr = (ItemPocket*)equipPtr->itemInfo[i].pocketPtr;
				for (int pocketItr = 0; pocketItr < pocketPtr->itemInfo.size(); pocketItr++)
				{
					//2층 : 현재 이 포켓의 아이템 중에서 넣을 수 있는 탄창이 있는지 확인
					std::vector<unsigned short> onlyItem = equipPtr->itemInfo[targetEquipCursor].pocketOnlyItem;
					//넣을 수 있는 아이템인 경우
					if (std::find(onlyItem.begin(), onlyItem.end(), pocketPtr->itemInfo[pocketItr].itemCode) != onlyItem.end())
					{
						if (counter == wtoi(coAnswer.c_str()))//일치하는 아이템일 경우(2층)
						{
							if (equipPtr->itemInfo[targetEquipCursor].checkFlag(itemFlag::GUN))
							{
								if (pocketPtr->itemInfo[pocketItr].checkFlag(itemFlag::AMMO))
								{
									//(%bullet) (%number)개를(을) (%gun)에 장전했다!

									std::wstring str1 = replaceStr(sysStr[103], L"(%bullet)", pocketPtr->itemInfo[pocketItr].name);
									std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(pocketPtr->itemInfo[pocketItr].number));
									std::wstring str3 = replaceStr(str2, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
									updateLog(col2Str(col::white) + str3);
								}
								else
								{
									//(%magazine)를(을) (%gun)에 장전했다!
									std::wstring str1 = replaceStr(sysStr[101], L"(%magazine)", pocketPtr->itemInfo[pocketItr].name);
									std::wstring str2 = replaceStr(str1, L"(%gun)", equipPtr->itemInfo[targetEquipCursor].name);
									updateLog(col2Str(col::white) + str2);
								}
							}
							else
							{
								//(%bullet) (%number)개를(을) (%magazine)에 장전했다!
								std::wstring str1 = replaceStr(sysStr[102], L"(%bullet)", pocketPtr->itemInfo[pocketItr].name);
								std::wstring str2 = replaceStr(str1, L"(%number)", std::to_wstring(pocketPtr->itemInfo[pocketItr].number));
								std::wstring str3 = replaceStr(str2, L"(%magazine)", equipPtr->itemInfo[targetEquipCursor].name);
								updateLog(col2Str(col::white) + str3);
							}


							pocketPtr->transferItem
							(
								(ItemPocket*)equipPtr->itemInfo[targetEquipCursor].pocketPtr,
								pocketItr,
								pocketPtr->itemInfo[pocketItr].number
							);
							co_return;
						}
						counter++;
						break;
					}
				}
			}
		}
	}

	Corouter executeThrowing(ItemPocket* inputPocket, int inputIndex)//던지기
	{
		new CoordSelect(sysStr[131]);
		deactDraw();

		co_await std::suspend_always();

		std::wstring targetStr = coAnswer;
		int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
		targetStr.erase(0, targetStr.find(L",") + 1);
		int targetZ = wtoi(targetStr.c_str());

		Player::ins()->setDirection(getIntDegree(Player::ins()->getGridX(), Player::ins()->getGridY(), targetX, targetY));

		prt(L"executeThrowing에서 사용한 좌표의 값은 (%d,%d,%d)이다.\n", targetX, targetY, targetZ);

		if (targetX == Player::ins()->getGridX() && targetY == Player::ins()->getGridY() && targetZ == Player::ins()->getGridZ())
		{
			ItemPocket* drop = new ItemPocket(storageType::null);
			inputPocket->transferItem(drop, inputIndex, 1);
			Player::ins()->drop(drop);
			Player::ins()->updateStatus();
			updateLog(L"#FFFFFF아이템을 버렸다.");
		}
		else
		{
			ItemPocket* throwing = new ItemPocket(storageType::null);
			//이큅일 떄는 그렇다쳐도 가방 안에 있는 아이템을 던질 떄 원하는대로 작동하지않아 오류가 생긴다
			inputPocket->transferItem(throwing, inputIndex, 1);
			Player::ins()->throwing(throwing, targetX, targetY);
			Player::ins()->updateStatus();
			updateLog(L"#FFFFFF아이템을 던졌다.");
		}
		close(aniFlag::null);
	}

};
