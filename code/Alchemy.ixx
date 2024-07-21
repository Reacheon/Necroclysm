#include <SDL.h>

#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();

export module Alchemy;

import std;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
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

const int reactantMaxSize = 13; //���� �������� �ִ� 13�������� �Է� ����, ������� 13����

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

	ItemPocket* rootPathPocket;
	std::vector<ItemPocket*> currentPathPocket; //���� ������ ��ġ�� �˷��ִ� ���� C/FOLDER1/FOLDER2, �������� ���� ����

	int targetProductIndex = -1;
	int targetToolQuality = -1;

	ItemPocket* prodPocket;

	bool showAlchemyTooltip = false;
	int elapsedTime = 0; //�Ѱ� ���� �ð�
	int targetCraftingTime = 10;
public:
	Alchemy() : GUI(false)
	{
		prt(L"Alchemy : �����ڰ� ȣ��Ǿ���.\n");
		//1�� �̻��� �޽��� ��ü ���� ���� ���� ó��
		errorBox(ptr != nullptr, "More than one message instance was generated.");
		ptr = this;

		//�޼��� �ڽ� ������
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this,aniFlag::winUnfoldOpen);
		
		rootPathPocket = new ItemPocket(storageType::null);
		prodPocket = new ItemPocket(storageType::null);
		currentPathPocket.push_back(rootPathPocket);
	}
	~Alchemy()
	{
		prt(L"Alchemy : �Ҹ��ڰ� ȣ��Ǿ���.\n");
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
		delete rootPathPocket;
		delete prodPocket;
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
		if (showAlchemyTooltip)
		{
			SDL_Rect tooltipBox = { cameraW / 2 - 140, 0, 280,130 };
			drawWindow(&tooltipBox);
			setZoom(1.0);
			int markerIndex = 0;
			if (Msg::ins() == nullptr)
			{
				if (timer::timer600 % 25 < 2) { markerIndex = 0; }
				else if (timer::timer600 % 25 < 4) { markerIndex = 1; }
				else if (timer::timer600 % 25 < 6) { markerIndex = 2; }
				else if (timer::timer600 % 25 < 8) { markerIndex = 3; }
				else if (timer::timer600 % 25 < 10) { markerIndex = 4; }
				else if (timer::timer600 % 25 < 12) { markerIndex = 5; }
				else if (timer::timer600 % 25 < 14) { markerIndex = 6; }
				else if (timer::timer600 % 25 < 16) { markerIndex = 7; }
				else if (timer::timer600 % 25 < 18) { markerIndex = 8; }
				else if (timer::timer600 % 25 < 20) { markerIndex = 9; }
				else if (timer::timer600 % 25 < 22) { markerIndex = 10; }
				else if (timer::timer600 % 25 < 24) { markerIndex = 11; }
				else { markerIndex = 0; }
			}
			drawSprite(spr::loadingAnime, markerIndex, tooltipBox.x + tooltipBox.w / 2 - 78, tooltipBox.y + 6);
			setFontSize(13);
			drawText(col2Str(col::white) + L"������ ���� ��...", tooltipBox.x + tooltipBox.w / 2 - 40, tooltipBox.y + 14);

			std::wstring reactStr = col2Str(lowCol::yellow) + L"������ : ";
			reactStr += col2Str(col::white);
			for (int i = 0; i < reactPtrVec.size(); i++)
			{
				if (reactPtrVec[i] != nullptr)
				{
					reactStr += reactPtrVec[i]->name;

					if (i != reactPtrVec.size() - 1)
					{
						if (reactPtrVec[i + 1] != nullptr)
						{
							reactStr += L", ";
						}
					}
				}
			}
			std::wstring toolStr = col2Str(lowCol::yellow) + L"���� ��� : ";
			toolStr += col2Str(col::white);
			if (targetToolQuality == -1) {toolStr += L"����";}
			else
			{
				toolStr += toolQuality2String(targetToolQuality);
			}
			drawText(reactStr, tooltipBox.x + tooltipBox.w / 2 - 120, tooltipBox.y + 40);
			drawText(toolStr, tooltipBox.x + tooltipBox.w / 2 - 120, tooltipBox.y + 58);

			SDL_Rect tooltipGauge = { cameraW / 2 - 130, 100, 260,16 };
			drawRect(tooltipGauge, col::white);

			SDL_Rect tooltipInGauge = { cameraW / 2 - 130 + 4, 100 + 4, 252,8 };
			tooltipInGauge.w = 252.0 * ((float)elapsedTime / (float)targetCraftingTime);
			drawFillRect(tooltipInGauge, col::white);

			setFontSize(11);
			std::wstring topText = std::to_wstring(targetCraftingTime - elapsedTime);
			topText += L" �� ���� ( ";
			topText += std::to_wstring((int)(((float)elapsedTime * 100.0 / (float)targetCraftingTime)));
			topText += L"% )";

			drawTextCenter(col2Str(col::white) + topText, tooltipGauge.x + tooltipGauge.w / 2, tooltipGauge.y - 10);
		}

		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			drawWindow(&alchemyBase, sysStr[134], 8);

			auto cursorCheck = [](SDL_Rect checkRect)->cursorFlag {
				if (checkCursor(&checkRect))
				{
					//�ٸ� �ڷ�ƾ ��ü�� �����ϴ��� Ȯ��
					if (Msg::ins() == nullptr && Lst::ins() == nullptr)
					{
						if (click == true) { return cursorFlag::click; }
						else { return cursorFlag::hover; }
					}
					else return cursorFlag::none;
				}
				else return cursorFlag::none;
				};



			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////������///////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Point2 reactPivot = { alchemyBase.x + 10, alchemyBase.y + 59 };
			setFontSize(11);
			drawTextCenter(col2Str(col::white) + L"������", reactPivot.x + 109, reactPivot.y - 9);
			drawSprite(spr::alchemyMaterialEdge, 0, reactPivot.x - 1, reactPivot.y - 1);
			for (int i = 0; i < myMin(reactPtrVec.size(), reactantMaxSize); i++)
			{
				SDL_Rect targetRect = { reactRect[i].x, reactRect[i].y,210,18 };
				if (reactPtrVec[i] == nullptr) drawSimpleItemRectAdd(cursorCheck(targetRect), targetRect.x, targetRect.y);
				else
				{
					std::wstring indivItemName = L"";
					if (reactPtrVec[i]->reactSelect >= 2)
					{
						indivItemName = std::to_wstring(reactPtrVec[i]->reactSelect);
						indivItemName += L" ";
					}
					indivItemName += reactPtrVec[i]->name;

					drawSimpleItemRect(cursorCheck(targetRect), targetRect.x, targetRect.y, reactPtrVec[i]->sprIndex, col2Str(col::white) + indivItemName, false);
				}
			}

			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////������///////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			Point2 prodPivot = { alchemyBase.x + 476, alchemyBase.y + 59 };
			setFontSize(11);
			drawTextCenter(col2Str(col::white) + L"������", prodPivot.x + 109, prodPivot.y - 9);
			drawSprite(spr::alchemyMaterialEdge, 0, prodPivot.x - 1, prodPivot.y - 1);
			
			for (int i = 0; i < myMin(prodPocket->itemInfo.size(), reactantMaxSize); i++)
			{
				SDL_Rect targetRect = { prodRect[i].x, prodRect[i].y,210,18 };
				std::wstring indivItemName;

				if (prodPocket->itemInfo[i].number > 1)
				{
					indivItemName = std::to_wstring(prodPocket->itemInfo[i].number);
					indivItemName += L" ";
				}
				indivItemName += prodPocket->itemInfo[i].name;
				drawSimpleItemRect(cursorCheck(targetRect), targetRect.x, targetRect.y, prodPocket->itemInfo[i].sprIndex, col2Str(col::white) + indivItemName, false);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////����//////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			SDL_Point toolPivot = { alchemyBase.x + 248, alchemyBase.y + 89 };
			setFontSize(11);
			drawTextCenter(col2Str(col::white) + L"����", alchemyBase.x + 350, alchemyBase.y + 182);




			drawSprite(spr::alchemyArrow, 0, alchemyBase.x + 242, alchemyBase.y + 181);

			//���ο� ��ǰ �߰� ��ư
			if (toolPtr == nullptr)
			{
				drawSimpleItemRectAdd(cursorCheck(toolItemBtn), toolItemBtn.x, toolItemBtn.y);
			}
			else
			{
				std::wstring indivItemName;
				indivItemName += L"#CEC327";
				indivItemName += toolQuality2String(targetToolQuality);
				indivItemName += L"#FFFFFF";
				indivItemName += toolPtr->name;
				drawSimpleItemRect(cursorCheck(toolItemBtn), toolItemBtn.x, toolItemBtn.y, toolPtr->sprIndex, L"#CEC327���� #FFFFFF������", false);
			}

			//355,106
			SDL_Rect toolFigure = { alchemyBase.x + 355 - 48,alchemyBase.y + 106 - 48, 96,96 };
			drawWindow(&toolFigure);

			setZoom(2.0);
			int toolSprIndex = 0;
			switch (targetToolQuality)
			{
			case toolQuality::boiling:
				toolSprIndex = 54;
				break;
			case toolQuality::distillation:
				toolSprIndex = 53;
				break;
			case toolQuality::frying:
				toolSprIndex = 49;
				break;
			case toolQuality::heating:
				toolSprIndex = 51;
				break;
			default:
				toolSprIndex = 0;
				break;
			}
			drawSpriteCenter(spr::icon48, toolSprIndex, alchemyBase.x + 355, alchemyBase.y + 106);
			setZoom(1.0);

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			/////////////////////////////////////////////�����ϱ� ��ư///////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			int startSprIndex = 0;
			if (reactPtrVec.size() > 1)
			{
				switch (cursorCheck(alchemyStartBtn))
				{
				default:
					startSprIndex = 0;
					break;
				case cursorFlag::hover:
					startSprIndex = 1;
					break;
				case cursorFlag::click:
					startSprIndex = 2;
					break;
				}
			}

			drawSprite(spr::alchemyStart, startSprIndex, alchemyStartBtn.x, alchemyStartBtn.y);
			setFontSize(20);
			drawTextCenter(L"#FFFFFF�����ϱ�", alchemyStartBtn.x + 100, alchemyStartBtn.y + 22);
			setFontSize(10);
			drawText(L"#FFFFFF������ : 82%", alchemyStartBtn.x + 45, alchemyStartBtn.y + 38);
			drawText(L"#FFFFFF���սð� : 12�� 30��", alchemyStartBtn.x + 45, alchemyStartBtn.y + 51);

			if (reactPtrVec.size() == 1)
			{
				drawFillRect(alchemyStartBtn, col::black, 200);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///////////////////////////////////////////����Ʈ�ڽ� ��ư///////////////////////////////////////////////////////////
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			if (dropDownList::mode != selectFlag::none)//����Ʈ�ڽ� �����۵� �׸���
			{
				dropDownList::rect.h = 13 + 17 * myMin(10, dropDownList::itemVec.size());
				drawFillRect(dropDownList::rect, col::black);

				SDL_Rect line1 = { dropDownList::rect.x, dropDownList::rect.y, 1, dropDownList::rect.h };
				drawFillRect(line1, { 0x4a,0x4a,0x4a });

				SDL_Rect line2 = { dropDownList::rect.x + dropDownList::rect.w - 1, dropDownList::rect.y, 1, dropDownList::rect.h };
				drawFillRect(line2, { 0x4a,0x4a,0x4a });

				SDL_Rect line3 = { dropDownList::rect.x, dropDownList::rect.y + dropDownList::rect.h - 1, dropDownList::rect.w, 1 };
				drawFillRect(line3, { 0x4a,0x4a,0x4a });

				//���� â�� ������ ���� ������ �׸���
				for (int i = 0; i < myMin(10, dropDownList::itemVec.size()); i++)
				{
					SDL_Rect miniRect = { dropDownList::rect.x, dropDownList::rect.y + 17 * i ,205, 16 };
					if (cursorCheck(miniRect) == cursorFlag::hover) drawFillRect(miniRect, { 0x2b,0x81,0xe8 });
					else if (cursorCheck(miniRect) == cursorFlag::click) drawFillRect(miniRect, { 0x20,0x50,0xa8 });
					else drawFillRect(miniRect, { 0,0,0 });

					int targetSprIndex = 0;
					if (dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr == nullptr || ((ItemPocket*)(dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr))->itemInfo.size() == 0)
					{
						targetSprIndex = dropDownList::itemVec[i + dropDownList::scroll]->sprIndex;
					}
					else targetSprIndex = 124;
					drawSpriteCenter(spr::itemset, targetSprIndex, dropDownList::rect.x + 15, dropDownList::rect.y + 9 + 17 * i);


					std::wstring indivItemName = L"";
					if (dropDownList::itemVec[i + dropDownList::scroll]->number >= 2)
					{
						indivItemName = std::to_wstring(dropDownList::itemVec[i + dropDownList::scroll]->number);
						indivItemName += L" ";
					}
					indivItemName += dropDownList::itemVec[i + dropDownList::scroll]->name;

					int fontSize = 12;
					int yCorrection = 0;
					setFontSize(12);
					while (queryTextWidthColorCode(indivItemName) > 170)
					{
						if (fontSize > 6)
						{
							fontSize -= 1;
							yCorrection += 1;
							setFontSize(fontSize);
						}
						else break;
					}

					drawText(col2Str(col::white) + indivItemName, dropDownList::rect.x + 34, dropDownList::rect.y + 1 + yCorrection + 17 * i);
				}

				for (int i = 0; i < myMin(10, dropDownList::itemVec.size()); i++)
				{
					SDL_Rect lineSep = { dropDownList::rect.x + 4, dropDownList::rect.y + 16 + 17 * i, 197, 1 };
					drawFillRect(lineSep, { 0x4a,0x4a,0x4a });
				}

				SDL_Rect scrollBox = { dropDownList::rect.x + 205, dropDownList::rect.y + 2, 2, 15 + 17 * (myMin(10,dropDownList::itemVec.size()) - 1) };
				drawFillRect(scrollBox, { 0x4a,0x4a,0x4a });
			}
		}
		else
		{
			SDL_Rect vRect = alchemyBase;
			int type = 1;
			switch (type)
			{
			case 0:
				vRect.w = vRect.w * getFoldRatio();
				vRect.h = vRect.h * getFoldRatio();
				break;
			case 1:
				vRect.x = vRect.x + vRect.w * (1 - getFoldRatio()) / 2;
				vRect.w = vRect.w * getFoldRatio();
				break;
			}
			drawWindow(&vRect);


		}
	}
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
		}
		else if (dropDownList::mode == selectFlag::none) //������ ���� ����Ʈ�� �������� ���� ���
		{
			if (checkCursor(&alchemyStartBtn))
			{
				if (reactPtrVec.size() > 1)
				{
					CORO(startAlchemy());
				}
			}
			if (checkCursor(&toolItemBtn))
			{
				if (toolPtr == nullptr)
				{
					updateTool();
				}
				else
				{
					dropDownList::mode = selectFlag::none;
					toolPtr = nullptr;
					targetToolQuality = -1;
					updateProduct();
				}
			}
			else
			{
				//������ ��ư�� ������ ���
				for (int i = 0; i < 13; i++)
				{
					if (checkCursor(&reactRect[i]))
					{
						if (i < reactPtrVec.size())//��������� ������(���� ������ Ŭ�� ����)
						{
							if (i == reactPtrVec.size() - 1) // ������ ��ư�� ������ ��� ���ο� ������ �߰�
							{
								dropDownList::itemVec.clear();
								updateLootPath();

								for (int i = 0; i < rootPathPocket->itemInfo.size(); i++) dropDownList::itemVec.push_back(&rootPathPocket->itemInfo[i]);
								if (dropDownList::itemVec.size() != 0)
								{
									dropDownList::mode = selectFlag::react;
									dropDownList::rect = { reactRect[i].x, reactRect[i].y + 18, 210, 17 * myMin(10,dropDownList::itemVec.size()) + 13 };
								}
							}
							else//�� ���� �������� ������ ��� 
							{
								reactPtrVec.erase(reactPtrVec.begin() + i);
								updateProduct();
							}
						}
					}
				}
			}
		}
		else //�Ʒ��� ��Ӵٿ��Ʈ�� �÷��õ� ������ ��(���� ��)
		{
			if (checkCursor(&dropDownList::rect))
			{
				for (int i = 0; i < 10; i++)
				{
					if (i > dropDownList::itemVec.size() - 1) break;

					SDL_Rect selectMiniRect = { dropDownList::rect.x , dropDownList::rect.y + 17 * i, 205,16 };
					if (checkCursor(&selectMiniRect))
					{
						if (dropDownList::mode == selectFlag::tool) //������ �������� ���
						{
							CORO(selectTool(dropDownList::scroll + i));
							dropDownList::mode = selectFlag::none;
						}
						else if (dropDownList::mode == selectFlag::react)
						{
							//������ ���ο� ������ ������ �߰��ϱ�(�ȿ� �������� ���� ���)
							if (dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr == nullptr || ((ItemPocket*)(dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr))->itemInfo.size() == 0)
							{
								CORO(executeAddNewReactant(dropDownList::scroll + i));
							}
							else if (dropDownList::itemVec[dropDownList::scroll + i]->itemCode == 87) //���� �ܺη� �̵�(�ڷ� ����)
							{
								ItemPocket* prevPtr = (ItemPocket*)dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr;
								if (prevPtr->itemInfo.size() > 0)
								{
									dropDownList::itemVec.clear();
									currentPathPocket.erase(currentPathPocket.end() - 1);//������ ���� ����
									if (prevPtr != rootPathPocket) //���� �ֻ��� ������ �ƴ� ��� �ڷΰ���
									{
										dropDownList::itemVec.push_back(&itemDex[87]);//�ڷ� ����
										itemDex[87].pocketPtr = currentPathPocket[currentPathPocket.size() - 2];
									}

									//���� Ptr�� �־��� ��ҵ� ���� �Է�
									for (int j = 0; j < prevPtr->itemInfo.size(); j++)
									{
										dropDownList::itemVec.push_back(&prevPtr->itemInfo[j]);

										//sort�� ����� ������ �ִ� �������� �Ǻ���
										std::sort(dropDownList::itemVec.begin() + (currentPathPocket.size() != 1), dropDownList::itemVec.end(), [](ItemData* a, ItemData* b) -> bool {
											if (a->pocketPtr != nullptr && b->pocketPtr != nullptr)
											{
												if (((ItemPocket*)a->pocketPtr)->itemInfo.size() > 0)
												{
													return true;
												}
												else if (((ItemPocket*)b->pocketPtr)->itemInfo.size() > 0)
												{
													return false;
												}
												else return false;
											}
											else return false;
											});
									}
								}
							}
							else //���� ���η� �̵�
							{
								ItemPocket* newPtr = (ItemPocket*)dropDownList::itemVec[dropDownList::scroll + i]->pocketPtr;
								if (newPtr->itemInfo.size() > 0)
								{
									dropDownList::itemVec.clear();
									dropDownList::itemVec.push_back(&itemDex[87]);//�ڷ� ����
									itemDex[87].pocketPtr = currentPathPocket[currentPathPocket.size() - 1];
									currentPathPocket.push_back(newPtr);

									//���� Ptr�� �־��� ��ҵ� ���� �Է�
									for (int j = 0; j < newPtr->itemInfo.size(); j++)
									{
										newPtr->itemInfo[j].lootSelect = 0;
										dropDownList::itemVec.push_back(&newPtr->itemInfo[j]);

										//sort�� ����� ������ �ִ� �������� �Ǻ���
										std::sort(dropDownList::itemVec.begin() + (currentPathPocket.size() != 1), dropDownList::itemVec.end(), [](ItemData* a, ItemData* b) -> bool {
											if (a->pocketPtr != nullptr && b->pocketPtr != nullptr)
											{
												if (((ItemPocket*)a->pocketPtr)->itemInfo.size() > 0)
												{
													return true;
												}
												else if (((ItemPocket*)b->pocketPtr)->itemInfo.size() > 0)
												{
													return false;
												}
												else return false;
											}
											else return false;
											});
									}
								}
							}
						}
						else errorBox("Click item select box while dropDownList::mode Var is none");
					}
				}
			}
			else
			{
				dropDownList::mode = selectFlag::none;
			}
		}


	}
	void clickMotionGUI(int dx, int dy)
	{
		if (checkCursor(&dropDownList::rect))
		{
			if (click == true)
			{
				int scrollAccelConst = 20; // ���ӻ��, �۾������� ��ũ�� �ӵ��� ������
				dropDownList::scroll = dropDownList::initScroll + dy / scrollAccelConst;
				if (abs(dy / scrollAccelConst) >= 1)
				{
					deactClickUp = true;
					cursorMotionLock = true;
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
	void gamepadBtnDown() { }
	void gamepadBtnMotion() { }
	void gamepadBtnUp() { }
	void step()
	{
		//�߸��� ��ũ�� ��ġ ����
		if (dropDownList::scroll + 10 >= dropDownList::itemVec.size()) { dropDownList::scroll = myMax(0, (int)dropDownList::itemVec.size() - 10); }
		else if (dropDownList::scroll < 0) { dropDownList::scroll = 0; }
	}

	void updateLootPath()
	{
		rootPathPocket->itemInfo.clear();

		//������ �� �ֺ� Ÿ�Ͽ� �������� �ִ��� üũ
		if (Player::ins()->getEquipPtr()->itemInfo.size() > 0)
		{
			rootPathPocket->itemInfo.push_back(itemDex[86]);
			rootPathPocket->itemInfo[0].name = L"���";
			rootPathPocket->itemInfo[0].pocketPtr = Player::ins()->getEquipPtr();
		}

		for (int i = 0; i < 9; i++)
		{
			int dx, dy;
			dir2Coord(i, dx, dy);
			if (World::ins()->getTile(Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy, Player::ins()->getGridZ()).ItemStackPtr != nullptr)
			{
				rootPathPocket->itemInfo.push_back(itemDex[86]);
				switch (i)
				{
				default:
					rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"�÷��̾� Ÿ��";
					break;
				case 0:
					rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"Ÿ��(��)";
					break;
				case 1:
					rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"Ÿ��(��)";
					break;
				case 2:
					rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"Ÿ��(��)";
					break;
				case 3:
					rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"Ÿ��(��)";
					break;
				case 4:
					rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"Ÿ��(��)";
					break;
				case 5:
					rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"Ÿ��(��)";
					break;
				case 6:
					rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"Ÿ��(��)";
					break;
				case 7:
					rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].name = L"Ÿ��(��)";
					break;
				}
				ItemStack* targetStack = (ItemStack*)World::ins()->getTile(Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy, Player::ins()->getGridZ()).ItemStackPtr;
				rootPathPocket->itemInfo[rootPathPocket->itemInfo.size() - 1].pocketPtr = targetStack->getPocket();
			}
		}
	}

	void updateTool()
	{
		//�÷��̾��� �κ��丮�� ������ ã��
		ItemPocket* equipPtr = Player::ins()->getEquipPtr();
		dropDownList::itemVec.clear();
		for (int i = 0; i < equipPtr->itemInfo.size(); i++)
		{
			if (equipPtr->itemInfo[i].checkFlag(itemFlag::ALCHEMYTOOL))
			{
				dropDownList::itemVec.push_back(&equipPtr->itemInfo[i]);
			}
		}

		//�ֺ� Ÿ���� ������ ã��
		for (int i = 0; i < 9; i++)
		{
			int dx, dy;
			dir2Coord(i, dx, dy);
			if (World::ins()->getTile(Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy, Player::ins()->getGridZ()).ItemStackPtr != nullptr)
			{
				ItemStack* targetStack = (ItemStack*)World::ins()->getTile(Player::ins()->getGridX() + dx, Player::ins()->getGridY() + dy, Player::ins()->getGridZ()).ItemStackPtr;
				ItemPocket* targetPocket = targetStack->getPocket();
				for (int j = 0; j < targetPocket->itemInfo.size(); j++)
				{
					if (targetPocket->itemInfo[j].checkFlag(itemFlag::ALCHEMYTOOL))
					{
						dropDownList::itemVec.push_back(&targetPocket->itemInfo[j]);
					}
				}
			}
		}

		if (dropDownList::itemVec.size() != 0) //������ 1���� ã�µ� ����������
		{
			dropDownList::mode = selectFlag::tool;
			dropDownList::rect = { toolItemBtn.x, toolItemBtn.y + 18, 210, 17 * myMin(10,dropDownList::itemVec.size()) + 13 };
		}
	}

	Corouter executeAddNewReactant(int cursor)
	{
		if (dropDownList::itemVec[cursor]->number == 1) //�������� ������ 1���� ���
		{
			dropDownList::itemVec[cursor]->reactSelect = 1;
		}
		else
		{
			std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//Ȯ��, ���
			new Msg(msgFlag::input, sysStr[40], sysStr[39], choiceVec);//������ ����, �󸶳�?
			co_await std::suspend_always();

			if (coAnswer == sysStr[38])//Ȯ��
			{
				dropDownList::itemVec[cursor]->reactSelect = myMax(1, myMin(dropDownList::itemVec[cursor]->number, wtoi(exInputText.c_str())));
			}
			else//��� and ��
			{
				co_return;
			}
		}

		if (std::find(reactPtrVec.begin(), reactPtrVec.end(), dropDownList::itemVec[cursor]) != reactPtrVec.end())//���� ������ ��Ͽ� ���� ���
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
				prt(L"�ε��� %d ���ݼ� ���� ������� �������� ��ҵ�\n", alchemyCounter);
				continue;
			}

			for (int recipeCounter = 0; recipeCounter < alchemyDex[alchemyCounter].reactant.size(); recipeCounter++)
			{
				//find�� ���� itemCode vector�� number vector ����
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
				if (codeIt != itemCodeVec.end()) //���� �ʿ� �������� ���� �������� ������ ���
				{
					
					int index = std::distance(itemCodeVec.begin(), codeIt); //�ݺ��ڸ� �ε����� ��ȯ
					prt(L"�ʿ��� ������ %d�� ���� �������� ������\n", itemCodeVec[index]);
					if (numberVec[index] < alchemyDex[alchemyCounter].reactant[recipeCounter].second)
					{
						prt(L"������ %d ���� ����(���� %d)���� break;\n", itemCodeVec[index],numberVec[index]);
						break;//���ڰ� ������� Ȯ��
					}
				}
				else
				{
					prt(L"������ %d�� �������� ��� break\n", alchemyDex[alchemyCounter].reactant[recipeCounter].first);
					break;
				}

				if (recipeCounter == alchemyDex[alchemyCounter].reactant.size() - 1)//�������� �������� �������� ���
				{
					prt(L"���ݼ� ������ ����\n");

					int reactPtrVecSize = 0;
					for (int i = 0; i < reactPtrVec.size(); i++)
					{
						if (reactPtrVec[i] != nullptr) reactPtrVecSize++;
					}

					if (itemCodeVec.size() == reactPtrVecSize)
					{
						prt(L"���ݼ� ����\n");
						//std::wprintf(L"����\n");
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
			new Lst(sysStr[95], sysStr[94], toolQualityList);//�ֱ�, ���� ������ �������ּ���.
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
			prt(L"Alchemy While ���� �����\n");

			if (negateMonster == false)
			{
				prt(L"���� �����ұ�?\n");
				for (int x = Player::ins()->getGridX() - 1; x <= Player::ins()->getGridX() + 1; x++)
				{
					for (int y = Player::ins()->getGridY() - 1; y <= Player::ins()->getGridY() + 1; y++)
					{
						if (!(x == Player::ins()->getGridX() && y == Player::ins()->getGridY()))
							if (World::ins()->getTile(x, y, Player::ins()->getGridZ()).fov == fovFlag::white)
								if (World::ins()->getTile(x, y, Player::ins()->getGridZ()).EntityPtr != nullptr)
								{
									new Msg(msgFlag::normal, L"���", L"�ֺ��� ���� �ֽ��ϴ�. ��� �����Ͻðڽ��ϱ�?", { L"��",L"�ƴϿ�",L"�����ϱ�" });
									co_await std::suspend_always();
									if (coAnswer == L"��") goto loopEnd;
									else if (coAnswer == L"�����ϱ�")
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

			turnWait(1.0);
			coTurnSkip = true;

			prt(L"exeCraft �ڷ�ƾ ���� ��\n");

			co_await std::suspend_always();

			prt(L"exeCraft �ڷ�ƾ ���� ��\n");

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


