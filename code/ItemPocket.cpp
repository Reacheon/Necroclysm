#include <SDL.h>
#include <SDL_image.h>

import ItemPocket;
import std;
import util;
import World;
import globalVar;
import constVar;
import Coord;
import log;
import ItemData;

ItemPocket::ItemPocket(storageType inputType) { type = inputType; }

ItemPocket::~ItemPocket() { prt(L"ItemPocket : 소멸자가 호출되었습니다..\n"); }

storageType ItemPocket::getType() { return type; }

void ItemPocket::eraseItemInfo(int index)
{
	auto it = itemInfo.begin();
	it += index;
	itemInfo.erase(it);
}

//@brief 입력한 숫자만큼 아이템의 수를 제거합니다. 만약 0이 되면 목록에서 제거합니다. 아이템의 수보다 큰 값을 입력 시 오류가 발생, 동시에 Select가 더 클 경우 그 값을 맞춤
void ItemPocket::subtractItemIndex(int index, int number)
{
	errorBox(itemInfo[index].number < number, "The number of items to remove is greater than the number of items.");
	itemInfo[index].number -= number;
	//셀렉트 값 조정
	if (itemInfo[index].lootSelect > itemInfo[index].number) { itemInfo[index].lootSelect = itemInfo[index].number; }
	if (itemInfo[index].number == 0) { eraseItemInfo(index); }
}

//@brief 입력한 아이템 코드의 아이템을 number개 포켓에서 제거합니다. subtractItemIndex를 활용하는 재귀함수.
void ItemPocket::subtractItemCode(int code, int number)
{
	int count = 0;
	for (int i = 0; i < itemInfo.size(); i++)
	{
		if (itemInfo[i].itemCode = code)
		{
			int currentCount = count;
			count += myMin(number - currentCount, itemInfo[i].number);
			subtractItemIndex(i, myMin(number - currentCount, itemInfo[i].number));
		}

		if (count == number) return;
		else if (count > number) errorBox("Error occurs at method subtractItemCode(ItemPocket.ixx), the number of Item has been subtracted over its own number.");
	}
}

//@brief 입력한 storagePtr로 아이템을 number개 옮김(송신하는 Ptr은 이 메소드를 실행하는 인스턴스), 수신++, 송신--,  실제 아이템의 수보다 많이 넣으면 오류 발생
//@param storagePtr 아이템을 받을 storage 포인터
//@param index 보낼 아이템의 인덱스(송신 객체의 index번째 아이템임)
//@return 수신받은 ItemPocket의 index
int ItemPocket::transferItem(ItemPocket* storagePtr, int index, int number)
{
	//전송할 아이템의 숫자가 명령한 숫자보다 부족할 경우 오류 발생
	errorBox(itemInfo[index].number < number, "item number that have to transfer is not enough in transferItem function(ItemPocket.ixx)");

	//일치하는 아이템이 있을 경우 숫자만 더하고 종료함
	for (int i = 0; i < storagePtr->itemInfo.size(); i++)
	{
		//완전히 일치하는 아이템이 있을 경우
		if (itemOverlay(itemInfo[index], storagePtr->itemInfo[i]) == true)
		{
			storagePtr->itemInfo[i].number += number;
			itemInfo[index].lootSelect = 0;
			subtractItemIndex(index, number);
			return i;
		}
	}

	//일치하는 아이템을 찾지 못했을 경우 그 아이템을 위해 새로운 행을 추가함
	ItemData nextItem = itemInfo[index];
	nextItem.number = number;
	nextItem.lootSelect = 0;
	nextItem.eraseFlag(itemFlag::GRAYFILTER);
	if (storagePtr->getType() == storageType::equip) { nextItem.equipState = equipHandFlag::normal; }
	else { nextItem.equipState = equipHandFlag::none; }
	storagePtr->itemInfo.push_back(nextItem);
	subtractItemIndex(index, number);//원래 자리에 있는 아이템을 number개 제거함
	return storagePtr->itemInfo.size() - 1;
}

