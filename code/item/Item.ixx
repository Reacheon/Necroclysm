export module Item;

import std;
import util;
import constVar;
import Light;
import ItemDataBase;
import Point;
import dir16;

/*
* ItemData.ixx
* ItemData와 ItemPocket의 선언을 포함한 파일
* 일단은 미봉책으로 두 선언을 합쳐둔다. 섵부르게 수정하지 말 것
*/

class ItemPocket;

//__int8 : -128~127
//__int16 : -32768 ~32767

export struct ItemData : public ItemDataBase
{
    ///////////////////비데이터베이스 변수////////////////////
    unsigned __int32 pocketVolume = 0;
    unsigned __int32 number = 1;
    unsigned __int32 lootSelect = 0;
    unsigned __int32 reactSelect = 0;

    std::unique_ptr<ItemPocket> pocketPtr;
    equipHandFlag equipState = equipHandFlag::none;
    unsigned __int8 modeCurrent = 0;

    unsigned __int16 unfinishItemCode = 0;
    unsigned __int8 unfinishPercent = 100;

    unsigned __int8 extraSprIndexSingle = 0; //벽의 연결로 추가되는 sprIndex(일단은 차량만 구현)
    unsigned __int8 extraSprIndex16 = 0; //16단위로 움직이는 추가 sprIndex

    unsigned __int16 rotatedCCW90ItemCode = 0;
    dir16 propDir16 = dir16::dir0;

    std::unique_ptr<Light> lightPtr;
    __int16 propHP = 100;
    float propFakeHP = 100.0f;

    double powerStorage = 0; //현재 저장된 전하 용량(파워뱅크용)
    
    bool checkFlag(itemFlag inputFlag) const;

    void addFlag(itemFlag inputFlag);

    void eraseFlag(itemFlag inputFlag);

    ItemData cloneForTransfer(int transferNumber) const;

    bool itemOverlay(const ItemData& tgtItem) const;

    ItemData();
    virtual ~ItemData();
    ItemData(ItemData&&) noexcept;
    ItemData& operator=(ItemData&&) noexcept = default;
    ItemData(const ItemData&) = delete;
    ItemData& operator=(const ItemData&) = delete;

    bool isPocketOnlyItem(int inputCode);

    int getVolume() const;
    int getSprIndex();
};

export ItemData cloneFromItemDex(ItemData& inputData, int transferNumber);


export class ItemPocket
{
private:
	storageType type;//스토리지의 타입, 0:일반_1:장비
public:
	static std::unique_ptr<ItemPocket> unlockRecipes;
	std::vector<ItemData> itemInfo;

	ItemPocket(storageType inputType);

	~ItemPocket();

	storageType getType();

	void eraseItemInfo(int index);

	//@brief 입력한 숫자만큼 아이템의 수를 제거합니다. 만약 0이 되면 목록에서 제거합니다. 아이템의 수보다 큰 값을 입력 시 오류가 발생, 동시에 Select가 더 클 경우 그 값을 맞춤
	void subtractItemIndex(int index, int number);

	//@brief 입력한 아이템 코드의 아이템을 number개 포켓에서 제거합니다. subtractItemIndex를 활용하는 재귀함수.
	void subtractItemCode(int code, int number);

	//@brief 입력한 storagePtr로 아이템을 number개 옮김(송신하는 Ptr은 이 메소드를 실행하는 인스턴스), 수신++, 송신--,  실제 아이템의 수보다 많이 넣으면 오류 발생
	//@param storagePtr 아이템을 받을 storage 포인터
	//@param index 보낼 아이템의 인덱스(송신 객체의 index번째 아이템임)
	//@return 수신받은 ItemPocket의 index
	int transferItem(ItemPocket* storagePtr, int index, int number);

	int addItemFromDex(int index, int number);
	int addItemFromDex(int index);

	void addItemFromDex(std::vector<Point2> inputVec);

	void addRecipe(int inputItemCode);

	void swap(int index1, int index2);

	void sortByUnicode(int startIndex, int endIndex);
	void sortByUnicode();

	void sortEquip();

	void sortWeightDescend(int startIndex, int endIndex);

	void sortWeightDescend();

	void sortWeightAscend(int startIndex, int endIndex);

	void sortWeightAscend();

	void sortVolumeDescend(int startIndex, int endIndex);

	void sortVolumeDescend();

	void sortVolumeAscend(int startIndex, int endIndex);

	void sortVolumeAscend();

	int searchTxt(std::wstring inputStr, int startIndex, int endIndex);
	int searchTxt(std::wstring inputStr);

	int searchFlag(itemFlag inputFlag, int startIndex, int endIndex);
	int searchFlag(itemFlag inputFlag);

	int searchCategory(itemCategory input, int startIndex, int endIndex);
	int searchCategory(itemCategory input);

	int searchSubcategory(itemSubcategory input, int startIndex, int endIndex);
	int searchSubcategory(itemSubcategory input);

	//현재 이 포켓의 하위포켓을 모두 체크해 가지고있는 아이템의 숫자 반환
	int numberItem(int inputCode, int currentDepth = 0);

	//현재 이 포켓의 하위포켓을 모두 체크하여 입력한 도구기술이 존재하는지 반환
	bool checkToolQuality(int input, int currentDepth = 0);

	int getPocketVolume();
	int getPocketWeight();
	int getPocketNumber();

	int countPocketItemNumber();

	//상단 총알의 데이터를 삭제
	void popTopBullet();

	void updateItems();

	//입력한 코드의 아이템의 인덱스를 반환
	int findItemIndex(int inputItemCode);
};

//현재 이 총에 장전된 모든 총알을 벡터 형태로 반환
export ItemPocket* getBulletPocket(ItemData& inputGun);

//이 총에 장전된 모든 총알의 갯수 반환, 탄창일 때도 할것
export int getBulletNumber(ItemData& inputGun);