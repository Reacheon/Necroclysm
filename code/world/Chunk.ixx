export module Chunk;

import std;
import util;
import constVar;
import TileData;

import Monster;
import Vehicle;
import ItemStack;
import Prop;

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
		chunkLoad(input);
	}
	~Chunk()
	{
	}
	TileData& getChunkTile(int x, int y) { return singleTile[y][x]; }

	void chunkLoad(chunkFlag inputChunk)
	{
		int floorVal = itemID::grass;
		int wallVal  = itemID::none;

		switch (inputChunk)
		{
		case chunkFlag::seawater:    floorVal = itemID::deepSeaWater; break;
		case chunkFlag::freshwater:  floorVal = itemID::deepFreshWater; break;
		case chunkFlag::none:        floorVal = itemID::none; break;
		case chunkFlag::underground: floorVal = itemID::dirt; wallVal = itemID::dirtWall; break;
		case chunkFlag::meadow:      floorVal = itemID::grass; break;
		case chunkFlag::dirt:        floorVal = itemID::dirt; break;
		case chunkFlag::city:        floorVal = itemID::dirt; break;
		case chunkFlag::bridge:      floorVal = itemID::dirt; break;
		default: break;
		}

		for (int x = 0; x < CHUNK_SIZE_X; x++)
		{
			for (int y = 0; y < CHUNK_SIZE_Y; y++)
			{
				singleTile[x][y].randomVal = randomRange(0, 65535);
				singleTile[x][y].floor = floorVal;
				if (wallVal != itemID::none) singleTile[x][y].setWall(wallVal);
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