int ItemPocket::addItemFromDex(int index, int number)
{
	ItemData targetItem = itemDex[index];
	if (targetItem.pocketMaxVolume > 0 || targetItem.pocketMaxNumber > 0) // 해당 아이템이 포켓을 가질 경우
	{
		targetItem.number = 1;
		for (int i = 0; i < number; i++)
		{
			targetItem.pocketPtr = new ItemPocket(storageType::pocket);
			itemInfo.push_back(targetItem);
		}
		return -1;
	}
	else
	{
		targetItem.number = number;

		for (int i = 0; i < itemInfo.size(); i++)
		{
			if (itemOverlay(targetItem, itemInfo[i])) //일치하는 아이템을 찾 았을 경우
			{
				itemInfo[i].number += targetItem.number;
				return i;
			}
		}
		//일치하는 아이템을 찾지 못했을 경우 그 아이템을 위해 새로운 행을 추가함
		itemInfo.push_back(targetItem);
		return itemInfo.size() - 1;
	}
}
int ItemPocket::addItemFromDex(int index) { return addItemFromDex(index, 1); }

void ItemPocket::addRecipe(int inputItemCode)
{
	errorBox(getType() != storageType::recipe, "The addRecipe function was executed while the storageType was not set to 'recipe'(ItemPocket.ixx).");

	if (itemDex[inputItemCode].checkFlag(itemFlag::NOT_RECIPE)) return; //NOT_RECIPE 플래그가 있을 경우 레시피목록에 추가하지 않음

	if (itemInfo.size() == 0) { addItemFromDex(inputItemCode, 1); }
	else
	{
		for (int i = 0; i < itemInfo.size(); i++)
		{
			//이미 레시피에 있는 아이템이면 추가하지 않음
			if (itemInfo[i].itemCode == inputItemCode) break;

			if (i == itemInfo.size() - 1) addItemFromDex(inputItemCode, 1);
		}
	}
}

void ItemPocket::swap(int index1, int index2)
{
	if (index1 == index2) { return; }
	errorBox(index1 > itemInfo.size() - 1 || index1 < 0, "index1 is an invalid value in swap.");
	errorBox(index2 > itemInfo.size() - 1 || index2 < 0, "index2 is an invalid value in swap.");
	ItemData tempItem = itemInfo[index2];
	itemInfo[index2] = itemInfo[index1];//인덱스1의 내용을 인덱스2에 덮어쓰기
	itemInfo[index1] = tempItem;//tempArr의 내용을 인덱스1에 덮어쓰기
}

void ItemPocket::sortByUnicode(int startIndex, int endIndex)
{
	std::sort(itemInfo.begin() + startIndex, itemInfo.begin() + endIndex + 1,
		[](ItemData a, ItemData b)
		{
			std::wstring str1 = a.name;
			std::wstring str2 = b.name;
			for (int i = 0; i < myMin(str1.size(), str2.size()); i++)
			{
				if (str1[i] != str2[i]) { return str1[i] < str2[i]; }
			}
			return str1.size() < str2.size();
		}
	);
}
void ItemPocket::sortByUnicode() { sortByUnicode(0, itemInfo.size() - 1); }

void ItemPocket::sortEquip()
{
	int stackCursor = 0;

	//양손
	for (int i = itemInfo.size() - 1; i >= 0; i--)
	{
		if (itemInfo[i].equipState == equipHandFlag::both)
		{
			swap(stackCursor, i);
			stackCursor++;
			break;
		}
	}

	//왼손
	for (int i = itemInfo.size() - 1; i >= 0; i--)
	{
		if (itemInfo[i].equipState == equipHandFlag::left)
		{
			swap(stackCursor, i);
			stackCursor++;
			break;
		}
	}

	//오른손
	for (int i = itemInfo.size() - 1; i >= 0; i--)
	{
		if (itemInfo[i].equipState == equipHandFlag::right)
		{
			swap(stackCursor, i);
			stackCursor++;
			break;
		}
	}
}

void ItemPocket::sortWeightDescend(int startIndex, int endIndex)
{
	std::sort(itemInfo.begin() + startIndex, itemInfo.begin() + endIndex + 1,
		[](ItemData a, ItemData b)
		{
			return (a.weight > b.weight);
		}
	);
}
void ItemPocket::sortWeightDescend() { sortWeightDescend(0, itemInfo.size() - 1); }

