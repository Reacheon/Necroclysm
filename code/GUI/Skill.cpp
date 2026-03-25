module;
#define CORO(func) delete coFunc; coFunc = new Corouter(func); (*coFunc).run();
#include <SDL3/SDL.h>

module Skill;

import std;
import util;
import constVar;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import Player;
import SkillData;
import useSkill;
import wrapFunc;

Skill::Skill() : GUI(false)
{
	//1개 이상의 메시지 객체 생성 시의 예외 처리
	errorBox(ptr != nullptr, L"More than one message instance was generated.");
	ptr = this;

	//메세지 박스 렌더링
	changeXY(cameraW / 2, cameraH / 2, true);

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

Skill::~Skill()
{
	ptr = nullptr;
}

void Skill::changeXY(int inputX, int inputY, bool center)
{
	skillBase = { 0, 0, 410, 548 };

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

	generalBox = { skillBase.x + 22 + 90 * 0,skillBase.y + 82,90,26 };
	mutationBox = { skillBase.x + 22 + 90 * 1,skillBase.y + 82,90,26 };
	bionicBox = { skillBase.x + 22 + 90 * 2,skillBase.y + 82,90,26 };
	magicBox = { skillBase.x + 22 + 90 * 3,skillBase.y + 82,90,26 };

	for (int i = 0; i < 7; i++)
	{
		skillBtn[i] = { skillBase.x + 13, skillBase.y + 121 + 61*i, 374,48 };
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

void Skill::drawGUI()
{
	if (getStateDraw() == false) { return; }

	if (getFoldRatio() == 1.0)
	{
		drawWindow(&skillBase, sysStr[197], 113);

		auto drawSubcategoryBox = [](std::wstring boxStr, SDL_Rect box, bool pressed, bool deactColorChange)
			{
				SDL_Color btnColor = { 0x00, 0x00, 0x00 };
				SDL_Color outlineColor = col::gray;
				SDL_Color letterColor = { 0xff,0xff,0xff };
				if (checkCursor(&box) && deactColorChange == false)
				{
					if (click == false) { btnColor = lowCol::blue; }
					else { btnColor = lowCol::deepBlue; }
					outlineColor = col::gray;
				}
				else if (pressed)
				{
					btnColor = lowCol::deepBlue;
					outlineColor = col::gray;
				}

				if (pressed)
				{
					box.h += 2;
					box.y -= 2;
				}

				drawFillRect(box, btnColor);
				drawRect(box, outlineColor);

				if (pressed)
				{
					SDL_Rect bottomWhiteRect = { box.x + 20, box.y + 24, 50,3 };
					drawFillRect(bottomWhiteRect, col::white);
				}

				setFontSize(14);
				drawTextCenter(boxStr, box.x + box.w / 2, box.y + box.h / 2 - 1, letterColor);
			};

		drawLine(skillBase.x + 3, skillBase.y + 107, skillBase.x + 405, skillBase.y + 107, col::gray);
		// 왼쪽 그라데이션
		drawLine(skillBase.x + 3, skillBase.y + 107, skillBase.x + 3 + 16, skillBase.y + 107, { 0x4c,0x4c,0x4c });
		drawLine(skillBase.x + 3, skillBase.y + 107, skillBase.x + 3 + 12, skillBase.y + 107, { 0x34,0x34,0x34 });
		drawLine(skillBase.x + 3, skillBase.y + 107, skillBase.x + 3 + 8, skillBase.y + 107, { 0x26,0x26,0x26 });
		drawLine(skillBase.x + 3, skillBase.y + 107, skillBase.x + 3 + 4, skillBase.y + 107, { 0x1f,0x1f,0x1f });
		// 오른쪽 그라데이션
		drawLine(skillBase.x + 405 - 16, skillBase.y + 107, skillBase.x + 405, skillBase.y + 107, { 0x4c,0x4c,0x4c });
		drawLine(skillBase.x + 405 - 12, skillBase.y + 107, skillBase.x + 405, skillBase.y + 107, { 0x34,0x34,0x34 });
		drawLine(skillBase.x + 405 - 8, skillBase.y + 107, skillBase.x + 405, skillBase.y + 107, { 0x26,0x26,0x26 });
		drawLine(skillBase.x + 405 - 4, skillBase.y + 107, skillBase.x + 405, skillBase.y + 107, { 0x1f,0x1f,0x1f });


		drawSubcategoryBox(sysStr[199], generalBox, categoryCursor == skillCategory::general, false);
		drawSubcategoryBox(sysStr[200], mutationBox, categoryCursor == skillCategory::mutation, false);
		drawSubcategoryBox(sysStr[201], bionicBox, categoryCursor == skillCategory::bionic, false);
		drawSubcategoryBox(sysStr[202], magicBox, categoryCursor == skillCategory::magic, false);


		// 스킬 스크롤 그리기
		if (filteredSkills.size() > SKILL_GUI_MAX)
		{
			SDL_Rect skillScrollBox = { skillBase.x + 397, skillBase.y + 121, 2, 414 };
			drawFillRect(skillScrollBox, { 120,120,120 });
			SDL_Rect inScrollBox = skillScrollBox; // 내부 스크롤 커서
			inScrollBox.h = skillScrollBox.h * myMin(1.0, (float)SKILL_GUI_MAX / (float)filteredSkills.size());
			inScrollBox.y = skillScrollBox.y + skillScrollBox.h * ((float)skillScroll / (float)filteredSkills.size());
			if (inScrollBox.y + inScrollBox.h > skillScrollBox.y + skillScrollBox.h) { inScrollBox.y = skillScrollBox.y + skillScrollBox.h - inScrollBox.h; }
			drawFillRect(inScrollBox, col::white);
		}

		//setFontSize(10);
		//std::wstring aquiredSkillText = sysStr[231] + L" : 13";
		//drawText(aquiredSkillText, skillBase.x + 272 - queryTextWidth(aquiredSkillText), skillBase.y +34);//습득한 스킬

		for (int i = 0; i < 7; i++)
		{
			if (skillScroll + i < filteredSkills.size())
			{

				SDL_Color skillBtnColor = col::black;
				SDL_Color skillOutlineColor = col::gray;
				if (checkCursor(&skillBtn[i]))
				{
					if (click) skillBtnColor = { 0x14, 0x38, 0x78 };
					else skillBtnColor = lowCol::deepBlue;
				}
				drawFillRect(skillBtn[i], skillBtnColor);
				drawRect(skillBtn[i], skillOutlineColor);
				SkillData tgtData = filteredSkills[skillScroll + i];
				setZoom(2.0);
				drawSprite(spr::icon24, tgtData.iconIndex, skillBtn[i].x, skillBtn[i].y);
				setZoom(1.0);

				std::wstring skillName = L"";
				skillName = tgtData.name;
				setFontSize(22);
				setFont(fontType::mainFontMedium);
				drawText(skillName, skillBtn[i].x + 58, skillBtn[i].y + 3);

				drawLine(skillBtn[i].x + skillBtn[i].w - 1 - 36, skillBtn[i].y, skillBtn[i].x + skillBtn[i].w - 1 - 36, skillBtn[i].y + skillBtn[i].h - 1, col::gray);

				setFont(fontType::mainFontSemiBold);
				setFontSize(16);
				std::wstring rankText = L"Rank " + tgtData.skillRank;
				drawText(rankText, skillBtn[i].x + 330- queryTextWidth(rankText), skillBtn[i].y + 4);
				setFont(fontType::mainFont);
				std::wstring profText = L"Invocation / Fighting";
				setFontSize(12);
				drawText(profText, skillBtn[i].x + 330 - queryTextWidth(profText), skillBtn[i].y + 25);


				drawTextCenter(L"Fail", skillBtn[i].x + skillBtn[i].w - 20, skillBtn[i].y + 12);
				setFont(fontType::mainFontSemiBold);
				setFontSize(15);
				drawTextCenter(L"33%", skillBtn[i].x + skillBtn[i].w - 18, skillBtn[i].y + 29);
			}
		}

		if (dragSkillTarget != -1)
		{
			bool cursorIconDraw = true;
			for (int i = 0; i < 7; i++)
			{
				if (skillScroll + i < filteredSkills.size() && checkCursor(&skillBtn[i]))
				{
					if (filteredSkills[skillScroll + i].skillCode == dragSkillTarget)
					{
						cursorIconDraw = false;
					}
				}
			}

			if (cursorIconDraw)
			{
				setZoom(2.0);
				SDL_SetTextureAlphaMod(spr::icon24->getTexture(), 180); //텍스쳐 투명도 설정
				SDL_SetTextureBlendMode(spr::icon24->getTexture(), SDL_BLENDMODE_BLEND); //블렌드모드 설정
				drawSpriteCenter(spr::icon24, skillDex[dragSkillTarget].iconIndex, getMouseX(), getMouseY());
				SDL_SetTextureAlphaMod(spr::icon24->getTexture(), 255); //텍스쳐 투명도 설정
				setZoom(1.0);
			}
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

void Skill::clickUpGUI()
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
			if (skillScroll + i < filteredSkills.size() && checkCursor(&skillBtn[i]))
			{
				if (dragSkillTarget != -1 && filteredSkills[skillScroll + i].skillCode == dragSkillTarget)
				{
					CORO(useSkill(dragSkillTarget));
					delete this;
				}
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

void Skill::clickDownGUI()
{
	for (int i = 0; i < SKILL_GUI_MAX; i++)
	{
		if (dragSkillTarget == -1)
		{
			if (skillScroll + i < filteredSkills.size() && checkCursor(&skillBtn[i]))
			{
				dragSkillTarget = filteredSkills[skillScroll + i].skillCode;
			}
		}
	}
}

void Skill::clickRightGUI()
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

void Skill::mouseWheel()
{
	if (checkCursor(&skillBase))
	{
		if (event.wheel.y > 0 && skillScroll > 1) skillScroll -= 1;
		else if (event.wheel.y < 0 && skillScroll + SKILL_GUI_MAX < filteredSkills.size()) skillScroll += 1;
	}
}

void Skill::step()
{
	tabType = tabFlag::back;

	//잘못된 스크롤 위치 조정
	if (option::inputMethod == input::mouse || option::inputMethod == input::touch)
	{
		if (skillScroll + SKILL_GUI_MAX >= filteredSkills.size()) { skillScroll = myMax(0, (int)filteredSkills.size() - SKILL_GUI_MAX); }
		else if (skillScroll < 0) { skillScroll = 0; }
	}
}
