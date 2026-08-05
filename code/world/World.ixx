module;
#include <SDL3_image/SDL_image.h>

export module World;

import std;
import util;
import Chunk;
import constVar;
import TileData;

import Vehicle;
import Prop;
import Monster;
import ItemStack;
import Entity;
import ItemPocket;

export class World
{
private:
	std::unordered_map<Point3, std::unique_ptr<Chunk>, Point3::Hash> chunkPtr;
	std::vector<Chunk*> activeChunk; // 비소유 포인터
	std::unordered_map<uint32_t, std::unique_ptr<Vehicle>> vehicleOwnerMap;
	uint32_t vehicleIdCounter = 0;

public:
	static World* ins()//싱글톤 함수
	{
		static World* ptr = new World();
		return ptr;
	}

	World(const World&) = delete;
	World& operator=(const World&) = delete;

private:
	World()
	{
		const int baseRange = 4;

		for (int y = -baseRange; y <= baseRange; y++)
		{
			for (int x = -baseRange; x <= baseRange; x++)
			{
				chunkPtr[{x, y, 0}] = std::make_unique<Chunk>(chunkFlag::seawater);
			}
		}

		for (int y = -baseRange; y <= baseRange; y++)
		{
			for (int x = -baseRange; x <= baseRange; x++)
			{
				chunkPtr[{x, y, 1}] = std::make_unique<Chunk>(chunkFlag::none);
			}
		}

		for (int y = -baseRange; y <= baseRange; y++)
		{
			for (int x = -baseRange; x <= baseRange; x++)
			{
				chunkPtr[{x, y, -1}] = std::make_unique<Chunk>(chunkFlag::underground);
			}
		}
	}

public:
	TileData& getTile(int x, int y, int z)
	{
		int chunkX, chunkY;
		changeToChunkCoord(x, y, chunkX, chunkY);
		int localX = x - (chunkX * CHUNK_SIZE_X);
		int localY = y - (chunkY * CHUNK_SIZE_Y);
		return chunkPtr.at({chunkX, chunkY, z})->getChunkTile(localX, localY);
	}
	TileData& getTile(Point3 inputCoor)
	{
		return getTile(inputCoor.x, inputCoor.y, inputCoor.z);
	}

	// 청크 1개 생성 — z별 디폴트 채움. 정의는 World_createChunk.cpp.
	void createChunk(int chunkX, int chunkY, int chunkZ);
	bool existChunk(int chunkX, int chunkY, int chunkZ)
	{
		if (chunkPtr.find({ chunkX,chunkY,chunkZ }) != chunkPtr.end()) return true;
		else return false;
	}
	void changeToChunkCoord(int x, int y, int& chunkX, int& chunkY)
	{
		chunkX = (x >= 0)
			? (x / CHUNK_SIZE_X)
			: ((x - (CHUNK_SIZE_X - 1)) / CHUNK_SIZE_X);

		chunkY = (y >= 0)
			? (y / CHUNK_SIZE_Y)
			: ((y - (CHUNK_SIZE_Y - 1)) / CHUNK_SIZE_Y);

	}
	Point2 changeToChunkCoord(int x, int y)
	{
		int chunkX = (x >= 0)
			? (x / CHUNK_SIZE_X)
			: ((x - (CHUNK_SIZE_X - 1)) / CHUNK_SIZE_X);

		int chunkY = (y >= 0)
			? (y / CHUNK_SIZE_Y)
			: ((y - (CHUNK_SIZE_Y - 1)) / CHUNK_SIZE_Y);

		return Point2{ chunkX,chunkY };
	}
	void activate(int x, int y, int z)
	{
		activeChunk.push_back(chunkPtr.at({x, y, z}).get());
	}
	void deactivate()
	{
		activeChunk.clear();
	}

	std::vector<Chunk*>& getActiveChunk()
	{
		return activeChunk;
	}

	std::unordered_set<Monster*> getActiveMonsterSet()
	{
		std::unordered_set<Monster*> totalMonsterSet;
		const std::unordered_set<Monster*>* chunkMonsterSet;
		for (int i = 0; i < activeChunk.size(); i++)
		{
			chunkMonsterSet = &(activeChunk[i]->getMonsterSet());
			for (auto monster : *chunkMonsterSet)
			{
				totalMonsterSet.insert(monster);
			}
		}
		return totalMonsterSet;
	}

	std::unordered_set<Vehicle*> getActiveVehicleSet()
	{
		std::unordered_set<Vehicle*> totalVehicleSet;
		const std::unordered_set<Vehicle*>* chunkVehicleSet;
		for (int i = 0; i < activeChunk.size(); i++)
		{
			chunkVehicleSet = &(activeChunk[i]->getVehicleSet());
			for (auto vehicle : *chunkVehicleSet)
			{
				totalVehicleSet.insert(vehicle);
			}
		}
		return totalVehicleSet;
	}

