module;
#include <SDL3/SDL.h>

export module ItemListPanel;

import std;
import util;
import constVar;
import globalVar;
import ItemPocket;
import ItemData;
import drawItemList;
import drawText;
import checkCursor;


///@brief Loot, Equip, Inventory 등 아이템 리스트 GUI에서 공통으로 사용되는 컴포넌트.
/// GUI를 상속하지 않는 단순 컴포지션 클래스로, 각 GUI가 멤버로 소유한다.
export class ItemListPanel
{
public:
	ItemPocket* pocket = nullptr;
	int scroll = 0;
	int cursor = -1;
	int itemMax;
	sortFlag sortType = sortFlag::null;

	SDL_Rect itemRect[30];
	SDL_Rect itemSelectRect[30];
	SDL_Rect label;
	SDL_Rect labelSelect;
	SDL_Rect labelName;
	SDL_Rect labelQuantity;

	///@brief 요리/상점 등에서 특정 아이템을 회색으로 표시하기 위한 필터 콜백.
	/// true를 반환하면 해당 아이템을 회색으로 표시한다.
	std::function<bool(const ItemData&)> grayFilter = nullptr;

	ItemListPanel(int maxItems) : itemMax(maxItems) {}

	// ======================================================================
	// Rect 초기화
	// ======================================================================

	///@brief 아이템 리스트 영역의 좌표를 기준으로 모든 rect를 초기화한다.
	///@param baseX 윈도우 base의 x좌표
	///@param areaY 아이템 리스트 영역의 y좌표 (첫 아이템이 그려지는 y)
	void initRects(int baseX, int areaY)
	{
		for (int i = 0; i < 30; i++)
		{
			itemRect[i] = { baseX + 62, areaY + 37 * i, 325, 32 };
			itemSelectRect[i] = { baseX + 12, areaY + 37 * i, 43, 32 };
		}
		label = { baseX + 12, areaY - 36, 376, 31 };
		labelSelect = { label.x, label.y, 75, 31 };
		labelName = { label.x + labelSelect.w, label.y, 219, 31 };
		labelQuantity = { label.x + labelName.w + labelSelect.w, label.y, 85, 31 };
	}

	///@brief Equip처럼 itemRect의 x 오프셋이 다른 경우 사용
	void initRects(int baseX, int areaY, int itemRectOffsetX)
	{
		for (int i = 0; i < 30; i++)
		{
			itemRect[i] = { baseX + itemRectOffsetX, areaY + 37 * i, 325, 32 };
			itemSelectRect[i] = { baseX, areaY + 37 * i, 43, 32 };
		}
		label = { baseX, areaY - 36, 376, 31 };
		labelSelect = { label.x, label.y, 75, 31 };
		labelName = { label.x + labelSelect.w, label.y, 219, 31 };
		labelQuantity = { label.x + labelName.w + labelSelect.w, label.y, 85, 31 };
	}

	// ======================================================================
	// 그리기 (Draw)
	// ======================================================================

	///@brief 상단 라벨바(선택/이름/물리량)를 그린다.
	///@param isActive 이 GUI가 최상단(포커스)인지 여부
	///@param showSort true이면 물리량 칸에 정렬 상태를 표시
	void drawLabelBar(bool isActive, bool showSort = false)
	{
		drawStadium(label.x, label.y, label.w, label.h, { 0,0,0 }, 183, 5);
		if (isActive)
		{
			if (checkCursor(&labelSelect))
			{
				SDL_Color btnColor = lowCol::blue;
				if (click) btnColor = lowCol::deepBlue;
				drawStadium(labelSelect.x, labelSelect.y, labelSelect.w, labelSelect.h, btnColor, 183, 5);
			}
			else if (checkCursor(&labelName))
			{
				SDL_Color btnColor = lowCol::blue;
				if (click) btnColor = lowCol::deepBlue;
				drawStadium(labelName.x, labelName.y, labelName.w, labelName.h, btnColor, 183, 5);
			}
			else if (checkCursor(&labelQuantity))
			{
				SDL_Color btnColor = lowCol::blue;
				if (click) btnColor = lowCol::deepBlue;
				drawStadium(labelQuantity.x, labelQuantity.y, labelQuantity.w, labelQuantity.h, btnColor, 183, 5);
			}
		}

		setFontSize(14);
		drawTextCenter(sysStr[10], label.x + 30, label.y + 14);     //선택
		drawTextCenter(sysStr[11], label.x + 183, label.y + 14);    //이름

		if (showSort)
		{
			switch (sortType)
			{
			default:
				drawTextCenter(sysStr[15], label.x + 337, label.y + 14);
				break;
			case sortFlag::weightDescend:
				drawTextCenter(sysStr[28], label.x + 337, label.y + 14);
				break;
			case sortFlag::weightAscend:
				drawTextCenter(sysStr[29], label.x + 337, label.y + 14);
				break;
			case sortFlag::volumeDescend:
				drawTextCenter(sysStr[30], label.x + 337, label.y + 14);
				break;
			case sortFlag::volumeAscend:
				drawTextCenter(sysStr[31], label.x + 337, label.y + 14);
				break;
			}
		}
		else
		{
			drawTextCenter(sysStr[15], label.x + 337, label.y + 14); //물리량
		}
	}

