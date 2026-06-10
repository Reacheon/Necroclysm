#include <SDL3/SDL.h>

import Craft;
import globalVar;
import constVar;
import util;
import Lst;
import Player;
import World;
import CoordSelect;
import CoordSelectCraft;
import log;
import Msg;
import Vehicle;
import Prop;
import turnWait;
import ItemData;

void Craft::openBookmarkDropdown()
{
	if (craftCursor < 0) return;

	//이미 북마크가 있으면 토글 해제
	if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK1)) { recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK1); return; }
	else if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK2)) { recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK2); return; }
	else if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK3)) { recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK3); return; }
	else if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK4)) { recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK4); return; }
	else if (recipePtr->itemInfo[craftCursor].checkFlag(itemFlag::BOOKMARK5)) { recipePtr->itemInfo[craftCursor].eraseFlag(itemFlag::BOOKMARK5); return; }

	//북마크가 없으면 드롭다운 열기
	bmDdOpen = true;
	bmDdRatio = 0.0f;

	int ddW = myMax(tooltipBookmarkBtn.w, 135);
	bmDdRect = { tooltipBookmarkBtn.x, tooltipBookmarkBtn.y + tooltipBookmarkBtn.h, ddW, BM_DD_BLOCK_H * BM_DD_COUNT };
}

void Craft::closeBookmarkDropdown()
{
	bmDdOpen = false;
	bmDdRatio = 0.0f;
}

void Craft::selectBookmark(int slotIndex)
{
	if (craftCursor < 0) return;
	switch (slotIndex)
	{
	case 0: recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK1); break;
	case 1: recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK2); break;
	case 2: recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK3); break;
	case 3: recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK4); break;
	case 4: recipePtr->itemInfo[craftCursor].addFlag(itemFlag::BOOKMARK5); break;
	}
}

