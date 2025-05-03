#include <SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module Alchemy;

import std;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import wrapVar;
import checkCursor;
import drawWindow;
import util;
import Vehicle;
import drawItemSlot;
import ItemData;
import ItemPocket;
import ItemStack;
import Player;
import World;
import Msg;
import Lst;
import const2Str;
import turnWait;

enum class selectFlag
{
	none,
	tool,
	react,
	prod,
};

struct ItemPath
{
	std::vector<__int64> path;
	std::wstring name = L"TEST";
	int sprIndex = 0;
	int number = 1;
	bool isFolder = false;
};

const int reactantMaxSize = 13; //조합 아이템은 최대 13종류까지 입력 가능, 결과물도 13종류

namespace dropDownList
{
	selectFlag mode = selectFlag::none;
	std::vector<ItemData*> itemVec;
	int scroll = 0;
	int initScroll = 0;
	SDL_Rect rect = { 0,0,0,0 };
};


export class Alchemy : public GUI
{
private:
	inline static Alchemy* ptr = nullptr;

	SDL_Rect alchemyBase;
	SDL_Rect alchemyStartBtn;

	SDL_Rect toolItemBtn;

	ItemData* toolPtr = nullptr;
	std::vector<ItemData*> reactPtrVec = { nullptr };

	SDL_Rect reactRect[reactantMaxSize] = { {0,0,0,0}, };
	SDL_Rect prodRect[reactantMaxSize] = { {0,0,0,0}, };

	SDL_Rect prodFirstRect;

	std::unique_ptr<ItemPocket> rootPathPocket;
	std::vector<ItemPocket*> currentPathPocket; //현재 폴더의 위치를 알려주는 벡터 C/FOLDER1/FOLDER2, 마지막은 현재 폴더

	int targetProductIndex = -1;
	int targetToolQuality = -1;

	std::unique_ptr<ItemPocket> prodPocket;

	bool showAlchemyTooltip = false;
	int elapsedTime = 0; //총괄 제작 시간
	int targetCraftingTime = 10;

	std::vector<ItemPath> allPaths; //모든 아이템경로들(하위 모두 포함)
public:
	Alchemy() : GUI(false)
	{
		prt(L"Alchemy : 생성자가 호출되었다.\n");
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this,aniFlag::winUnfoldOpen);




		
		std::function<void(ItemPocket*, std::vector<__int64>)> pathAdder = [&](ItemPocket* currentPocket, std::vector<__int64> currentPathIDs)
			{
				if (currentPocket==nullptr) return;

				for (const ItemData& item : currentPocket->itemInfo) 
				{
					if (item.codeID <= 0) continue;
					ItemPath pathEntry;
					pathEntry.sprIndex = item.sprIndex;
					pathEntry.name = item.name;
					pathEntry.number = item.number;
					pathEntry.isFolder = item.pocketPtr != nullptr;

					pathEntry.path = currentPathIDs;
					pathEntry.path.push_back(item.codeID);

					allPaths.push_back(pathEntry);

					if (item.pocketPtr != nullptr) pathAdder(item.pocketPtr.get(), pathEntry.path);
				}
			};

		ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		if(equipPtr != nullptr) pathAdder(equipPtr, {});

