export module Chunk;

import std;
import util;
import constVar;
import TileData;

export class Entity;
export class ItemStack;
export class Vehicle;
export class Chunk
{
private:
	std::array<std::array<TileData, chunkSizeY>, chunkSizeX> singleTile;
	chunkFlag flag = chunkFlag::none;
	weatherFlag chunkWeather = weatherFlag::sunny;
public:
	Chunk(chunkFlag input)
	{
		//prt(lowCol::green, L"Chunk : 생성자가 호출되었습니다..\n");
		
		for (int x = 0; x < chunkSizeX; x++)
		{
			for (int y = 0; y < chunkSizeY; y++)
			{
				singleTile[x][y].randomVal = randomRange(0, 65535);
			}
		}

		if (input == chunkFlag::seawater)
		{
			//prt(lowCol::blue, L"Chunk : 이 청크는 해수 타일이다..\n");
			for (int x = 0; x < chunkSizeX; x++)
			{
				for (int y = 0; y < chunkSizeY; y++)
				{
					singleTile[x][y].floor = tileFloorFlag::seawater;
				}
			}
		}
		else if (input == chunkFlag::none)
		{
			//prt(lowCol::blue, L"Chunk : 이 청크는 해수 타일이다..\n");
			for (int x = 0; x < chunkSizeX; x++)
			{
				for (int y = 0; y < chunkSizeY; y++)
				{
					singleTile[x][y].floor = 0;
				}
			}
		}
		else if (input == chunkFlag::underground)
		{
			for (int x = 0; x < chunkSizeX; x++)
			{
				for (int y = 0; y < chunkSizeY; y++)
				{
					singleTile[x][y].floor = 109;

					singleTile[x][y].setWall(302);
				}
			}
		}
		else
		{
			//prt(lowCol::blue, L"Chunk : 이 청크는 해수 타일이다..\n");
			for (int x = 0; x < chunkSizeX; x++)
			{
				for (int y = 0; y < chunkSizeY; y++)
				{
					singleTile[x][y].floor = tileFloorFlag::grass;
				}
			}
		}

		flag = input;
	}
	~Chunk()
	{
		//prt(lowCol::green, L"Chunk : 소멸자가 호출되었습니다..\n");
	}
	TileData& getChunkTile(int x, int y){return singleTile[y][x];}

	chunkFlag getFlag()
	{
		return flag;
	}
	void setFlag(chunkFlag input)
	{
		flag = input;
	}

	ItemStack* getChunkItemPos(int x, int y){return (ItemStack*)getChunkTile(x, y).ItemStackPtr;}
	void setChunkItemPos(int x, int y, ItemStack* inputPtr){getChunkTile(x, y).ItemStackPtr = inputPtr;}

	Vehicle* getChunkVehiclePos(int x, int y) { return (Vehicle*)getChunkTile(x, y).VehiclePtr; }
	void setChunkVehiclePos(int x, int y, Vehicle* inputPtr) { prt(lowCol::green, L"Chunk : %d,%d에 Vehicle %p를 배정했다.\n",x,y,inputPtr); getChunkTile(x, y).VehiclePtr = inputPtr; }
	
	std::vector<Entity*> getChunkEntityList()
	{
		std::vector<Entity*> entityList;
		for (int x = 0; x < chunkSizeX; x++)
		{
			for (int y = 0; y < chunkSizeY; y++)
			{
				if (getChunkTile(x, y).EntityPtr != nullptr) { entityList.push_back((Entity*)getChunkTile(x, y).EntityPtr); }
			}
		}
		return entityList;
	}

	std::vector<Vehicle*> getChunkVehicleList()
	{
		std::vector<Vehicle*> VehicleList;
		for (int x = 0; x < chunkSizeX; x++)
		{
			for (int y = 0; y < chunkSizeY; y++)
			{
				if (getChunkTile(x, y).VehiclePtr != nullptr)
				{
					VehicleList.push_back((Vehicle*)getChunkTile(x, y).VehiclePtr);
				}
			}
		}
		return VehicleList;
	}

	std::vector<ItemStack*> getChunkItemList()
	{
		std::vector<ItemStack*> itemList;
		for (int x = 0; x < chunkSizeX; x++)
		{
			for (int y = 0; y < chunkSizeY; y++)
			{
				if (getChunkTile(x, y).ItemStackPtr != nullptr) { itemList.push_back((ItemStack*)getChunkTile(x, y).ItemStackPtr); }
			}
		}
		return itemList;
	}

	weatherFlag getWeather() {
		return chunkWeather;
	}

	void setWeather(weatherFlag input) {
		chunkWeather = input;
	}
};