Corouter Craft::executeCraft()
{
	bool negateMonster = false;

	// ============================================================
	// 1. 조합 좌표 및 아이템 설정
	// ============================================================

	if (craftCursor != -1)//최초 조합일 때
	{
		targetItemCode = recipePtr->itemInfo[craftCursor].itemCode;
		elapsedTime = 0;

		// ============================================================
		// 1-1. 재료, 도구, 재능 체크 및 제거
		// ============================================================
		if (debug::noCraftMaterialNeed == false)
		{
			if (canCraft(targetItemCode) == false)//재료,도구,재능을 만족하는지 체크
			{
				updateLog(sysStr[318]);//재료가 부족하다.
				co_return;
			}
			else//조합에 필요한 재료 제거
			{
				for (int i = 0; i < itemDex[targetItemCode].recipe.size(); i++)
				{
					//툴 퀄리티에 따라 적색, 녹색 변화
					int materialItemCode = itemDex[targetItemCode].recipe[i].first;
					int needNumber = itemDex[targetItemCode].recipe[i].second;
					PlayerEquip()->subtractItemCode(materialItemCode, needNumber);
				}
			}
		}

		// ============================================================
		// 1-2. 좌표 선택
		// ============================================================
		if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
		{
			if (itemDex[targetItemCode].checkFlag(itemFlag::VFRAME))
			{
				//새로운 차량을 설치하시겠습니까?
				std::vector<Point2> selectableTile;
				for (int dir = -1; dir < 8; dir++)
				{
					int dx, dy;
					dir2Coord(dir, dx, dy);
					if (TileVehicle(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == nullptr) selectableTile.push_back({ PlayerX() + dx, PlayerY() + dy });
				}

				if (selectableTile.size() > 0)
				{
					deactDraw();
					for (int i = 0; i < selectableTile.size(); i++) rangeSet.insert({ selectableTile[i].x,selectableTile[i].y });

					new CoordSelect(sysStr[303]);//차량 프레임을 설치할 위치를 선택해주세요.
					co_await std::suspend_always();
					rangeSet.clear();
					actDraw();

					if (coAnswer.empty() == false)
					{
						std::wstring targetStr = coAnswer;
						int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetZ = wtoi(targetStr.c_str());

						buildLocation = { targetX,targetY,PlayerZ() };

						createProp(buildLocation, itemID::craftingItem);

						TileProp(buildLocation)->leadItem.propSprIndex = itemDex[targetItemCode].craftWIPSprIndex;

						PlayerPtr->setFakeX(3 * (buildLocation.x - PlayerX()));
						PlayerPtr->setFakeY(3 * (buildLocation.y - PlayerY()));
						PlayerPtr->setDirection(coord2Dir(buildLocation.x - PlayerX(), buildLocation.y - PlayerY()));
					}
					else co_return;
				}
				else
				{
					updateLog(sysStr[300]);//주변에 차량 프레임을 설치할만한 공간이 없다.
					co_return;
				}
			}
			else if (itemDex[targetItemCode].checkFlag(itemFlag::VPART))
			{
				std::vector<Point2> selectableTile;
				for (int dir = -1; dir < 8; dir++)
				{
					int dx, dy;
					dir2Coord(dir, dx, dy);
					Vehicle* targetVehicle = TileVehicle(PlayerX() + dx, PlayerY() + dy, PlayerZ()); //차량부품이므로 이미 있는 프레임 위에 건설되어야 함
					if (targetVehicle != nullptr)
					{
						//설치 게이트(프레임 유무/우선도/차벽-천장 상호배제)를 통과하는 타일만 선택지에 노출
						if (targetVehicle->checkAddPart(PlayerX() + dx, PlayerY() + dy, targetItemCode).result == vehAddResult::ok)
						{
							selectableTile.push_back({ PlayerX() + dx, PlayerY() + dy });
						}
					}
				}

				if (selectableTile.size() > 0)
				{
					deactDraw();
					for (int i = 0; i < selectableTile.size(); i++) rangeSet.insert({ selectableTile[i].x,selectableTile[i].y });

					new CoordSelect(sysStr[304]);//차량 부품을 설치할 프레임을 선택해주세요.
					co_await std::suspend_always();
					rangeSet.clear();
					actDraw();

					if (coAnswer.empty() == false)
					{
						std::wstring targetStr = coAnswer;
						int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetZ = wtoi(targetStr.c_str());
						buildLocation = { targetX,targetY,targetZ };
					}
					else co_return;

				}
				else
				{
					updateLog(sysStr[301]);//주변에 차량 부품을 설치할만한 프레임이 없다.
					co_return;
				}
			}
			else
			{
				//새로운 차량을 설치하시겠습니까? HUD의 propInstall과 같은 조건임 수정할거면 같이 수정할 것
				std::vector<Point2> selectableTile;
				for (int dir = 0; dir < 8; dir++)
				{
					int dx, dy;
					dir2Coord(dir, dx, dy);
					if (TileProp(PlayerX() + dx, PlayerY() + dy, PlayerZ()) == nullptr) selectableTile.push_back({ PlayerX() + dx, PlayerY() + dy });
				}

				if (selectableTile.size() > 0)
				{
					deactDraw();
					for (int i = 0; i < selectableTile.size(); i++) rangeSet.insert({ selectableTile[i].x,selectableTile[i].y });

					new CoordSelectCraft(targetItemCode, sysStr[299], selectableTile);//조합할 아이템을 설치할 위치를 선택해주세요.
					co_await std::suspend_always();
					rangeSet.clear();
					actDraw();

					if (coAnswer.empty() == false)
					{
						std::wstring targetStr = coAnswer;
						int targetX = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						int targetY = wtoi(targetStr.substr(0, targetStr.find(L",")).c_str());
						targetStr.erase(0, targetStr.find(L",") + 1);
						targetItemCode = wtoi(targetStr.c_str());

						buildLocation = { targetX,targetY,PlayerZ() };

						createProp(buildLocation, itemID::craftingItem);

						TileProp(buildLocation)->leadItem.propSprIndex = itemDex[targetItemCode].craftWIPSprIndex;

						PlayerPtr->setFakeX(3 * (buildLocation.x - PlayerX()));
						PlayerPtr->setFakeY(3 * (buildLocation.y - PlayerY()));
						PlayerPtr->setDirection(coord2Dir(buildLocation.x - PlayerX(), buildLocation.y - PlayerY()));
					}
					else co_return;

				}
				else
				{
					updateLog(sysStr[302]);//주변에 해당 아이템을 설치할만한 공간이 없다.
					co_return;
				}
			}
		}
	}
	else // 조합 재개 : 최초로 조합창을 오픈할 때 조합데이터 확인
	{
		if (existCraftData())
		{



			int percent = (int)(100.0 * (float)ongoingElapsedTime / (float)itemDex[ongoingTargetCode].craftTime);
			//알림, (%number)%에서 조합을 중단한 아이템이 존재합니다.계속 조합하시겠습니까?, 계속, 아니오, 파기
			new Msg(msgFlag::normal, sysStr[307], replaceStr(sysStr[313], L"(%number)", std::to_wstring(percent)), { sysStr[312],sysStr[37],sysStr[314] }, ongoingTargetCode);
			deactColorChange = true;
			co_await std::suspend_always();
			deactColorChange = false;

			if (coAnswer.empty() == false)
			{
				if (coAnswer == sysStr[312])//계속
				{
					loadCraftData(targetItemCode, elapsedTime);
				}
				else if (coAnswer == sysStr[37])//아니오
				{
					close(aniFlag::null);
					co_return;
				}
				else
				{
					deleteCraftData();
					co_return;
				}
			}
			else
			{
				close(aniFlag::null);
				co_return;
			}
		}
		else if (existCraftDataStructure())
		{
			int dx = abs(PlayerX() - buildLocation.x);
			int dy = abs(PlayerY() - buildLocation.y);
			int dz = abs(PlayerZ() - buildLocation.z);

			if (dx <= 1 && dy <= 1 && dz == 0)
			{
				//알림,조합 중인 아이템이 주변에 있습니다.조합을 계속하시겠습니까? ,계속,아니오,파기
				new Msg(msgFlag::normal, sysStr[307], sysStr[315], { sysStr[312],sysStr[37],sysStr[314] }, ongoingTargetCodeStructure);
				deactColorChange = true;
				co_await std::suspend_always();
				deactColorChange = false;
				if (coAnswer.empty() == false)
				{
					if (coAnswer == sysStr[312])//계속
					{
						loadCraftDataStructure(targetItemCode, elapsedTime, buildLocation);
						prt(L"현재 빌드 로케이션의 좌표는 %d,%d,%d이다\n", buildLocation.x, buildLocation.y, buildLocation.z);
					}
					else if (coAnswer == sysStr[37])//아니오
					{
						close(aniFlag::null);
						co_return;
					}
					else
					{
						deleteCraftDataStructure();
						co_return;
					}
				}
				else
				{
					close(aniFlag::null);
					co_return;
				}
			}
			else
			{
				//떨어진 좌표 (▲,▲,▲)에 조합 중인 아이템이 존재합니다. 파기하고 새로운 아이템을 제작하시겠습니까?
				std::wstring text = replaceStr(sysStr[316], L"▲", { std::to_wstring(buildLocation.x),std::to_wstring(buildLocation.y),std::to_wstring(buildLocation.z) });
				//알림,네, 아니오
				new Msg(msgFlag::normal, sysStr[307], text, { sysStr[36],sysStr[37] }, ongoingTargetCodeStructure);
				deactColorChange = true;
				co_await std::suspend_always();
				deactColorChange = false;

				if (coAnswer.empty() == false)
				{
					if (coAnswer == sysStr[36])//네
					{
						deleteCraftDataStructure();
						co_return;
					}
					else//아니오
					{
						close(aniFlag::null);
						co_return;
					}
				}
				else
				{
					close(aniFlag::null);
					co_return;
				}
			}

		}
	}

	// ============================================================
	// 2. 좌표 선정 완료 후 조합 루프
	// ============================================================
	if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT)) isNowBuilding = true;
	deactDraw();
	showCraftingTooltip = true;
	while (1)
	{
		if (negateMonster == false)
		{
			for (int x = PlayerX() - 1; x <= PlayerX() + 1; x++)
			{
				for (int y = PlayerY() - 1; y <= PlayerY() + 1; y++)
				{
					if (!(x == PlayerX() && y == PlayerY()))
						if (TileFov(x, y, PlayerZ()) == fovFlag::white)
							if (TileEntity(x, y, PlayerZ()) != nullptr)
							{
								//경고, 주변에 적이 있습니다. 계속 조합하시겠습니까?
								new Msg(msgFlag::normal, sysStr[306], sysStr[310], { sysStr[36],sysStr[37],sysStr[311] });
								deactColorChange = true;
								co_await std::suspend_always();
								if (coAnswer.empty() == false)
								{
									if (coAnswer == sysStr[36]) goto loopEnd;//네
									else if (coAnswer == sysStr[311])//무시하기
									{
										negateMonster = true;
										goto loopEnd;
									}
									else//아니오
									{
										//조합 데이터 저장
										if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
										{
											saveCraftDataStructure(targetItemCode, elapsedTime, buildLocation);
											isNowBuilding = false;
										}
										else saveCraftData(targetItemCode, elapsedTime);
										coTurnSkip = false;
										close(aniFlag::null);
										co_return;
									}
								}
								else goto loopEnd; //탭 누르면 계속하는걸로 간주
							}
				}
			}
		}

	loopEnd:
		if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT) && itemDex[targetItemCode].checkFlag(itemFlag::VPART) == false)
		{
			int pSprIndex = TileProp(buildLocation)->leadItem.propSprIndex;
			float ratio = (float)elapsedTime / (float)itemDex[targetItemCode].craftTime;

			int baseIndex = (pSprIndex / 4) * 4;
			int progressIndex;

			if (ratio < 0.25) progressIndex = 0;
			else if (ratio < 0.5) progressIndex = 1;
			else if (ratio < 0.75) progressIndex = 2;
			else progressIndex = 3;

			TileProp(buildLocation)->leadItem.propSprIndex = baseIndex + progressIndex;
		}


		turnWait(1.0);
		coTurnSkip = true;

		co_await std::suspend_always();

		elapsedTime++;
		if (elapsedTime >= itemDex[targetItemCode].craftTime)
		{
			if (TileProp(buildLocation) != nullptr) destroyProp(buildLocation);
			break;
		}
	}

	// ============================================================
	// 3. 조합 종료 : 아이템 생성
	// ============================================================

	if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT))
	{
		if (itemDex[targetItemCode].checkFlag(itemFlag::VFRAME))//새로운 차량 설치
		{
			//설치점 주변에 연결 가능한 차량이 있는지 체크
			std::vector<Vehicle*> canConnect;
			for (int dir = 0; dir < 8; dir++)
			{
				if (dir % 2 == 1) continue; //대각선 비허용
				int dx, dy;
				dir2Coord(dir, dx, dy);
				Vehicle* targetCoordPtr = TileVehicle(buildLocation.x + dx, buildLocation.y + dy, buildLocation.z);
				if (targetCoordPtr != nullptr)
				{
					if (std::find(canConnect.begin(), canConnect.end(), targetCoordPtr) == canConnect.end())
					{
						canConnect.push_back(targetCoordPtr);
					}
				}
			}

			if (canConnect.size() == 0)
			{
				Vehicle* newVehicle = World::ins()->createVehicle(buildLocation.x, buildLocation.y, buildLocation.z, targetItemCode);
				new Msg(msgFlag::input, sysStr[138], sysStr[137], { sysStr[139], sysStr[140] });//새로운 차량의 이름을 입력해주세요. 결정, 취소
				co_await std::suspend_always();
				if (coAnswer == sysStr[139]) newVehicle->name = exInputText;
				else newVehicle->name = sysStr[305]+L" " + std::to_wstring(randomRange(1, 9999999));//차량+번호
			}
			else
			{
				//확인, 설치한 프레임을 주변 차량에 연결하시겠습니까?, 네, 아니오
				new Msg(msgFlag::normal, sysStr[308], sysStr[309], { sysStr[36],sysStr[37] });
				co_await std::suspend_always();

				if (coAnswer == sysStr[36])//네
				{
					Vehicle* targetVehicle;
					if (canConnect.size() == 1)
					{
						targetVehicle = canConnect[0];
						targetVehicle->extendPart(buildLocation.x, buildLocation.y, targetItemCode);
					}
					else
					{
						std::vector<std::wstring> vehicleNameList;
						for (int i = 0; i < canConnect.size(); i++) vehicleNameList.push_back(canConnect[i]->name);
						new Lst(sysStr[95], sysStr[94], vehicleNameList);//넣기, 아이템을 넣을 포켓을 골라주세요.
						co_await std::suspend_always();

						int targetNumber = 0;
                        if (coAnswer.empty() == false) targetNumber = wtoi(coAnswer.c_str());
						errorBox(targetNumber >= canConnect.size() || targetNumber < 0, L"Lst error, unknown vehicle selected");
						targetVehicle = canConnect[targetNumber];
						targetVehicle->extendPart(buildLocation.x, buildLocation.y, targetItemCode);
					}
				}
				else
				{
					Vehicle* newVehicle = World::ins()->createVehicle(buildLocation.x, buildLocation.y, buildLocation.z, targetItemCode);
					new Msg(msgFlag::input, sysStr[138], sysStr[137], { sysStr[139], sysStr[140] });//새로운 차량의 이름을 입력해주세요. 결정, 취소
					co_await std::suspend_always();
					if (coAnswer == sysStr[139]) newVehicle->name = exInputText;
					else newVehicle->name = sysStr[305] + L" " + std::to_wstring(randomRange(1, 9999999));//차량+번호
				}
			}
		}
		else if (itemDex[targetItemCode].checkFlag(itemFlag::VPART))
		{
			Vehicle* targetVehicle = TileVehicle(buildLocation.x, buildLocation.y, buildLocation.z);
			errorBox(targetVehicle == nullptr, L"targetVehicle is nullptr in Craft.ixx");
			errorBox(!targetVehicle->hasFrame(buildLocation.x, buildLocation.y), L"first part doesn't have VFRAME flag");
			targetVehicle->addPart(buildLocation.x, buildLocation.y, targetItemCode);
		}
		else if (itemDex[targetItemCode].checkFlag(itemFlag::PROP))
		{
			errorBox(TileProp(buildLocation.x, buildLocation.y, buildLocation.z) != nullptr, L"Cannot install new structure because one already exists at this location.");
			createProp({ buildLocation.x, buildLocation.y, buildLocation.z }, targetItemCode);
		}
		else if (itemDex[targetItemCode].checkFlag(itemFlag::WALL))
		{
			int a = targetItemCode;
			setWall(buildLocation, targetItemCode);
		}
		else if (itemDex[targetItemCode].checkFlag(itemFlag::FLOOR))
		{
			setFloor(buildLocation, targetItemCode);
		}
	}

	//객체가 형성되고 1턴 쉬기
	//이는 프롭이 설치된 후 전자회로나 유체회로가 바로 작동하도록 하기 위함
	//만약 이 줄이 없을 경우 1턴 플레이어가 대기해야 회로들이 바뀐 회로에 적응함...
	//다만 1.0턴이 필요할 지는 고민해보자. 13턴 소모시간이면 위에서는 12턴을 대기해야할 지도?
	turnWait(1.0);

	// ============================================================
	// 4. 조합 데이터 초기화
	// ============================================================
	updateLog(replaceStr(sysStr[298], L"(%item)", itemDex[targetItemCode].name)); //(%item) 설치를 완료하였다.
	if (itemDex[targetItemCode].checkFlag(itemFlag::COORDCRAFT)) deleteCraftDataStructure();
	else deleteCraftData();



	isNowBuilding = false;
	close(aniFlag::null);
}
