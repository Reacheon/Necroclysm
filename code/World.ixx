
#include <SDL_image.h>

export module World;

import std;
import util;
import Chunk;
import Mapmaker;
import constVar;
import TileData;

export class World
{
private:
	std::unordered_map<std::array<int, 3>, Chunk*, decltype(arrayHasher3)> chunkPtr;
	std::vector<Chunk*> activeChunk;
	std::unordered_set<std::array<int, 3>, decltype(arrayHasher3)> isSectorCreated;

public:
	static World* ins()//싱글톤 함수
	{
		static World* ptr = new World();
		return ptr;
	}
	World()
	{
		const int baseRange = 2;

		for (int y = -baseRange; y <= baseRange; y++)
		{
			for (int x = -baseRange; x <= baseRange; x++)
			{
				chunkPtr[{x, y, 0}] = new Chunk(chunkFlag::seawater);
			}
		}

		for (int y = -baseRange; y <= baseRange; y++)
		{
			for (int x = -baseRange; x <= baseRange; x++)
			{
				chunkPtr[{x, y, 1}] = new Chunk(chunkFlag::none);
			}
		}

		for (int y = -baseRange; y <= baseRange; y++)
		{
			for (int x = -baseRange; x <= baseRange; x++)
			{
				chunkPtr[{x, y, -1}] = new Chunk(chunkFlag::underground);
			}
		}
	}

	TileData& getTile(int x, int y, int z)
	{
		int chunkX, chunkY;
		changeToChunkCoord(x, y, chunkX, chunkY);
		return chunkPtr[{chunkX, chunkY, z}]->getChunkTile(x + ((-CHUNK_SIZE_X * chunkX) + ((CHUNK_SIZE_X - 1) / 2)), y + ((-CHUNK_SIZE_Y * chunkY) + ((CHUNK_SIZE_Y - 1) / 2)));
	}
	void createChunk(int chunkX, int chunkY, int chunkZ)
	{
		chunkFlag inputFlag = chunkFlag::seawater;
		if (chunkZ > 0) inputFlag = chunkFlag::none;
		else if (chunkZ < 0) inputFlag = chunkFlag::underground;
		if (Mapmaker::ins()->isEmptyProphecy(chunkX, chunkY, chunkZ) == false) inputFlag = Mapmaker::ins()->getProphecy(chunkX, chunkY, chunkZ);
		chunkPtr[{chunkX, chunkY, chunkZ}] = new Chunk(inputFlag);
	}
	bool existChunk(int chunkX, int chunkY, int chunkZ)
	{
		if (chunkPtr.find({ chunkX,chunkY,chunkZ }) != chunkPtr.end()) return true;
		else return false;
	}
	ItemStack* getItemPos(int x, int y, int z)
	{
		int chunkX, chunkY;
		changeToChunkCoord(x, y, chunkX, chunkY);
		return chunkPtr[{chunkX, chunkY, z}]->getChunkItemPos(x + ((-CHUNK_SIZE_X * chunkX) + ((CHUNK_SIZE_X - 1) / 2)), y + ((-CHUNK_SIZE_Y * chunkY) + ((CHUNK_SIZE_Y - 1) / 2)));
	}
	void setItemPos(int x, int y, int z, ItemStack* inputPtr)
	{
		int chunkX, chunkY;
		changeToChunkCoord(x, y, chunkX, chunkY);
		chunkPtr[{chunkX, chunkY, z}]->setChunkItemPos(x + ((-CHUNK_SIZE_X * chunkX) + ((CHUNK_SIZE_X - 1) / 2)), y + ((-CHUNK_SIZE_Y * chunkY) + ((CHUNK_SIZE_Y - 1) / 2)), inputPtr);
	}
	Vehicle* getVehiclePos(int x, int y, int z)
	{
		int chunkX, chunkY;
		changeToChunkCoord(x, y, chunkX, chunkY);
		return chunkPtr[{chunkX, chunkY, z}]->getChunkVehiclePos(x + ((-CHUNK_SIZE_X * chunkX) + ((CHUNK_SIZE_X - 1) / 2)), y + ((-CHUNK_SIZE_Y * chunkY) + ((CHUNK_SIZE_Y - 1) / 2)));
	}
	void setVehiclePos(int x, int y, int z, Vehicle* inputPtr)
	{
		int chunkX, chunkY;
		changeToChunkCoord(x, y, chunkX, chunkY);
		chunkPtr[{chunkX, chunkY, z}]->setChunkVehiclePos(x + ((-CHUNK_SIZE_X * chunkX) + ((CHUNK_SIZE_X - 1) / 2)), y + ((-CHUNK_SIZE_Y * chunkY) + ((CHUNK_SIZE_Y - 1) / 2)), inputPtr);
	}
	void changeToChunkCoord(int x, int y, int& chunkX, int& chunkY)
	{
		chunkX = (sgn(x)) * ((std::abs(x) + ((CHUNK_SIZE_X - 1) / 2)) / CHUNK_SIZE_X);
		chunkY = (sgn(y)) * ((std::abs(y) + ((CHUNK_SIZE_Y - 1) / 2)) / CHUNK_SIZE_Y);
	}
	void activate(int x, int y, int z)
	{
		activeChunk.push_back(chunkPtr[{x, y, z}]);
	}
	void deactivate()
	{
		activeChunk.clear();
	}

