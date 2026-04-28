export module Chunk;

import std;
import util;
import constVar;
import TileData;

import Monster;
import Vehicle;

export class Chunk
{
private:
	std::array<std::array<TileData, CHUNK_SIZE_Y>, CHUNK_SIZE_X> singleTile;
	chunkFlag flag = chunkFlag::none;
	weatherFlag chunkWeather = weatherFlag::sunny;
	
	std::unordered_set<Monster*> chunkMonsterSet;
    std::unordered_set<Vehicle*> chunkVehicleSet;
	std::unordered_set<Prop*> chunkPropSet;
	std::unordered_set<ItemStack*> chunkStackSet;
public:
	Chunk(chunkFlag input)
	{
		//prt(lowCol::green, L"Chunk : 생성자가 호출되었습니다..\n");
		chunkLoad(input);
	}
	~Chunk()
	{
		//prt(lowCol::green, L"Chunk : 소멸자가 호출되었습니다..\n");
	}
	TileData& getChunkTile(int x, int y) { return singleTile[y][x]; }

	void chunkLoad(chunkFlag inputChunk)
	{
		unsigned __int16 floorVal = itemID::grass;
		unsigned __int16 wallVal = 0;

		switch (inputChunk)
		{
		case chunkFlag::seawater:    floorVal = itemID::deepSeaWater; break;
		case chunkFlag::freshwater:  floorVal = itemID::deepFreshWater; break;
		case chunkFlag::none:        floorVal = 0; break;
		case chunkFlag::underground: floorVal = 109; wallVal = 302; break;
		case chunkFlag::meadow:      floorVal = 220; break;
		case chunkFlag::dirt:        floorVal = 109; break;
		case chunkFlag::city:        floorVal = 109; break;
		case chunkFlag::bridge:      floorVal = 109; break;
		default: break;
		}

		for (int x = 0; x < CHUNK_SIZE_X; x++)
		{
			for (int y = 0; y < CHUNK_SIZE_Y; y++)
			{
				singleTile[x][y].randomVal = randomRange(0, 65535);
				singleTile[x][y].floor = floorVal;
				if (wallVal != 0) singleTile[x][y].setWall(wallVal);
			}
		}

		flag = inputChunk;
	}

	chunkFlag getFlag()
	{
		return flag;
	}
	void setFlag(chunkFlag input)
	{
		flag = input;
	}


	weatherFlag getWeather() {
		return chunkWeather;
	}

	void setWeather(weatherFlag input) {
		chunkWeather = input;
	}

	chunkFlag getChunkFlag() {
		return flag;
	}


	//Monster 관련 함수들
	void addMonster(Monster* monster)
	{
		if (monster != nullptr) chunkMonsterSet.insert(monster);
	}
	const std::unordered_set<Monster*>& getMonsterSet() const { return chunkMonsterSet; }
	bool eraseMonster(Monster* monster) { return chunkMonsterSet.erase(monster) > 0; }

	// Vehicle 관련 함수들
	void addVehicle(Vehicle* vehicle)
	{
		if (vehicle != nullptr) chunkVehicleSet.insert(vehicle);
	}
	const std::unordered_set<Vehicle*>& getVehicleSet() const { return chunkVehicleSet; }
	bool eraseVehicle(Vehicle* vehicle) { return chunkVehicleSet.erase(vehicle) > 0; }

	// Prop 관련 함수들
	void addProp(Prop* prop)
	{
		if (prop != nullptr) chunkPropSet.insert(prop);
	}
	const std::unordered_set<Prop*>& getPropSet() const { return chunkPropSet; }
	bool eraseProp(Prop* prop) { return chunkPropSet.erase(prop) > 0; }

	// ItemStack 관련 함수들
	void addStack(ItemStack* stack)
	{
		if (stack != nullptr) chunkStackSet.insert(stack);
	}
	const std::unordered_set<ItemStack*>& getStackSet() const { return chunkStackSet; }
	bool eraseStack(ItemStack* stack) { return chunkStackSet.erase(stack) > 0; }
};