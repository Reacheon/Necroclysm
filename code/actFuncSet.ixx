export module actFuncSet;

import ItemData;
import ItemPocket;
import util;

//액트가 실행되는 환경은 3가지 경우가 가능
// 0:기본 HUD, 1:Loot, 2:Equip
export enum class actEnv
{
	HUD,
	Loot,
	Equip,
	Inventory
};

export namespace actFunc
{
	//장전 : 총이나 탄창에 사용, 자기 자신의 탄환을 채워넣음
	export Corouter reloadSelf(actEnv envType, ItemPocket* reloadItemPocket, int reloadItemCursor);
	//삽탄 : 총알에 사용, 이 탄환을 넣을 수 있는 탄창 리스트를 표시하고 거기에 넣음
	export Corouter reloadOther(actEnv envType, ItemPocket* reloadItemPocket, int reloadItemCursor);
	//장전해제 : 타겟아이템에 들어있는 아이템을 드랍하거나 인벤토리에 넣는다.
	export void unload(ItemPocket* unloadItemPocket, int unloadItemCursor);
	export void closeDoor(int tgtX, int tgtY, int tgtZ);
	export void closeVDoor(int tgtX, int tgtY, int tgtZ);
	export void toggle(ItemData& inputItem);
	export void drinkBottle(ItemData& inputData);
	export void eatFood(ItemPocket* inputPocket, int inputCursor);
	export void spillPocket(ItemData& inputData);
	export Corouter executeWield(ItemPocket* targetPocket, int targetPocketCursor);
	//던지기
	export Corouter executeThrowing(ItemPocket* inputPocket, int inputIndex);
	export void executeEquip(ItemPocket* sourcePocket, int sourceIndex);
	//배터리 장착 : 전자기기에 사용, 자신에게 배터리를 추가함
	export Corouter insertBattery(actEnv envType, ItemPocket* targetItemPocket, int targetItemCursor);
	//배터리 분리 : 전자기기 내부에 들어있는 배터리를 분리한다
	export void removeBattery(ItemPocket* unloadItemPocket, int unloadItemCursor);
	//BFS로 연결된 회로 네트워크의 와이어 표시/숨김 토글
	export void setWireVisibility(Point3 tgtPoint, bool hide);
	export void hideWire(Point3 tgtPoint);
	export void showWire(Point3 tgtPoint);
	export Corouter executePlant(ItemPocket* tgtPocket, int tgtIndex);
	//씨앗 추출 : SEED_FRUIT 과일에서 씨앗을 추출, Equip이면 바닥 드랍, Inventory면 가방에 넣되 부피초과시 바닥 드랍
	export void extractSeed(actEnv envType, ItemPocket* tgtPocket, int tgtIndex, int pocketMaxVolume = 0);
};