	///@brief 아이템 리스트를 그린다. 호출 전에 itemListColorLock을 설정해야 한다.
	void drawList(int x, int y, bool whiteCursor)
	{
		drawItemList(pocket, x, y, itemMax, cursor, scroll, whiteCursor);
	}

	///@brief 스크롤바를 그린다.
	void drawScrollbar(int x, int topY, int height)
	{
		if (pocket->itemInfo.size() <= itemMax) return;

		SDL_Rect scrollBox = { x, topY, 2, height };
		drawFillRect(scrollBox, { 120, 120, 120 });

		SDL_Rect inBox = scrollBox;
		inBox.h = (int)(scrollBox.h * myMin(1.0, (double)itemMax / pocket->itemInfo.size()));
		if (inBox.h < 5) inBox.h = 5;

		if (!pocket->itemInfo.empty())
			inBox.y = scrollBox.y + (int)(scrollBox.h * ((float)scroll / (float)pocket->itemInfo.size()));
		else
			inBox.y = scrollBox.y;

		if (inBox.y < scrollBox.y) inBox.y = scrollBox.y;
		if (inBox.y + inBox.h > scrollBox.y + scrollBox.h)
			inBox.y = scrollBox.y + scrollBox.h - inBox.h;

		drawFillRect(inBox, col::white);
	}

	///@brief 빈 리스트 메시지를 표시한다.
	void drawEmptyMsg(int centerX, int centerY)
	{
		if (pocket->itemInfo.size() == 0)
		{
			setFont(fontType::mainFont);
			setFontSize(16);
			drawTextCenter(sysStr[101], centerX, centerY, col::lightGray); //가방 안에 아이템이 없다
		}
	}

	///@brief 커서/전체 수 표시 (예: "3/12")
	void drawCursorInfo(int x, int y)
	{
		setFontSize(12);
		drawText(std::to_wstring(cursor + 1) + L"/" + std::to_wstring(pocket->itemInfo.size()), x, y);
	}

	// ======================================================================
	// 선택 (Selection)
	// ======================================================================

	///@brief 전체 아이템의 선택 상태를 토글한다.
	void selectAll()
	{
		bool allSelected = true;
		for (int i = 0; i < pocket->itemInfo.size(); i++)
		{
			if (pocket->itemInfo[i].lootSelect != pocket->itemInfo[i].number)
			{
				allSelected = false;
				break;
			}
		}

		for (int i = 0; i < pocket->itemInfo.size(); i++)
		{
			pocket->itemInfo[i].lootSelect = allSelected ? 0 : pocket->itemInfo[i].number;
		}
	}

	///@brief 특정 인덱스의 아이템을 전량 선택한다.
	void selectItem(int index)
	{
		pocket->itemInfo[index].lootSelect = pocket->itemInfo[index].number;
	}

	///@brief 모든 아이템의 선택을 해제한다.
	void clearAllSelections()
	{
		for (int i = 0; i < pocket->itemInfo.size(); i++)
		{
			pocket->itemInfo[i].lootSelect = 0;
		}
	}

	///@brief 선택된 아이템이 하나라도 있는지 확인한다.
	bool hasAnySelection()
	{
		for (int i = 0; i < pocket->itemInfo.size(); i++)
		{
			if (pocket->itemInfo[i].lootSelect > 0) return true;
		}
		return false;
	}

	// ======================================================================
	// 정렬 (Sort)
	// ======================================================================