void ItemPocket::sortWeightAscend(int startIndex, int endIndex)
{
	std::sort(itemInfo.begin() + startIndex, itemInfo.begin() + endIndex + 1,
		[](ItemData a, ItemData b)
		{
			return (a.weight < b.weight);
		}
	);
}
void ItemPocket::sortWeightAscend() { sortWeightAscend(0, itemInfo.size() - 1); }

void ItemPocket::sortVolumeDescend(int startIndex, int endIndex)
{
	std::sort(itemInfo.begin() + startIndex, itemInfo.begin() + endIndex + 1,
		[](ItemData a, ItemData b)
		{
			return (a.volume > b.volume);
		}
	);
}
void ItemPocket::sortVolumeDescend() { sortVolumeDescend(0, itemInfo.size() - 1); }

void ItemPocket::sortVolumeAscend(int startIndex, int endIndex)
{
	std::sort(itemInfo.begin() + startIndex, itemInfo.begin() + endIndex + 1,
		[](ItemData a, ItemData b)
		{
			return (a.volume < b.volume);
		}
	);
}
void ItemPocket::sortVolumeAscend() { sortVolumeAscend(0, itemInfo.size() - 1); }

////////////////////////////////////////////////////////////////////////////////

int ItemPocket::searchTxt(std::wstring inputStr, int startIndex, int endIndex)
{
	std::wstring keyword = inputStr.substr(0, inputStr.find(L"-") - 1);
	std::sort(itemInfo.begin() + startIndex, itemInfo.begin() + endIndex + 1,
		[keyword](ItemData a, ItemData b)
		{
			bool sameKeyword1 = (a.name.find(keyword.c_str()) != std::wstring::npos);
			bool sameKeyword2 = (b.name.find(keyword.c_str()) != std::wstring::npos);
			return sameKeyword1 > sameKeyword2;
		}
	);

	int matchCount = 0;
	for (int i = startIndex; i <= endIndex; i++)
	{
		if (itemInfo[i].name.find(keyword.c_str()) != std::wstring::npos) matchCount++;
	}
	return matchCount;
}
int ItemPocket::searchTxt(std::wstring inputStr) { return searchTxt(inputStr, 0, itemInfo.size() - 1); }

int ItemPocket::searchFlag(itemFlag inputFlag, int startIndex, int endIndex)
{
	std::sort(itemInfo.begin() + startIndex, itemInfo.begin() + endIndex + 1,
		[inputFlag](ItemData a, ItemData b)
		{
			bool sameKeyword1 = a.checkFlag(inputFlag);
			bool sameKeyword2 = b.checkFlag(inputFlag);
			return (sameKeyword1 > sameKeyword2);
		}
	);

	int matchCount = 0;
	for (int i = startIndex; i <= endIndex; i++)
	{
		if (itemInfo[i].checkFlag(inputFlag)) matchCount++;
	}
	return matchCount;
}
int ItemPocket::searchFlag(itemFlag inputFlag) { return searchFlag(inputFlag, 0, itemInfo.size() - 1); }

int ItemPocket::searchCategory(itemCategory input, int startIndex, int endIndex)
{
	std::sort(itemInfo.begin() + startIndex, itemInfo.begin() + endIndex + 1,
		[input](ItemData a, ItemData b)
		{
			bool sameKeyword1 = a.category == input;
			bool sameKeyword2 = b.category == input;
			return (sameKeyword1 > sameKeyword2);
		}
	);


	int matchCount = 0;
	for (int i = startIndex; i <= endIndex; i++) if (itemInfo[i].category == input) matchCount++;
	return matchCount;
}
int ItemPocket::searchCategory(itemCategory input) { return searchCategory(input, 0, itemInfo.size() - 1); }

int ItemPocket::searchSubcategory(itemSubcategory input, int startIndex, int endIndex)
{
	std::sort(itemInfo.begin() + startIndex, itemInfo.begin() + endIndex + 1,
		[input](ItemData a, ItemData b)
		{
			bool sameKeyword1 = a.subcategory == input;
			bool sameKeyword2 = b.subcategory == input;
			return (sameKeyword1 > sameKeyword2);
		}
	);

	int matchCount = 0;
	for (int i = startIndex; i <= endIndex; i++) if (itemInfo[i].subcategory == input) matchCount++;
	return matchCount;
}
int ItemPocket::searchSubcategory(itemSubcategory input) { return searchSubcategory(input, 0, itemInfo.size() - 1); }