		//플레이어 Equip의 모든 아이템에 codeID(int)를 할당(중복되지 않도록), -1은 현재타일
		for (int dir = -1; dir < 8; dir++)
		{
			int dx, dy;
			dir2Coord(dir, dx, dy);
			if (TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ()) != nullptr)
			{
				ItemPocket* tilePocket = TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ())->getPocket();
				if (tilePocket != nullptr) pathAdder(tilePocket, {});
			}
		}

		rootPathPocket = std::make_unique<ItemPocket>(storageType::null);
		prodPocket = std::make_unique<ItemPocket>(storageType::null);
		currentPathPocket.push_back(rootPathPocket.get());
	}
	~Alchemy()
	{
		prt(L"Alchemy : 소멸자가 호출되었다.\n");
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Alchemy* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		alchemyBase = { 0, 0, 710, 376 };

		if (center == false)
		{
			alchemyBase.x += inputX;
			alchemyBase.y += inputY;
		}
		else
		{
			alchemyBase.x += inputX - alchemyBase.w / 2;
			alchemyBase.y += inputY - alchemyBase.h / 2;
		}

		alchemyStartBtn = { alchemyBase.x + 254 , alchemyBase.y + 258, 202, 74 };

		toolItemBtn = { alchemyBase.x + 247, alchemyBase.y + 198, 210, 18 };

		for (int i = 0; i < reactantMaxSize; i++) reactRect[i] = { alchemyBase.x + 10 + 6, alchemyBase.y + 59 + 8 + 21 * i, 210, 18 };

		for (int i = 0; i < reactantMaxSize; i++) prodRect[i] = { alchemyBase.x + 476 + 6, alchemyBase.y + 59 + 8 + 21 * i, 210, 18 };

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - alchemyBase.w / 2;
			y = inputY - alchemyBase.h / 2;
		}
	}
	void drawGUI()
	{
		//if (showAlchemyTooltip)
		//{
		//	SDL_Rect tooltipBox = { cameraW / 2 - 140, 0, 280,130 };
		//	drawWindow(&tooltipBox);
		//	setZoom(1.0);
		//	int markerIndex = 0;
		//	if (Msg::ins() == nullptr)
		//	{
		//		if (timer::timer600 % 25 < 2) { markerIndex = 0; }
		//		else if (timer::timer600 % 25 < 4) { markerIndex = 1; }
		//		else if (timer::timer600 % 25 < 6) { markerIndex = 2; }
		//		else if (timer::timer600 % 25 < 8) { markerIndex = 3; }
		//		else if (timer::timer600 % 25 < 10) { markerIndex = 4; }
		//		else if (timer::timer600 % 25 < 12) { markerIndex = 5; }
		//		else if (timer::timer600 % 25 < 14) { markerIndex = 6; }
		//		else if (timer::timer600 % 25 < 16) { markerIndex = 7; }
		//		else if (timer::timer600 % 25 < 18) { markerIndex = 8; }
		//		else if (timer::timer600 % 25 < 20) { markerIndex = 9; }
		//		else if (timer::timer600 % 25 < 22) { markerIndex = 10; }
		//		else if (timer::timer600 % 25 < 24) { markerIndex = 11; }
		//		else { markerIndex = 0; }
		//	}
		//	drawSprite(spr::loadingAnime, markerIndex, tooltipBox.x + tooltipBox.w / 2 - 78, tooltipBox.y + 6);
		//	setFontSize(13);
		//	drawText(col2Str(col::white) + L"아이템 조합 중...", tooltipBox.x + tooltipBox.w / 2 - 40, tooltipBox.y + 14);

		//	std::wstring reactStr = col2Str(lowCol::yellow) + L"반응물 : ";
		//	reactStr += col2Str(col::white);
		//	for (int i = 0; i < reactPtrVec.size(); i++)
		//	{
		//		if (reactPtrVec[i] != nullptr)
		//		{
		//			reactStr += reactPtrVec[i]->name;

		//			if (i != reactPtrVec.size() - 1)
		//			{
		//				if (reactPtrVec[i + 1] != nullptr)
		//				{
		//					reactStr += L", ";
		//				}
		//			}
		//		}
		//	}
		//	std::wstring toolStr = col2Str(lowCol::yellow) + L"도구 기술 : ";
		//	toolStr += col2Str(col::white);
		//	if (targetToolQuality == -1) {toolStr += L"없음";}
		//	else
		//	{
		//		toolStr += toolQuality2String(targetToolQuality);
		//	}
		//	drawText(reactStr, tooltipBox.x + tooltipBox.w / 2 - 120, tooltipBox.y + 40);
		//	drawText(toolStr, tooltipBox.x + tooltipBox.w / 2 - 120, tooltipBox.y + 58);

		//	SDL_Rect tooltipGauge = { cameraW / 2 - 130, 100, 260,16 };
		//	drawRect(tooltipGauge, col::white);

		//	SDL_Rect tooltipInGauge = { cameraW / 2 - 130 + 4, 100 + 4, 252,8 };
		//	tooltipInGauge.w = 252.0 * ((float)elapsedTime / (float)targetCraftingTime);
		//	drawFillRect(tooltipInGauge, col::white);

		//	setFontSize(11);
		//	std::wstring topText = std::to_wstring(targetCraftingTime - elapsedTime);
		//	topText += L" 분 남음 ( ";
		//	topText += std::to_wstring((int)(((float)elapsedTime * 100.0 / (float)targetCraftingTime)));
		//	topText += L"% )";

		//	drawTextCenter(col2Str(col::white) + topText, tooltipGauge.x + tooltipGauge.w / 2, tooltipGauge.y - 10);
		//}

		//if (getStateDraw() == false) { return; }

		//if (getFoldRatio() == 1.0)
		//{
		//	drawWindow(&alchemyBase, sysStr[134], 8);

		//	auto cursorCheck = [](SDL_Rect checkRect)->cursorFlag {
		//		if (checkCursor(&checkRect))
		//		{
		//			//다른 코루틴 객체가 존재하는지 확인
		//			if (Msg::ins() == nullptr && Lst::ins() == nullptr)
		//			{
		//				if (click == true) { return cursorFlag::click; }
		//				else { return cursorFlag::hover; }
		//			}
		//			else return cursorFlag::none;
		//		}
		//		else return cursorFlag::none;
		//		};



		//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	///////////////////////////////////////////반응물///////////////////////////////////////////////////////////////////
		//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	Point2 reactPivot = { alchemyBase.x + 10, alchemyBase.y + 59 };
		//	setFontSize(11);
		//	drawTextCenter(col2Str(col::white) + L"반응물", reactPivot.x + 109, reactPivot.y - 9);
		//	drawSprite(spr::alchemyMaterialEdge, 0, reactPivot.x - 1, reactPivot.y - 1);
		//	for (int i = 0; i < myMin(reactPtrVec.size(), reactantMaxSize); i++)
		//	{
		//		SDL_Rect targetRect = { reactRect[i].x, reactRect[i].y,210,18 };
		//		if (reactPtrVec[i] == nullptr) drawSimpleItemRectAdd(cursorCheck(targetRect), targetRect.x, targetRect.y);
		//		else
		//		{
		//			std::wstring indivItemName = L"";
		//			if (reactPtrVec[i]->reactSelect >= 2)
		//			{
		//				indivItemName = std::to_wstring(reactPtrVec[i]->reactSelect);
		//				indivItemName += L" ";
		//			}
		//			indivItemName += reactPtrVec[i]->name;

		//			drawSimpleItemRect(cursorCheck(targetRect), targetRect.x, targetRect.y, reactPtrVec[i]->sprIndex, col2Str(col::white) + indivItemName, false);
		//		}
		//	}

		//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	///////////////////////////////////////////생성물///////////////////////////////////////////////////////////////////
		//	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	Point2 prodPivot = { alchemyBase.x + 476, alchemyBase.y + 59 };
		//	setFontSize(11);
		//	drawTextCenter(col2Str(col::white) + L"생성물", prodPivot.x + 109, prodPivot.y - 9);
		//	drawSprite(spr::alchemyMaterialEdge, 0, prodPivot.x - 1, prodPivot.y - 1);
		//	
		//	for (int i = 0; i < myMin(prodPocket->itemInfo.size(), reactantMaxSize); i++)
		//	{
		//		SDL_Rect targetRect = { prodRect[i].x, prodRect[i].y,210,18 };
		//		std::wstring indivItemName;

		//		if (prodPocket->itemInfo[i].number > 1)
		//		{
		//			indivItemName = std::to_wstring(prodPocket->itemInfo[i].number);
		//			indivItemName += L" ";
		//		}
		//		indivItemName += prodPocket->itemInfo[i].name;
		//		drawSimpleItemRect(cursorCheck(targetRect), targetRect.x, targetRect.y, prodPocket->itemInfo[i].sprIndex, col2Str(col::white) + indivItemName, false);
		//	}

		//	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	///////////////////////////////////////////도구//////////////////////////////////////////////////////////////////////
		//	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	SDL_Point toolPivot = { alchemyBase.x + 248, alchemyBase.y + 89 };
		//	setFontSize(11);
		//	drawTextCenter(col2Str(col::white) + L"도구", alchemyBase.x + 350, alchemyBase.y + 182);




		//	drawSprite(spr::alchemyArrow, 0, alchemyBase.x + 242, alchemyBase.y + 181);

		//	//새로운 부품 추가 버튼
		//	if (toolPtr == nullptr)
		//	{
		//		drawSimpleItemRectAdd(cursorCheck(toolItemBtn), toolItemBtn.x, toolItemBtn.y);
		//	}
		//	else
		//	{
		//		std::wstring indivItemName;
		//		indivItemName += L"#CEC327";
		//		indivItemName += toolQuality2String(targetToolQuality);
		//		indivItemName += L"#FFFFFF";
		//		indivItemName += toolPtr->name;
		//		drawSimpleItemRect(cursorCheck(toolItemBtn), toolItemBtn.x, toolItemBtn.y, toolPtr->sprIndex, L"#CEC327증류 #FFFFFF증류기", false);
		//	}

		//	//355,106
		//	SDL_Rect toolFigure = { alchemyBase.x + 355 - 48,alchemyBase.y + 106 - 48, 96,96 };
		//	drawWindow(&toolFigure);

		//	setZoom(2.0);
		//	int toolSprIndex = 0;
		//	switch (targetToolQuality)
		//	{
		//	case toolQuality::boiling:
		//		toolSprIndex = 54;
		//		break;
		//	case toolQuality::distillation:
		//		toolSprIndex = 53;
		//		break;
		//	case toolQuality::frying:
		//		toolSprIndex = 49;
		//		break;
		//	case toolQuality::heating:
		//		toolSprIndex = 51;
		//		break;
		//	default:
		//		toolSprIndex = 0;
		//		break;
		//	}
		//	drawSpriteCenter(spr::icon48, toolSprIndex, alchemyBase.x + 355, alchemyBase.y + 106);
		//	setZoom(1.0);

		//	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	/////////////////////////////////////////////조합하기 버튼///////////////////////////////////////////////////////////
		//	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//	int startSprIndex = 0;
		//	if (reactPtrVec.size() > 1)
		//	{
		//		switch (cursorCheck(alchemyStartBtn))
		//		{
		//		default:
		//			startSprIndex = 0;
		//			break;
		//		case cursorFlag::hover:
		//			startSprIndex = 1;
		//			break;
		//		case cursorFlag::click:
		//			startSprIndex = 2;
		//			break;
		//		}
		//	}

		//	drawSprite(spr::alchemyStart, startSprIndex, alchemyStartBtn.x, alchemyStartBtn.y);
		//	setFontSize(20);
		//	drawTextCenter(L"#FFFFFF조합하기", alchemyStartBtn.x + 100, alchemyStartBtn.y + 22);
		//	setFontSize(10);
		//	drawText(L"#FFFFFF성공률 : 82%", alchemyStartBtn.x + 45, alchemyStartBtn.y + 38);
		//	drawText(L"#FFFFFF조합시간 : 12분 30초", alchemyStartBtn.x + 45, alchemyStartBtn.y + 51);

		//	if (reactPtrVec.size() == 1)
		//	{
		//		drawFillRect(alchemyStartBtn, col::black, 200);
		//	}

		//	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//	///////////////////////////////////////////셀렉트박스 버튼///////////////////////////////////////////////////////////
		//	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		//	if (dropDownList::mode != selectFlag::none)//셀렉트박스 아이템들 그리기
		//	{
		//		dropDownList::rect.h = 13 + 17 * myMin(10, dropDownList::itemVec.size());
		//		drawFillRect(dropDownList::rect, col::black);

		//		SDL_Rect line1 = { dropDownList::rect.x, dropDownList::rect.y, 1, dropDownList::rect.h };
		//		drawFillRect(line1, { 0x4a,0x4a,0x4a });

		//		SDL_Rect line2 = { dropDownList::rect.x + dropDownList::rect.w - 1, dropDownList::rect.y, 1, dropDownList::rect.h };
		//		drawFillRect(line2, { 0x4a,0x4a,0x4a });

		//		SDL_Rect line3 = { dropDownList::rect.x, dropDownList::rect.y + dropDownList::rect.h - 1, dropDownList::rect.w, 1 };
		//		drawFillRect(line3, { 0x4a,0x4a,0x4a });

		//		//선택 창에 나오는 개별 아이템 그리기
		//		for (int i = 0; i < myMin(10, dropDownList::itemVec.size()); i++)
		//		{
		//			SDL_Rect miniRect = { dropDownList::rect.x, dropDownList::rect.y + 17 * i ,205, 16 };
		//			if (cursorCheck(miniRect) == cursorFlag::hover) drawFillRect(miniRect, { 0x2b,0x81,0xe8 });
		//			else if (cursorCheck(miniRect) == cursorFlag::click) drawFillRect(miniRect, { 0x20,0x50,0xa8 });
		//			else drawFillRect(miniRect, { 0,0,0 });

		//			int targetSprIndex = 0;
		//			if (dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr == nullptr || dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr.get()->itemInfo.size() == 0)
		//			{
		//				targetSprIndex = dropDownList::itemVec[i + dropDownList::scroll]->sprIndex;
		//			}
		//			else targetSprIndex = 124;
		//			drawSpriteCenter(spr::itemset, targetSprIndex, dropDownList::rect.x + 15, dropDownList::rect.y + 9 + 17 * i);


		//			std::wstring indivItemName = L"";
		//			if (dropDownList::itemVec[i + dropDownList::scroll]->number >= 2)
		//			{
		//				indivItemName = std::to_wstring(dropDownList::itemVec[i + dropDownList::scroll]->number);
		//				indivItemName += L" ";
		//			}
		//			indivItemName += dropDownList::itemVec[i + dropDownList::scroll]->name;

		//			int fontSize = 12;
		//			int yCorrection = 0;
		//			setFontSize(12);
		//			while (queryTextWidthColorCode(indivItemName) > 170)
		//			{
		//				if (fontSize > 6)
		//				{
		//					fontSize -= 1;
		//					yCorrection += 1;
		//					setFontSize(fontSize);
		//				}
		//				else break;
		//			}

		//			drawText(col2Str(col::white) + indivItemName, dropDownList::rect.x + 34, dropDownList::rect.y + 1 + yCorrection + 17 * i);
		//		}

		//		for (int i = 0; i < myMin(10, dropDownList::itemVec.size()); i++)
		//		{
		//			SDL_Rect lineSep = { dropDownList::rect.x + 4, dropDownList::rect.y + 16 + 17 * i, 197, 1 };
		//			drawFillRect(lineSep, { 0x4a,0x4a,0x4a });
		//		}

		//		SDL_Rect scrollBox = { dropDownList::rect.x + 205, dropDownList::rect.y + 2, 2, 15 + 17 * (myMin(10,dropDownList::itemVec.size()) - 1) };
		//		drawFillRect(scrollBox, { 0x4a,0x4a,0x4a });
		//	}
		//}
		//else
		//{
		//	SDL_Rect vRect = alchemyBase;
		//	int type = 1;
		//	switch (type)
		//	{
		//	case 0:
		//		vRect.w = vRect.w * getFoldRatio();
		//		vRect.h = vRect.h * getFoldRatio();
		//		break;
		//	case 1:
		//		vRect.x = vRect.x + vRect.w * (1 - getFoldRatio()) / 2;
		//		vRect.w = vRect.w * getFoldRatio();
		//		break;
		//	}
		//	drawWindow(&vRect);


		//}
	}
	void clickUpGUI()
	{
		//if (getStateInput() == false) { return; }

		//if (checkCursor(&tab))
		//{
		//	close(aniFlag::winUnfoldClose);
		//}
		//else if (dropDownList::mode == selectFlag::none) //아이템 선택 리스트가 존재하지 않을 경우
		//{
		//	if (checkCursor(&alchemyStartBtn))
		//	{
		//		if (reactPtrVec.size() > 1)
		//		{
		//			CORO(startAlchemy());
		//		}
		//	}
		//	if (checkCursor(&toolItemBtn))
		//	{
		//		if (toolPtr == nullptr)
		//		{
		//			updateTool();
		//		}
		//		else
		//		{
		//			dropDownList::mode = selectFlag::none;
		//			toolPtr = nullptr;
		//			targetToolQuality = -1;
		//			updateProduct();
		//		}
		//	}
		//	else
		//	{
		//		//반응물 버튼을 눌렀을 경우
		//		for (int i = 0; i < 13; i++)
		//		{
		//			if (checkCursor(&reactRect[i]))
		//			{
		//				if (i < reactPtrVec.size())//정상범위의 반응물(없는 반응물 클릭 방지)
		//				{
		//					if (i == reactPtrVec.size() - 1) // 마지막 버튼을 눌렀을 경우 새로운 반응물 추가
		//					{
		//						dropDownList::itemVec.clear();
		//						updateLootPath();

		//						for (int i = 0; i < rootPathPocket->itemInfo.size(); i++) dropDownList::itemVec.push_back(&rootPathPocket->itemInfo[i]);
		//						if (dropDownList::itemVec.size() != 0)
		//						{
		//							dropDownList::mode = selectFlag::react;
		//							dropDownList::rect = { reactRect[i].x, reactRect[i].y + 18, 210, 17 * myMin(10,dropDownList::itemVec.size()) + 13 };
		//						}
		//					}
		//					else//그 외의 반응물을 눌렀을 경우 
		//					{
		//						reactPtrVec.erase(reactPtrVec.begin() + i);
		//						updateProduct();
		//					}
		//				}
		//			}
		//		}
		//	}
		//}
		//else //아래에 드롭다운리스트가 플로팅된 상태일 때(선택 중)
		//{
		//	if (checkCursor(&dropDownList::rect))
		//	{
		//		for (int i = 0; i < 10; i++)
		//		{
		//			if (i > dropDownList::itemVec.size() - 1) break;

		//			SDL_Rect selectMiniRect = { dropDownList::rect.x , dropDownList::rect.y + 17 * i, 205,16 };
		//			if (checkCursor(&selectMiniRect))
		//			{
		//				if (dropDownList::mode == selectFlag::tool) //도구를 선택했을 경우
		//				{
		//					CORO(selectTool(dropDownList::scroll + i));
		//					dropDownList::mode = selectFlag::none;
		//				}
		//				else if (dropDownList::mode == selectFlag::react)
		//				{
		//					//선택한 새로운 반응물 아이템 추가하기(안에 아이템이 없을 경우)
		//					if (dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr == nullptr || dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr.get()->itemInfo.size() == 0)
		//					{
		//						CORO(executeAddNewReactant(dropDownList::scroll + i));
		//					}
		//					else if (dropDownList::itemVec[dropDownList::scroll + i]->itemCode == 87) //폴더 외부로 이동(뒤로 가기)
		//					{
		//						ItemPocket* prevPtr = dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr.get();
		//						if (prevPtr->itemInfo.size() > 0)
		//						{
		//							dropDownList::itemVec.clear();
		//							currentPathPocket.erase(currentPathPocket.end() - 1);//마지막 원소 삭제
		//							if (prevPtr != rootPathPocket.get()) //만약 최상위 폴더가 아닐 경우 뒤로가기
		//							{
		//								dropDownList::itemVec.push_back(&itemDex[87]);//뒤로 가기
		//								itemDex[87].pocketPtr = currentPathPocket[currentPathPocket.size() - 2];
		//							}

		//							//이전 Ptr에 있었던 요소들 전부 입력
		//							for (int j = 0; j < prevPtr->itemInfo.size(); j++)
		//							{
		//								dropDownList::itemVec.push_back(&prevPtr->itemInfo[j]);

		//								//sort를 사용해 포켓이 있는 아이템을 판별함
		//								std::sort(dropDownList::itemVec.begin() + (currentPathPocket.size() != 1), dropDownList::itemVec.end(), [](ItemData* a, ItemData* b) -> bool {
		//									if (a->pocketPtr != nullptr && b->pocketPtr != nullptr)
		//									{
		//										if (a->pocketPtr.get()->itemInfo.size() > 0)
		//										{
		//											return true;
		//										}
		//										else if (b->pocketPtr.get()->itemInfo.size() > 0)
		//										{
		//											return false;
		//										}
		//										else return false;
		//									}
		//									else return false;
		//									});
		//							}
		//						}
		//					}
		//					else //폴더 내부로 이동
		//					{
		//						ItemPocket* newPtr = dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr.get();
		//						if (newPtr->itemInfo.size() > 0)
		//						{
		//							dropDownList::itemVec.clear();
		//							dropDownList::itemVec.push_back(&itemDex[87]);//뒤로 가기
		//							itemDex[87].pocketPtr = currentPathPocket[currentPathPocket.size() - 1];
		//							currentPathPocket.push_back(newPtr);

		//							//내부 Ptr에 있었던 요소들 전부 입력
		//							for (int j = 0; j < newPtr->itemInfo.size(); j++)
		//							{
		//								newPtr->itemInfo[j].lootSelect = 0;
		//								dropDownList::itemVec.push_back(&newPtr->itemInfo[j]);

		//								//sort를 사용해 포켓이 있는 아이템을 판별함
		//								std::sort(dropDownList::itemVec.begin() + (currentPathPocket.size() != 1), dropDownList::itemVec.end(), [](ItemData* a, ItemData* b) -> bool {
		//									if (a->pocketPtr != nullptr && b->pocketPtr != nullptr)
		//									{
		//										if (a->pocketPtr.get()->itemInfo.size() > 0)
		//										{
		//											return true;
		//										}
		//										else if (b->pocketPtr.get()->itemInfo.size() > 0)
		//										{
		//											return false;
		//										}
		//										else return false;
		//									}
		//									else return false;
		//									});
		//							}
		//						}
		//					}
		//				}
		//				else errorBox("Click item select box while dropDownList::mode Var is none");
		//			}
		//		}
		//	}
		//	else
		//	{
		//		dropDownList::mode = selectFlag::none;
		//	}
		//}


	}
	void clickMotionGUI(int dx, int dy)
	{
		if (checkCursor(&dropDownList::rect))
		{
			if (click == true)
			{
				int scrollAccelConst = 20; // 가속상수, 작아질수록 스크롤 속도가 빨라짐
				dropDownList::scroll = dropDownList::initScroll + dy / scrollAccelConst;
				if (abs(dy / scrollAccelConst) >= 1)
				{
					deactClickUp = true;
					itemListColorLock = true;
				}
			}
		}
	}
	void clickDownGUI()
	{
		dropDownList::initScroll = dropDownList::scroll;
	}
	void clickRightGUI() { }
	void clickHoldGUI() { }
	void mouseWheel() {}
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step()
	{
		//잘못된 스크롤 위치 조정
		if (dropDownList::scroll + 10 >= dropDownList::itemVec.size()) { dropDownList::scroll = myMax(0, (int)dropDownList::itemVec.size() - 10); }
		else if (dropDownList::scroll < 0) { dropDownList::scroll = 0; }
	}

	void updateLootPath()
	{
		//rootPathPocket->itemInfo.clear();

		////생성될 때 주변 타일에 아이템이 있는지 체크
		//if (PlayerPtr->getEquipPtr()->itemInfo.size() > 0)
		//{
		//	rootPathPocket->itemInfo.push_back(itemDex[86]);
		//	rootPathPocket->itemInfo[0].name = L"장비";
		//	rootPathPocket->itemInfo[0].pocketPtr = PlayerPtr->getEquipPtr();
		//}

		//for (int i = 0; i < 9; i++)
		//{
		//	int dx, dy;
		//	dir2Coord(i, dx, dy);
		//	if (TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ()) != nullptr)
		//	{
		//		rootPathPocket->itemInfo.push_back(itemDex[86]);
		//		switch (i)
		//		{
		//		default:
		//			rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"플레이어 타일";
		//			break;
		//		case 0:
		//			rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"타일(→)";
		//			break;
		//		case 1:
		//			rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"타일(↗)";
		//			break;
		//		case 2:
		//			rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"타일(↑)";
		//			break;
		//		case 3:
		//			rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"타일(↖)";
		//			break;
		//		case 4:
		//			rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"타일(←)";
		//			break;
		//		case 5:
		//			rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"타일(↙)";
		//			break;
		//		case 6:
		//			rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"타일(↓)";
		//			break;
		//		case 7:
		//			rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"타일(↘)";
		//			break;
		//		}
		//		ItemStack* targetStack = TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ());
		//		rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].pocketPtr = targetStack->getPocket();
		//	}
		//}
	}

	void updateTool()
	{
		////플레이어의 인벤토리를 뒤져서 찾음
		//ItemPocket* equipPtr = PlayerPtr->getEquipPtr();
		//dropDownList::itemVec.clear();
		//for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		//{
		//	if (equipPtr->itemInfo[i].checkFlag(itemFlag::ALCHEMYTOOL))
		//	{
		//		dropDownList::itemVec.push_back(&equipPtr->itemInfo[i]);
		//	}
		//}

		////주변 타일을 뒤져서 찾음
		//for (int i = 0; i < 9; i++)
		//{
		//	int dx, dy;
		//	dir2Coord(i, dx, dy);
		//	if (TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ()) != nullptr)
		//	{
		//		ItemStack* targetStack = TileItemStack(PlayerX() + dx, PlayerY() + dy, PlayerZ());
		//		ItemPocket* targetPocket = targetStack->getPocket();
		//		for (int j = 0; j < targetPocket->itemInfo.size(); j++)
		//		{
		//			if (targetPocket->itemInfo[j].checkFlag(itemFlag::ALCHEMYTOOL))
		//			{
		//				dropDownList::itemVec.push_back(&targetPocket->itemInfo[j]);
		//			}
		//		}
		//	}
		//}

		if (dropDownList::itemVec.size() != 0) //공구를 1개라도 찾는데 성공했으면
		{
			dropDownList::mode = selectFlag::tool;
			dropDownList::rect = { toolItemBtn.x, toolItemBtn.y + 18, 210, 17 * myMin(10,dropDownList::itemVec.size()) + 13 };
		}
	}

	Corouter executeAddNewReactant(int cursor)
	{
		if (dropDownList::itemVec[cursor]->number == 1) //아이템의 갯수가 1개일 경우
		{
			dropDownList::itemVec[cursor]->reactSelect = 1;
		}
		else
		{
			std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
			new Msg(msgFlag::input, sysStr[40], sysStr[39], choiceVec);//아이템 선택, 얼마나?
			co_await std::suspend_always();

			if (coAnswer == sysStr[38])//확인
			{
				dropDownList::itemVec[cursor]->reactSelect = myMax(1, myMin(dropDownList::itemVec[cursor]->number, wtoi(exInputText.c_str())));
			}
			else//취소 and 탭
			{
				co_return;
			}
		}

		if (std::find(reactPtrVec.begin(), reactPtrVec.end(), dropDownList::itemVec[cursor]) != reactPtrVec.end())//기존 반응물 목록에 없을 경우
		{
			reactPtrVec.erase(std::find(reactPtrVec.begin(), reactPtrVec.end(), dropDownList::itemVec[cursor]));
		}

		reactPtrVec.insert(reactPtrVec.end() - 1, dropDownList::itemVec[cursor]);
		dropDownList::mode = selectFlag::none;
		updateProduct();
	};

	void updateProduct()
	{
		for (int alchemyCounter = 0; alchemyCounter < alchemyDex.size(); alchemyCounter++)
		{
			if (alchemyDex[alchemyCounter].qualityNeed != targetToolQuality)
			{
				prt(L"인덱스 %d 연금술 조합 공구기술 부족으로 취소됨\n", alchemyCounter);
				continue;
			}

			for (int recipeCounter = 0; recipeCounter < alchemyDex[alchemyCounter].reactant.size(); recipeCounter++)
			{
				//find를 위한 itemCode vector와 number vector 제조
				std::vector<int> itemCodeVec, numberVec;
				for (int reactCounter = 0; reactCounter < reactPtrVec.size(); reactCounter++)
				{
					if (reactPtrVec[reactCounter] != nullptr)
					{
						itemCodeVec.push_back(reactPtrVec[reactCounter]->itemCode);
						numberVec.push_back(reactPtrVec[reactCounter]->number);
					}
				}

				auto codeIt = std::find(itemCodeVec.begin(), itemCodeVec.end(), alchemyDex[alchemyCounter].reactant[recipeCounter].first);
				if (codeIt != itemCodeVec.end()) //만약 필요 아이템이 현재 반응물에 존재할 경우
				{
					
					int index = std::distance(itemCodeVec.begin(), codeIt); //반복자를 인덱스로 변환
					prt(L"필요한 아이템 %d이 현재 반응물에 존재함\n", itemCodeVec[index]);
					if (numberVec[index] < alchemyDex[alchemyCounter].reactant[recipeCounter].second)
					{
						prt(L"아이템 %d 숫자 부족(현재 %d)으로 break;\n", itemCodeVec[index],numberVec[index]);
						break;//숫자가 충분한지 확인
					}
				}
				else
				{
					prt(L"아이템 %d가 반응물에 없어서 break\n", alchemyDex[alchemyCounter].reactant[recipeCounter].first);
					break;
				}

				if (recipeCounter == alchemyDex[alchemyCounter].reactant.size() - 1)//마지막에 문제없이 도달했을 경우
				{
					prt(L"연금술 마지막 도달\n");

					int reactPtrVecSize = 0;
					for (int i = 0; i < reactPtrVec.size(); i++)
					{
						if (reactPtrVec[i] != nullptr) reactPtrVecSize++;
					}

					if (itemCodeVec.size() == reactPtrVecSize)
					{
						prt(L"연금술 성공\n");
						//std::wprintf(L"성공\n");
						targetProductIndex = alchemyCounter;
						return;
					}
					else break;
				}
			}
		}
		targetProductIndex = -1;
	}

	Corouter selectTool(int cursor)
	{
		
		if (dropDownList::itemVec[cursor]->toolQuality.size() > 1)
		{
			std::vector<std::wstring> toolQualityList;
			for (int i = 0; i < toolPtr->toolQuality.size(); i++)
			{
				toolQualityList.push_back(toolQuality2String(dropDownList::itemVec[cursor]->toolQuality[i]));
			}
			new Lst(sysStr[95], sysStr[94], toolQualityList);//넣기, 넣을 포켓을 선택해주세요.
			co_await std::suspend_always();
			if (wtoi(coAnswer) != -1)
			{
				toolPtr = dropDownList::itemVec[cursor];
				targetToolQuality = dropDownList::itemVec[cursor]->toolQuality[wtoi(coAnswer)];
			}
		}
		else
		{
			toolPtr = dropDownList::itemVec[cursor];
			targetToolQuality = dropDownList::itemVec[cursor]->toolQuality[0];
		}
		updateProduct();
	}

	Corouter startAlchemy()
	{
		bool negateMonster = false;
		showAlchemyTooltip = true;
		deactDraw();
		if (targetProductIndex == -1) targetCraftingTime = randomRange(10,100);
		else targetCraftingTime = alchemyDex[targetProductIndex].time;
		
		while (1)
		{
			prt(L"Alchemy While 루프 실행됨\n");

			if (negateMonster == false)
			{
				prt(L"적을 무시할까?\n");
				for (int x = PlayerX() - 1; x <= PlayerX() + 1; x++)
				{
					for (int y = PlayerY() - 1; y <= PlayerY() + 1; y++)
					{
						if (!(x == PlayerX() && y == PlayerY()))
							if (TileFov(x, y, PlayerZ()) == fovFlag::white)
								if (TileEntity(x, y, PlayerZ()) != nullptr)
								{
									new Msg(msgFlag::normal, L"경고", L"주변에 적이 있습니다. 계속 조합하시겠습니까?", { L"네",L"아니오",L"무시하기" });
									co_await std::suspend_always();
									if (coAnswer == L"네") goto loopEnd;
									else if (coAnswer == L"무시하기")
									{
										negateMonster = true;
										goto loopEnd;
									}
									else
									{
										coTurnSkip = false;
										close(aniFlag::null);
										co_return;
									}
								}
					}
				}
			}

		loopEnd:

			//turnWait(1.0);
			coTurnSkip = true;

			prt(L"exeCraft 코루틴 실행 전\n");

			co_await std::suspend_always();

			prt(L"exeCraft 코루틴 실행 후\n");

			elapsedTime++;
			if (elapsedTime >= targetCraftingTime)
			{
				actDraw();
				showAlchemyTooltip = false;
				elapsedTime = 0;
				break;
			}
		}

		if (targetProductIndex == -1)
		{
			for (int i = 0; i < alchemyDex[0].product.size(); i++)
			{
				prodPocket->addItemFromDex(alchemyDex[0].product[i].first, alchemyDex[0].product[i].second);
			}
		}
		else
		{
			for (int i = 0; i < alchemyDex[targetProductIndex].product.size(); i++)
			{
				prodPocket->addItemFromDex(alchemyDex[targetProductIndex].product[i].first, alchemyDex[targetProductIndex].product[i].second);
			}
		}
	};
};