	std::vector<Entity*> getEntityList()
	{
		std::vector<Entity*> entityList;
		std::vector<Entity*> chunkEntityList;
		for (int i = 0; i < activeChunk.size(); i++)
		{
			chunkEntityList = activeChunk[i]->getChunkEntityList();
			for (int j = 0; j < chunkEntityList.size(); j++)
			{
				entityList.push_back(chunkEntityList[j]);
			}
		}
		return entityList;
	}
	std::vector<Vehicle*> getVehicleList()
	{
		__int64 startTime = getNanoTimer();
		std::vector<Vehicle*> VehicleList;
		std::vector<Vehicle*> chunkVehicleList;
		for (int i = 0; i < activeChunk.size(); i++)
		{
			chunkVehicleList = activeChunk[i]->getChunkVehicleList();
			for (int j = 0; j < chunkVehicleList.size(); j++)
			{
				VehicleList.push_back(chunkVehicleList[j]);
			}
		}
		std::unordered_set<Vehicle*> cache;
		auto it = VehicleList.begin();
		while (it != VehicleList.end())
		{
			if (cache.find(*it) != cache.end()) it = VehicleList.erase(it);
			else
			{
				cache.insert(*it);
				++it;
			}
		}
		return VehicleList;
	}
	std::vector<ItemStack*> getItemList()
	{
		std::vector<ItemStack*> itemList;
		std::vector<ItemStack*> chunkItemList;
		for (int i = 0; i < activeChunk.size(); i++)
		{
			chunkItemList = activeChunk[i]->getChunkItemList();
			for (int j = 0; j < chunkItemList.size(); j++)
			{
				itemList.push_back(chunkItemList[j]);
			}
		}
		return itemList;
	}

	//섹터 관련
	std::array<int, 2> changeToSectorCoord(int inputGridX, int inputGridY)
	{
		//-1200 ~ -801 : -3
		//-800 ~ -401 : -2
		//-400~ -1 : -1
		//0~399 : 0
		//400~799 : 1
		//800~1199 : 2

		int sectorX, sectorY;
		int chunkX, chunkY;

		//그리드 좌표를 청크로 변환
		changeToChunkCoord(inputGridX, inputGridY, chunkX, chunkY);
		
		//청크 좌표를 섹터 좌표로 변환
		if (chunkX > 0) sectorX = chunkX / SECTOR_SIZE;
		else sectorX = (chunkX + 1) / SECTOR_SIZE - 1;
		
		if (chunkY > 0) sectorY = chunkY / SECTOR_SIZE;
		else sectorY = (chunkY + 1) / SECTOR_SIZE - 1;

		return { sectorX,sectorY };
	}

	bool isEmptySector(int sectorX, int sectorY, int sectorZ)
	{
		if (isSectorCreated.find({ sectorX,sectorY,sectorZ }) == isSectorCreated.end()) return true;
		else return false;
	}


	void createSector(int sectorX, int sectorY, int sectorZ)
	{
		if (sectorZ == 0)
		{
			if ((sectorY <= 26 && sectorY >= -27)&&(sectorX <= 53 && sectorX >= -54))
			{
				std::string filePath = "map/worldSector-";
				int number = 2971 + sectorX + 108 * sectorY;
				if (number < 100) filePath += "0";
				filePath += std::to_string(number);
				filePath += ".png";
				std::wstring wPath(filePath.begin(), filePath.end());
				//std::wprintf(L"[World] Sector : %ls의 파일을 읽어내었다.\n", wPath.c_str());
				SDL_Surface* refSector = IMG_Load(filePath.c_str());
				errorBox(refSector == NULL, L"섹터의 파일 읽기가 실패하였습니다. :" + std::to_wstring(sectorX) + L"," + std::to_wstring(sectorY) + L"," + std::to_wstring(sectorZ));
				Uint32* pixels = (Uint32*)refSector->pixels;


				for (int x = 0; x < 400; x++)
				{
					for (int y = 0; y < 400; y++)
					{
						chunkFlag targetFlag = chunkFlag::none;

						Uint32 pixel = pixels[(y * refSector->w) + x];
						SDL_Color pixelCol;
						SDL_GetRGB(pixel, refSector->format, &pixelCol.r, &pixelCol.g, &pixelCol.b);

						if (pixelCol.r == chunkCol::seawater.r && pixelCol.g == chunkCol::seawater.g && pixelCol.b == chunkCol::seawater.b)
						{
							targetFlag = chunkFlag::seawater;
						}
						
						//섹터 좌표로 청크 좌표 구하기
						int chunkOriginX, chunkOriginY;
						chunkOriginX = 400 * sectorX;
						chunkOriginY = 400 * sectorY;

						Mapmaker::ins()->addProphecy(chunkOriginX + x, chunkOriginY + y, sectorZ, targetFlag);
					}
				}

				SDL_FreeSurface(refSector);
			}
		}

		isSectorCreated.insert({ sectorX,sectorY,sectorZ });
	}