//현재 이 포켓의 하위포켓을 모두 체크해 가지고있는 아이템의 숫자 반환
int ItemPocket::numberItem(int inputCode)
{
	int count = 0;
	for (int i = 0; i < itemInfo.size(); i++)
	{
		if (itemInfo[i].itemCode == inputCode) count += itemInfo[i].number;
		if (itemInfo[i].pocketPtr != nullptr) count += ((ItemPocket*)itemInfo[i].pocketPtr)->numberItem(inputCode);
	}
	return count;
}

//현재 이 포켓의 하위포켓을 모두 체크하여 입력한 도구기술이 존재하는지 반환
bool ItemPocket::checkToolQuality(int input)
{
	for (int i = 0; i < itemInfo.size(); i++)
	{
		for (int j = 0; j < itemInfo[i].toolQuality.size(); j++)
		{
			if (itemInfo[i].toolQuality[j] == input) return true;
			if (itemInfo[i].pocketPtr != nullptr)
			{
				bool internalCheck = ((ItemPocket*)itemInfo[i].pocketPtr)->checkToolQuality(input);
				if (internalCheck) return true;
			}
		}
	}
	return false;
}

//현재 이 총에 장전된 모든 총알을 벡터 형태로 반환
ItemPocket* getBulletPocket(ItemData inputGun)
{
	ItemPocket* pocket = nullptr;

	//직탄식 총
	if (itemDex[inputGun.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
	{
		pocket = ((ItemPocket*)inputGun.pocketPtr);
	}
	//탄창식 총
	else if (itemDex[inputGun.pocketOnlyItem[0]].checkFlag(itemFlag::MAGAZINE))
	{
		if (((ItemPocket*)inputGun.pocketPtr)->itemInfo.size() > 0)
		{
			pocket = (ItemPocket*)((ItemPocket*)inputGun.pocketPtr)->itemInfo[0].pocketPtr;
		}
		else
		{
			pocket = ((ItemPocket*)inputGun.pocketPtr);
		}
	}

	for (int i = 0; i < pocket->itemInfo.size(); i++)
	{
		//포켓을 만나면 중단하고 그걸 피벗으로 삼고 처음부터 다시
		if (pocket->itemInfo[i].pocketPtr != nullptr && ((ItemPocket*)pocket->itemInfo[i].pocketPtr)->itemInfo.size() > 0)
		{
			pocket = (ItemPocket*)pocket->itemInfo[i].pocketPtr;
			i = 0;
			continue;
		}
		else
		{
			if (i == pocket->itemInfo.size() - 1)
			{
				return pocket;
			}
		}
	}
	return pocket;
};

//상단 총알의 데이터를 삭제
void popTopBullet(ItemPocket* inputPocket)
{
	errorBox(inputPocket->itemInfo.size() == 0, "function popTopBullet has executed with no bullet in ItemPocket.ixx");
	if (inputPocket->itemInfo[0].number == 1) { inputPocket->itemInfo.erase(inputPocket->itemInfo.begin()); }
	else { inputPocket->itemInfo[0].number--; }
};

//상단 총알의 데이터 복사본을 반환
ItemData getTopBulletData(ItemData inputGun)
{
	ItemPocket* pocket = getBulletPocket(inputGun);
	errorBox(pocket->itemInfo.size() == 0, "function getTopBullet has executed with no bullet in ItemPocket.ixx");
	return pocket->itemInfo[0];
};

//이 총에 장전된 모든 총알의 갯수 반환, 탄창일 때도 할것
int getBulletNumber(ItemData inputGun)
{
	ItemPocket* bulletPocket = getBulletPocket(inputGun);
	if (bulletPocket != nullptr)
	{
		int bulletNumber = 0;
		for (int i = 0; i < bulletPocket->itemInfo.size(); i++)
		{
			bulletNumber += bulletPocket->itemInfo[i].number;
		}
		return bulletNumber;
	}
	else { return 0; }
};

//입력한 포켓의 내부 아이템 수를 반환
int countPocketItemNumber(ItemPocket* inputPtr)
{
	int number = 0;
	for (int i = 0; i < inputPtr->itemInfo.size(); i++)
	{
		number += inputPtr->itemInfo[i].number;
	}
	return number;
};




