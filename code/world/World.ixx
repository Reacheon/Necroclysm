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
import VehiclePlan;

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
		// chunkX/Y는 raw input(음수·W 초과 가능)으로 floorDiv. localX 계산은
		// raw chunkX 기준이어야 [0,16) 로컬 좌표가 제대로 나온다 — render-space
		// 에서 들어온 tgtX=-50도 localX=14처럼 올바르게 매핑.
		// chunkPtr 룩업 직전에만 wrapChunkX로 X축 시암 정규화.
		int chunkX, chunkY;
		changeToChunkCoord(x, y, chunkX, chunkY);
		int localX = x - (chunkX * CHUNK_SIZE_X);
		int localY = y - (chunkY * CHUNK_SIZE_Y);
		return chunkPtr.at({worldWrap::wrapChunkX(chunkX), chunkY, z})->getChunkTile(localX, localY);
	}
	TileData& getTile(Point3 inputCoor)
	{
		return getTile(inputCoor.x, inputCoor.y, inputCoor.z);
	}

	// 청크 1개 생성 + (Phase 2 진입 후) Sector 데이터 기반 per-tile 페인트.
	// 정의는 World_createChunk.cpp.
	void createChunk(int chunkX, int chunkY, int chunkZ);
	bool existChunk(int chunkX, int chunkY, int chunkZ)
	{
		// X축 wrap — 시암 너머 chunkX(예: -3, W+5)도 같은 키로 룩업.
		chunkX = worldWrap::wrapChunkX(chunkX);
		if (chunkPtr.find({ chunkX,chunkY,chunkZ }) != chunkPtr.end()) return true;
		else return false;
	}
	// 주어진 z 의 모든 생성 청크 좌표를 콜백. 가시 그리드 스캔 대신
	//   생성된 청크만 순회할 때 사용 (월드맵에서 줌아웃 시 비용 폭주 방지).
	template<typename F>
	void forEachChunkAtZ(int z, F&& fn) const
	{
		for (auto& kv : chunkPtr)
			if (kv.first.z == z) fn(kv.first.x, kv.first.y);
	}
	// 모든 로드된 청크의 white fov 타일을 gray로 다운그레이드.
	// LotEditor가 렌더 영역 전체를 white로 강제 공개하므로, 종료 시 호출해
	// 정상 시야 상태(이전에 본 영역=gray)로 되돌린 뒤 updateVision으로 실제 가시영역만 다시 white로 만든다.
	void downgradeWhiteFovToGray()
	{
		for (auto& kv : chunkPtr)
		{
			Chunk* chunk = kv.second.get();
			for (int y = 0; y < CHUNK_SIZE_Y; y++)
			{
				for (int x = 0; x < CHUNK_SIZE_X; x++)
				{
					TileData& t = chunk->getChunkTile(x, y);
					if (t.fov == fovFlag::white) t.fov = fovFlag::gray;
				}
			}
		}
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
		activeChunk.push_back(chunkPtr.at({worldWrap::wrapChunkX(x), y, z}).get());
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

	// (Patch 시스템 제거됨 — 픽셀 데이터는 worldGrid mmap이 단일 진리원천.
	//  타이틀/Phase 1 미진입에는 createChunk가 chunkFlag::seawater 디폴트로 채움.
	//  Phase 1 이후엔 Sector 모듈이 mmap 경유로 per-tile 페인트 결정.)

	chunkFlag getChunkFlag(int chunkX, int chunkY, int chunkZ)
	{
		return chunkPtr.at({worldWrap::wrapChunkX(chunkX), chunkY, chunkZ})->getChunkFlag();
	}
	weatherFlag getChunkWeather(int chunkX, int chunkY, int chunkZ)
	{
		return chunkPtr.at({worldWrap::wrapChunkX(chunkX), chunkY, chunkZ})->getWeather();
	}

	void setChunkWeather(int chunkX, int chunkY, int chunkZ, weatherFlag input)
	{
		chunkPtr.at({worldWrap::wrapChunkX(chunkX), chunkY, chunkZ})->setWeather(input);
	}

	void chunkOverwrite(int chunkX, int chunkY, int chunkZ, chunkFlag inputChunk)
	{
		chunkPtr.at({worldWrap::wrapChunkX(chunkX), chunkY, chunkZ})->chunkLoad(inputChunk);
	}

	Chunk& getChunk(int chunkX, int chunkY, int chunkZ)
	{
		return *chunkPtr.at({worldWrap::wrapChunkX(chunkX), chunkY, chunkZ});
	}

	// 청크 누락 시 nullptr 반환. 핫 루프에서 .at() 예외 비용 없이 안전하게 룩업할 때 사용
	Chunk* tryGetChunk(int chunkX, int chunkY, int chunkZ)
	{
		auto it = chunkPtr.find({worldWrap::wrapChunkX(chunkX), chunkY, chunkZ});
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

	// 플레이어로부터 *radius* 청크 너머의 모든 청크를 제거. activeChunk도 클리어.
	//   WorldGen 후 startArea 잔여 청크를 날리는 용도.
	//   activeChunk는 클리어 후 다음 setGrid에서 재구성됨.
	void wipeOrphanedChunks(int playerChunkX, int playerChunkY, int playerZ, int radius)
	{
		// X축은 시암 wrap을 고려한 부호 있는 최단거리.
		const int normPlayerCX = worldWrap::wrapChunkX(playerChunkX);
		auto it = chunkPtr.begin();
		while (it != chunkPtr.end())
		{
			const Point3& c = it->first;
			const int dxWrap = worldWrap::signedDeltaChunkX(normPlayerCX, c.x);
			const bool farX  = std::abs(dxWrap)         > radius;
			const bool farY  = std::abs(c.y - playerChunkY) > radius;
			const bool diffZ = c.z != playerZ;
			if (farX || farY || diffZ)
				it = chunkPtr.erase(it);
			else
				++it;
		}
		activeChunk.clear();   // 남은 chunkPtr 기준으로 다음 setGrid가 재구성
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
//차량 spawn 헬퍼 — Lot→createChunk 파이프라인의 마지막 단계에서 호출.
//  anchor에 leadItem만 깐 빈 차량을 만든 뒤 plan.ops를 순서대로 replay한다
//  (extendPart/addPart/addCargo). bodyDir은 plan에 기록된 facing 그대로 둔다.
//  extendPart가 각 부품 타일의 TileVehicle도 함께 채우므로 별도 fixup 불필요.
export void createVehicleFromPlan(Point3 anchor, const VehiclePlan& plan);

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