	//void createSector(int sectorX, int sectorY, int sectorZ)
	//{
	//	if (sectorZ == 0)
	//	{
	//		if ((sectorY <= 26 && sectorY >= -27) && (sectorX <= 53 && sectorX >= -54))
	//		{
	//			std::string filePath = "map/worldSector-";
	//			int number = 2971 + sectorX + 108 * sectorY;
	//			if (number < 100) filePath += "0";
	//			filePath += std::to_string(number);
	//			filePath += ".png";
	//			std::wstring wPath(filePath.begin(), filePath.end());
	//			std::wprintf(L"[World] Sector : %ls의 파일을 읽어내었다.\n", wPath.c_str());
	//			SDL_Surface* refSector = IMG_Load(filePath.c_str());
	//			errorBox(refSector == NULL, L"섹터의 파일 읽기가 실패하였습니다. :" + std::to_wstring(sectorX) + L"," + std::to_wstring(sectorY) + L"," + std::to_wstring(sectorZ));
	//			Uint32* pixels = (Uint32*)refSector->pixels;

	//			const int sectorPixelW = 400;
	//			const int sectorPixelH = 400;

	//			std::vector<Point2> tasksVec;
	//			tasksVec.reserve((sectorPixelW + 1) * (sectorPixelH + 1));
	//			for (int tgtX = 0; tgtX < sectorPixelW; tgtX++)
	//			{
	//				for (int tgtY = 0; tgtY < sectorPixelH; tgtY++)
	//				{
	//					tasksVec.push_back({ tgtX,tgtY });
	//				}
	//			}

	//			auto sectorPixelCalcWorker = [=](const std::vector<Point2>& points)
	//				{
	//					for (const auto& point : points)
	//					{
	//						chunkFlag targetFlag = chunkFlag::none;

	//						Uint32 pixel = pixels[(point.y * refSector->w) + point.x];
	//						SDL_Color pixelCol;
	//						SDL_GetRGB(pixel, refSector->format, &pixelCol.r, &pixelCol.g, &pixelCol.b);

	//						if (pixelCol.r == chunkCol::seawater.r && pixelCol.g == chunkCol::seawater.g && pixelCol.b == chunkCol::seawater.b)
	//						{
	//							targetFlag = chunkFlag::seawater;
	//						}

	//						//섹터 좌표로 청크 좌표 구하기
	//						int chunkOriginX, chunkOriginY;
	//						chunkOriginX = 400 * sectorX;
	//						chunkOriginY = 400 * sectorY;

	//						Mapmaker::ins()->addProphecy(chunkOriginX + point.x, chunkOriginY + point.y, sectorZ, targetFlag);
	//					}
	//				};


	//			int numThreads = threadPoolPtr->getAvailableThreads();
	//			int chunkSize = tasksVec.size() / numThreads;
	//			for (int i = 0; i < numThreads; i++) {
	//				std::vector<Point2>::iterator startPoint = tasksVec.begin() + i * chunkSize;

	//				std::vector<Point2>::iterator endPoint;
	//				if (i == numThreads - 1) endPoint = tasksVec.end(); //만약 마지막 스레드일 경우 벡터의 끝을 강제로 설정
	//				else endPoint = startPoint + chunkSize;
	//				std::vector<Point2> chunk(startPoint, endPoint);

	//				threadPoolPtr->addTask([=]() {
	//					sectorPixelCalcWorker(chunk);
	//					});
	//			}

	//			threadPoolPtr->waitForThreads();

	//			SDL_FreeSurface(refSector);
	//		}
	//	}

	//	isSectorCreated.insert({ sectorX,sectorY,sectorZ });
	//}

	weatherFlag getChunkWeather(int chunkX, int chunkY, int chunkZ)
	{
		return chunkPtr[{chunkX, chunkY, chunkZ}]->getWeather();
	}

	void setChunkWeather(int chunkX, int chunkY, int chunkZ, weatherFlag input)
	{
		chunkPtr[{chunkX, chunkY, chunkZ}]->setWeather(input);
	}
};