	std::unordered_set<Prop*> getActivePropSet()
	{
		std::unordered_set<Prop*> totalPropSet;
		const std::unordered_set<Prop*>* chunkPropSet;
		for (int i = 0; i < activeChunk.size(); i++)
		{
			chunkPropSet = &(activeChunk[i]->getPropSet());
			for (auto prop : *chunkPropSet)
			{
				totalPropSet.insert(prop);
			}
		}
		return totalPropSet;
	}

	std::unordered_set<ItemStack*> getActiveStackSet()
	{
		std::unordered_set<ItemStack*> totalStackSet;
		const std::unordered_set<ItemStack*>* chunkStackSet;
		for (int i = 0; i < activeChunk.size(); i++)
		{
			chunkStackSet = &(activeChunk[i]->getStackSet());
			for (auto stack : *chunkStackSet)
			{
				totalStackSet.insert(stack);
			}
		}
		return totalStackSet;
	}

	chunkFlag getChunkFlag(int chunkX, int chunkY, int chunkZ)
	{
		return chunkPtr.at({chunkX, chunkY, chunkZ})->getChunkFlag();
	}
	weatherFlag getChunkWeather(int chunkX, int chunkY, int chunkZ)
	{
		return chunkPtr.at({chunkX, chunkY, chunkZ})->getWeather();
	}

	void setChunkWeather(int chunkX, int chunkY, int chunkZ, weatherFlag input)
	{
		chunkPtr.at({chunkX, chunkY, chunkZ})->setWeather(input);
	}

	void chunkOverwrite(int chunkX, int chunkY, int chunkZ, chunkFlag inputChunk)
	{
		chunkPtr.at({chunkX, chunkY, chunkZ})->chunkLoad(inputChunk);
	}

	Chunk& getChunk(int chunkX, int chunkY, int chunkZ)
	{
		return *chunkPtr.at({chunkX, chunkY, chunkZ});
	}

	// 청크 누락 시 nullptr 반환. 핫 루프에서 .at() 예외 비용 없이 안전하게 룩업할 때 사용
	Chunk* tryGetChunk(int chunkX, int chunkY, int chunkZ)
	{
		auto it = chunkPtr.find({chunkX, chunkY, chunkZ});
		return (it != chunkPtr.end()) ? it->second.get() : nullptr;
	}

	// 타일이 속한 청크가 없으면 nullptr 반환. 렌더링 등 로드 영역 경계를 넘어 이웃 타일을
	// 조회할 가능성이 있는 read-only 경로에서 사용 — 쓰기에는 getTile() 사용 (청크 보장).
	TileData* tryGetTile(int x, int y, int z)
	{
		int chunkX, chunkY;
		changeToChunkCoord(x, y, chunkX, chunkY);
		Chunk* chunk = tryGetChunk(chunkX, chunkY, z);
		if (chunk == nullptr) return nullptr;
		int localX = x - (chunkX * CHUNK_SIZE_X);
		int localY = y - (chunkY * CHUNK_SIZE_Y);
		return &chunk->getChunkTile(localX, localY);
	}

	Vehicle* createVehicle(int inputX, int inputY, int inputZ, int leadItemCode)
	{
		uint32_t id = vehicleIdCounter++;
		auto vehicle = std::make_unique<Vehicle>(inputX, inputY, inputZ, leadItemCode);
		vehicle->vehicleId = id;
		Vehicle* ptr = vehicle.get();
		vehicleOwnerMap[id] = std::move(vehicle);
		return ptr;
	}

	void destroyVehicle(uint32_t id)
	{
		auto it = vehicleOwnerMap.find(id);
		if (it != vehicleOwnerMap.end())
		{
			vehicleOwnerMap.erase(it);
		}
	}

	void destroyVehicle(Vehicle* target)
	{
		if (target != nullptr) destroyVehicle(target->vehicleId);
	}

};

////////////////////////////////////////////////////////////////////////////////
// 타일 접근 자유함수
////////////////////////////////////////////////////////////////////////////////

export int TileFloor(int x, int y, int z);
export int TileFloor(Point3 coord);
export bool TileSnow(int x, int y, int z);
export int TileWall(int x, int y, int z);
export bool ExistWall(int x, int y, int z);

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

//@brief 입력한 좌표의 타일에 해당한 아이템을 number개만큼 추가함. 아이템스택이 없다면 생성함. 있다면 그 스택에 추가함
export void addItemToTile(Point3 coord, int itemCode, int number);
//@brief 입력한 좌표의 타일에 inputPocket의 모든 아이템을 전송함. 아이템스택이 없다면 생성함. 있다면 그 스택에 추가함
export void addItemToTile(Point3 coord, ItemPocket* inputPokcet);

export bool isWetTile(Point3 coord);
export void updateWetTile(Point3 coord);
export void resetWetTile(Point3 coord);