	///@brief 정렬 모드를 순환한다: 무게↓ → 무게↑ → 부피↓ → 부피↑ → 유니코드 → 무게↓ ...
	void sort()
	{
		switch (sortType)
		{
		default:
			errorBox(L"ItemListPanel : 잘못된 sortFlag가 사용되었다.");
			break;
		case sortFlag::null:
			pocket->sortWeightDescend();
			sortType = sortFlag::weightDescend;
			break;
		case sortFlag::weightDescend:
			pocket->sortWeightAscend();
			sortType = sortFlag::weightAscend;
			break;
		case sortFlag::weightAscend:
			pocket->sortVolumeDescend();
			sortType = sortFlag::volumeDescend;
			break;
		case sortFlag::volumeDescend:
			pocket->sortVolumeAscend();
			sortType = sortFlag::volumeAscend;
			break;
		case sortFlag::volumeAscend:
			pocket->sortByUnicode();
			sortType = sortFlag::null;
			break;
		}
		scroll = 0;
	}

	// ======================================================================
	// 입력 처리 (Input)
	// ======================================================================

	///@brief 아이템 영역 클릭 시 커서를 토글한다.
	///@return 0: 처리 안됨, 1: 새 커서 선택됨 (updateBarAct 필요), -1: 커서 해제됨
	int handleItemClick()
	{
		for (int i = 0; i < itemMax; i++)
		{
			if ((int)pocket->itemInfo.size() - 1 >= i + scroll)
			{
				if (checkCursor(&itemRect[i]))
				{
					if (cursor != scroll + i) //새로운 커서 생성
					{
						cursor = scroll + i;
						return 1;
					}
					else //커서 해제
					{
						cursor = -1;
						barAct = actSet::null();
						return -1;
					}
				}
			}
		}
		return 0;
	}

	///@brief 셀렉트 박스 클릭 시 선택을 토글한다.
	///@return true이면 처리됨
	bool handleSelectClick()
	{
		for (int i = 0; i < itemMax; i++)
		{
			if (checkCursor(&itemSelectRect[i]))
			{
				if ((int)pocket->itemInfo.size() - 1 >= i + scroll)
				{
					if (pocket->itemInfo[i + scroll].lootSelect == 0)
					{
						selectItem(i + scroll);
					}
					else
					{
						pocket->itemInfo[i + scroll].lootSelect = 0;
					}
					return true;
				}
			}
		}
		return false;
	}

	///@brief 우클릭 시 해당 인덱스를 반환한다. 없으면 -1.
	int getSelectRightClickIndex()
	{
		for (int i = 0; i < itemMax; i++)
		{
			if (checkCursor(&itemSelectRect[i]))
			{
				if ((int)pocket->itemInfo.size() - 1 >= i + scroll)
				{
					if (pocket->itemInfo[i + scroll].lootSelect == 0)
						return i + scroll;
					else
					{
						pocket->itemInfo[i + scroll].lootSelect = 0;
						return -1;
					}
				}
			}
		}
		return -1;
	}

	///@brief 마우스 휠 스크롤 처리
	void handleWheel(SDL_Rect& baseRect)
	{
		if (checkCursor(&baseRect))
		{
			if (event.wheel.y > 0 && scroll > 0) scroll -= 1;
			else if (event.wheel.y < 0 && scroll + itemMax < (int)pocket->itemInfo.size()) scroll += 1;
		}
	}

	// ======================================================================
	// 스텝 유지보수 (Step)
	// ======================================================================

	///@brief 매 프레임 호출. 스크롤과 커서의 범위를 보정한다.
	void adjustScrollAndCursor()
	{
		if (cursor > (int)(pocket->itemInfo.size() - 1))
		{
			cursor = pocket->itemInfo.size() - 1;
		}

		if (option::inputMethod == input::mouse || option::inputMethod == input::touch)
		{
			if (scroll + itemMax >= (int)pocket->itemInfo.size())
			{
				scroll = myMax(0, (int)pocket->itemInfo.size() - itemMax);
			}
			else if (scroll < 0)
			{
				scroll = 0;
			}
		}
	}

	///@brief 아이템 수에 따른 윈도우 동적 높이를 계산한다.
	int calcWindowHeight()
	{
		return 197 + 38 * myMax(0, myMin(itemMax - 1, (int)pocket->itemInfo.size() - 1));
	}

	///@brief 액션 실행 후 스크롤을 보정한다.
	void adjustScrollAfterAction()
	{
		if ((int)pocket->itemInfo.size() - 1 <= scroll + itemMax)
		{
			scroll = (int)pocket->itemInfo.size() - itemMax;
			if (scroll < 0) scroll = 0;
		}
	}
};
