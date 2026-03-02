module;
#include <SDL3/SDL.h>

export module wrapFunc;

import std;
import util;
import constVar;
import statusEffect;
import ItemData;
import ItemPocket;
import ItemStack;
import Entity;
import Prop;
import Vehicle;

export int PlayerX();
export int PlayerY();
export int PlayerZ();

export const unsigned __int16 TileFloor(int x, int y, int z);
export const unsigned __int16 TileFloor(Point3 coord);
export const bool TileSnow(int x, int y, int z);
export const unsigned __int16 TileWall(int x, int y, int z);
export const bool ExistWall(int x, int y, int z);

export void setWall(Point3 coord, int val);
export void setFloor(Point3 coord, int val);

export Entity* TileEntity(int x, int y, int z);
export void EntityPtrMove(Point3 startCoor, Point3 endCoor);
export void EntityPtrMove(std::unique_ptr<Entity> inputPtr, Point3 endCoor);

export Prop* TileProp(int x, int y, int z);
export Prop* TileProp(Point3 pt);
export Vehicle*& TileVehicle(int x, int y, int z);
export Vehicle*& TileVehicle(Point3 pt);

export ItemStack* TileItemStack(int x, int y, int z);
export ItemStack* TileItemStack(Point3 pt);

export fovFlag& TileFov(int x, int y, int z);

export void createMonster(Point3 inputCoor, int inputEntityCode);
export void createItemStack(Point3 inputCoor);
export void createItemStack(Point3 inputCoor, std::vector<std::pair<int, int>> inputItems);
export void destroyItemStack(Point3 inputCoor);
export void destroyProp(Point3 inputCoor);
export void createProp(Point3 inputCoor, int inputItemCode);
export void createFlame(Point3 inputCoor, flameFlag inputFlag);

export void DestroyWall(int x, int y, int z);

export bool isWalkable(Point3 coord);
export bool isRayBlocker(Point3 coord);

export float getMouseX();
export float getMouseY();
export Point2 getAbsMouseGrid();

/*******************************************************************************
* 부피 관련 변수들
* 부피는 CONTAINER_FLEX로 인해 가변적이므로 반드시 래퍼 함수를 거쳐야 함
* ItemData의 originalVolume을 사용하는 코드가 이 래퍼함수 이외에 존재하면 제거할 것
 *******************************************************************************/

export int getVolume(const ItemData& inputData);
export void sortVolumeDescend(std::vector<ItemData>& inputInfo, int startIndex, int endIndex);
export void sortVolumeDescend(std::vector<ItemData>& inputInfo);
export void sortVolumeAscend(std::vector<ItemData>& inputInfo, int startIndex, int endIndex);
export void sortVolumeAscend(std::vector<ItemData>& inputInfo);

export bool checkStatusEffect(std::vector<statusEffect>& inputStatus, statusEffectFlag inputFlag);
export void eraseStatusEffect(std::vector<statusEffect>& inputStatus, statusEffectFlag inputFlag);

export int getItemSprIndex(ItemData& inputData);

export void changePlayerWalkMode(walkFlag inputMode);

//@brief 입력한 좌표의 타일에 해당한 아이템을 number개만큼 추가함. 아이템스택이 없다면 생성함. 있다면 그 스택에 추가함
export void addItemToTile(Point3 coord, int itemCode, int number);
//@brief 입력한 좌표의 타일에 inputPocket의 모든 아이템을 전송함. 아이템스택이 없다면 생성함. 있다면 그 스택에 추가함
export void addItemToTile(Point3 coord, ItemPocket* inputPokcet);

export int fluidTypeToCode(fluidType inputType);

export const bool isWetTile(Point3 coord);
export void updateWetTile(Point3 coord);
export void resetWetTile(Point3 coord);