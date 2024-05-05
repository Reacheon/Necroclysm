export module drawItemList;

import drawWindowArrow;
import ItemPocket;
import globalVar;
import drawItemSlot;

//@brief 해당 위치에 ItemPocket에 들어있는 아이템을 표시합니다. drawItemSlot을 연속으로 쓰는 함수입니다.
//@param storage 표시할 아이템의 주소
//@param x 표시할 사각형의 좌측상단 x값
//@param y 표시할 사각형의 좌측상단 y값
//@param itemNumber 아이템을 표시할 갯수(6이면 6개의 아이템까지만 표시)
//@param cursor 가리키는 커서, 해당 커서는 빛납니다.
//@param scroll 현재 스크롤
//@return 없음
export void drawItemList(ItemPocket* storage, int x, int y, int itemNumber, int cursor, int scroll, bool whiteCursor)
{
	bool select = false;
	int status = 0;
	for (int i = 0; i < itemNumber; i++)
	{
		if (i + scroll < storage->itemInfo.size()) //storage에 있는 아이템만 표시함
		{
			drawItemRectExtend(cursor == i + scroll, x, y + 42 * i, storage->itemInfo[i + scroll], 0, true, whiteCursor);
		}
	}

	if (scroll + itemNumber < storage->itemInfo.size()) { drawWindowArrow(x + 156, y + 42 * itemNumber - 16, 3); }
	if (scroll > 0) { drawWindowArrow(x + 156, y + 10, 1); }
}