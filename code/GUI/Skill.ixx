#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();
#include <SDL3/SDL.h>

export module Skill;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import Player;
import SkillData;
import useSkill;
import wrapVar;

enum class skillCategory
{
	all,
	general,
	mutation,
	bionic,
	magic,
};

export class Skill : public GUI
{
private:
	inline static Skill* ptr = nullptr;
	SDL_Rect skillBase;
	SDL_Rect generalBox;
	SDL_Rect mutationBox;
	SDL_Rect bionicBox;
	SDL_Rect magicBox;
	int skillCursor = -1;
	int skillScroll = 0;
	int initSkillScroll = 0; //모션스크롤이 시작되기 직전의 스크롤

	skillCategory categoryCursor = skillCategory::general; //0일반, 1돌연변이, 2바이오닉, 3마법

	std::array<SDL_Rect,7> skillBtn;

	std::vector<SkillData> filteredSkills; //카테고리에 의해 필터링된 스킬들, 기본값은 전체 스킬(생성될 때)

	int dragSkillTarget = -1;
public:
	Skill() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one message instance was generated.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		tabType = tabFlag::closeWin;

		deactInput();
		deactDraw();
		addAniUSetPlayer(this, aniFlag::winUnfoldOpen);

		filteredSkills.clear();
		for (int i = 0; i < PlayerPtr->entityInfo.skillList.size(); i++)
		{
			if (PlayerPtr->entityInfo.skillList[i].src == skillSrc::GENERAL)
				filteredSkills.push_back(PlayerPtr->entityInfo.skillList[i]);
		}
	}
	~Skill()
	{
		tabType = tabFlag::autoAtk;
		ptr = nullptr;
	}
	static Skill* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{


		skillBase = { 0, 0, 280, 408 };


		if (center == false)
		{
			skillBase.x += inputX;
			skillBase.y += inputY;
		}
		else
		{
			skillBase.x += inputX - skillBase.w / 2;
			skillBase.y += inputY - skillBase.h / 2;
		}

		generalBox = { skillBase.x,skillBase.y + 68,66,20 };
		mutationBox = { skillBase.x + 66,skillBase.y + 68,66,20 };
		bionicBox = { skillBase.x + 132,skillBase.y + 68,66,20 };
		magicBox = { skillBase.x + 198,skillBase.y + 68,66,20 };

		for (int i = 0; i < 7; i++)
		{
			skillBtn[i] = { skillBase.x + 14, skillBase.y + 101 + 43*i, 250,34 };
		}
		

		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - skillBase.w / 2;
			y = inputY - skillBase.h / 2;
		}

	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			drawWindow(&skillBase, sysStr[197], 5);


			auto drawSubcategoryBox = [](std::wstring boxStr, SDL_Rect box, bool pressed, bool deactColorChange)
				{
					SDL_Color btnColor = { 0x00, 0x00, 0x00 };
					SDL_Color outlineColor = { 0x4A, 0x4A, 0x4A };
					SDL_Color letterColor = { 0xff,0xff,0xff };
					if (checkCursor(&box) && deactColorChange == false)
					{
						if (click == false) { btnColor = lowCol::blue; }
						else { btnColor = lowCol::deepBlue; }
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}
					else if (pressed)
					{
						btnColor = lowCol::deepBlue;
						outlineColor = { 0xa6, 0xa6, 0xa6 };
					}

					if (pressed)
					{
						box.h += 2;
						box.y -= 2;
					}


					drawFillRect(box, btnColor);
					drawRect(box, col::gray);

					if (pressed)
					{
						SDL_Rect bottomWhiteRect = { box.x + 9, box.y + 19, 42,2 };
						drawRect(bottomWhiteRect, col::white);
					}

					setFontSize(10);
					renderTextCenter(boxStr, box.x + box.w / 2, box.y + box.h / 2, letterColor);
				};

			drawLine(skillBase.x+1, skillBase.y+87, skillBase.x + 1+278, skillBase.y + 87, col::lightGray);

			drawSubcategoryBox(sysStr[199], generalBox, categoryCursor == skillCategory::general, false);
			drawSubcategoryBox(sysStr[200], mutationBox, categoryCursor == skillCategory::mutation, false);
			drawSubcategoryBox(sysStr[201], bionicBox, categoryCursor == skillCategory::bionic, false);
			drawSubcategoryBox(sysStr[202], magicBox, categoryCursor == skillCategory::magic, false);

			// 스킬 스크롤 그리기
			if (filteredSkills.size() > SKILL_GUI_MAX)
			{
				SDL_Rect skillScrollBox = { skillBase.x + 270, skillBase.y + 101, 2, 292 };
				drawFillRect(skillScrollBox, { 120,120,120 });
				SDL_Rect inScrollBox = skillScrollBox; // 내부 스크롤 커서
				inScrollBox.h = skillScrollBox.h * myMin(1.0, (float)SKILL_GUI_MAX / (float)filteredSkills.size());
				inScrollBox.y = skillScrollBox.y + skillScrollBox.h * ((float)skillScroll / (float)filteredSkills.size());
				if (inScrollBox.y + inScrollBox.h > skillScrollBox.y + skillScrollBox.h) { inScrollBox.y = skillScrollBox.y + skillScrollBox.h - inScrollBox.h; }
				drawFillRect(inScrollBox, col::white);
			}

			setFontSize(10);
			std::wstring aquiredSkillText = sysStr[231] + L" : 13";
			renderText(aquiredSkillText, skillBase.x + 272 - queryTextWidth(aquiredSkillText), skillBase.y +34);//습득한 스킬

			for (int i = 0; i < 7; i++)
			{
				if (skillScroll + i < filteredSkills.size())
				{
					SkillData tgtData = filteredSkills[skillScroll + i];


					int btnIndex = 0;
					if (checkCursor(&skillBtn[i]))
					{
						if (click) btnIndex = 2;
						else btnIndex = 1;
					}
					drawSprite(spr::skillRect, btnIndex, skillBtn[i].x, skillBtn[i].y);

					setZoom(2.0);
					std::wstring skillName = L"";
					drawSprite(spr::skillSet, tgtData.iconIndex, skillBtn[i].x + 1, skillBtn[i].y + 1);
					skillName = tgtData.name;
					setZoom(1.0);

					setFontSize(12);

					renderText(skillName, skillBtn[i].x + 43, skillBtn[i].y + 3);
					int textWidth = queryTextWidth(skillName);


					renderText(tgtData.skillRank, skillBtn[i].x + 231, skillBtn[i].y + 3, lowCol::green);

					setFontSize(10);
					
					std::wstring expStr = decimalCutter(tgtData.skillExp,1);
					renderText(expStr + L"/100.0", skillBtn[i].x + 182, skillBtn[i].y + 20);

					drawRect({ skillBtn[i].x + 41,skillBtn[i].y + 23, 134,7 }, col::gray);
					drawFillRect(SDL_Rect{ skillBtn[i].x + 41 + 2,skillBtn[i].y + 23 + 2, int(42.0* (myMin(1.0,tgtData.skillExp/100.0))),3 }, col::white);

					drawLine(skillBtn[i].x + 33, skillBtn[i].y + 1, skillBtn[i].x + 33, skillBtn[i].y + 32, { 0x80,0x80,0x80 });

					//drawCross2(skillBtn[i].x, skillBtn[i].y, 0, 5, 0, 5);
					//drawCross2(skillBtn[i].x + 32, skillBtn[i].y, 0, 5, 5, 0);
					//drawCross2(skillBtn[i].x, skillBtn[i].y + 32, 5, 0, 0, 5);
					//drawCross2(skillBtn[i].x + 32, skillBtn[i].y + 32, 5, 0, 5, 0);
				}
			}


			if (dragSkillTarget != -1)
			{
				setZoom(2.0);
				SDL_SetTextureAlphaMod(spr::skillSet->getTexture(), 180); //텍스쳐 투명도 설정
				SDL_SetTextureBlendMode(spr::skillSet->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
				drawSpriteCenter(spr::skillSet, skillDex[dragSkillTarget].iconIndex, getMouseX(), getMouseY());
				SDL_SetTextureAlphaMod(spr::skillSet->getTexture(), 255); //텍스쳐 투명도 설정
				setZoom(1.0);
			}

		}
		else
		{
			SDL_Rect vRect = skillBase;
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
		else if (checkCursor(&generalBox))
		{
			if (categoryCursor != skillCategory::general) categoryCursor = skillCategory::general;
		}
		else if (checkCursor(&mutationBox))
		{
			if (categoryCursor != skillCategory::mutation) categoryCursor = skillCategory::mutation;
		}
		else if (checkCursor(&bionicBox))
		{
			if (categoryCursor != skillCategory::bionic) categoryCursor = skillCategory::bionic;
		}
		else if (checkCursor(&magicBox))
		{
			if (categoryCursor != skillCategory::magic) categoryCursor = skillCategory::magic;
		}
		else
		{
			for (int i = 0; i < QUICK_SLOT_MAX; i++)
			{
				if (checkCursor(&quickSlotBtn[i]) && dragSkillTarget != -1)
				{
					for (int j = 0; j < QUICK_SLOT_MAX; j++)
					{
						if (quickSlot[j].first == quickSlotFlag::SKILL && quickSlot[j].second == dragSkillTarget)
						{
							quickSlot[j].first = quickSlotFlag::NONE;
							quickSlot[j].second = -1;
						}
					}

					quickSlot[i].first = quickSlotFlag::SKILL;
					quickSlot[i].second = dragSkillTarget;
				}
			}

			for (int i = 0; i < SKILL_GUI_MAX; i++)
			{
				if (checkCursor(&skillBtn[i]))
				{
					CORO(useSkill(dragSkillTarget));

					delete this;
				}
			}
		}


		if (categoryCursor == skillCategory::all) filteredSkills = PlayerPtr->entityInfo.skillList;
		else if (categoryCursor == skillCategory::general)
		{
			filteredSkills.clear();
			for (int i = 0; i < PlayerPtr->entityInfo.skillList.size(); i++)
			{
				if (PlayerPtr->entityInfo.skillList[i].src == skillSrc::GENERAL) filteredSkills.push_back(PlayerPtr->entityInfo.skillList[i]);
			}
		}
		else if (categoryCursor == skillCategory::mutation)
		{
			filteredSkills.clear();
			for (int i = 0; i < PlayerPtr->entityInfo.skillList.size(); i++)
			{
				if (PlayerPtr->entityInfo.skillList[i].src == skillSrc::MUTATION) filteredSkills.push_back(PlayerPtr->entityInfo.skillList[i]);
			}
		}
		else if (categoryCursor == skillCategory::bionic)
		{
			filteredSkills.clear();
			for (int i = 0; i < PlayerPtr->entityInfo.skillList.size(); i++)
			{
				if (PlayerPtr->entityInfo.skillList[i].src == skillSrc::BIONIC)
				{
					filteredSkills.push_back(PlayerPtr->entityInfo.skillList[i]);
				}
			}
		}
		else if (categoryCursor == skillCategory::magic)
		{
			filteredSkills.clear();
			for (int i = 0; i < PlayerPtr->entityInfo.skillList.size(); i++)
			{
				if (PlayerPtr->entityInfo.skillList[i].src == skillSrc::MAGIC)
				{
					filteredSkills.push_back(PlayerPtr->entityInfo.skillList[i]);
				}
			}
		}

		dragSkillTarget = -1;

	}
	void clickMotionGUI(int dx, int dy) 
	{

	}
	void clickDownGUI() 
	{
		for (int i = 0; i < SKILL_GUI_MAX; i++)
		{
			if (checkCursor(&skillBtn[i]))
			{
				dragSkillTarget = filteredSkills[skillScroll + i].skillCode;
			}
		}
		
	}
	void clickRightGUI() 
	{
		if (checkCursor(&quickSlotRegion) == true)
		{
			for (int i = 0; i < QUICK_SLOT_MAX; i++)
			{
				if (checkCursor(&quickSlotBtn[i]))
				{
					quickSlot[i].first = quickSlotFlag::NONE;
					quickSlot[i].second = -1;
				}
			}
		}
	}
	void clickHoldGUI() {}
	void mouseWheel() 
	{
		if (checkCursor(&skillBase))
		{
			if (event.wheel.y > 0 && skillScroll > 1) skillScroll -= 1;
			else if (event.wheel.y < 0 && skillScroll + SKILL_GUI_MAX < filteredSkills.size()) skillScroll += 1;
		}
	}
	void gamepadBtnDown() {}
	void gamepadBtnMotion() {}
	void gamepadBtnUp() {}
	void step() 
	{
		//잘못된 스크롤 위치 조정
		if (option::inputMethod == input::mouse || option::inputMethod == input::touch)
		{
			if (skillScroll + SKILL_GUI_MAX >= filteredSkills.size()) { skillScroll = myMax(0, (int)filteredSkills.size() - SKILL_GUI_MAX); }
			else if (skillScroll < 0) { skillScroll = 0; }
		}
	}
};