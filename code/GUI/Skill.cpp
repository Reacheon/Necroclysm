#include <SDL3/SDL.h>

import Skill;

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
import SkillBehavior;
import SkillRegistry;
import useSkill;
import World;
import Entity;

//스킬 실패율에 따른 확률 텍스트 색상 변경
static SDL_Color getFailColor(int failRate)
{
	if (failRate <= 4) return col::yellowGreen;
	if (failRate <= 20) return lowCol::yellow;
	return lowCol::red;
}

Skill::Skill() : GUI(false)
{
	//1개 이상의 메시지 객체 생성 시의 예외 처리
	errorBox(ptr != nullptr, L"중복된 GUI 인스턴스가 생성되었다.");
	ptr = this;

	//메세지 박스 렌더링
	changeXY(cameraW / 2, cameraH / 2, true);

	deactInput();
	deactDraw();
	addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);

	refilterSkills();
}

// 표시할 탭 목록과 filteredSkills 재구성 (생성자/클릭 공용).
// 탭은 전체 + 해당 카테고리 스킬을 하나라도 보유한 것만, filteredSkills는 categoryCursor 기준 필터.
void Skill::refilterSkills()
{
	visibleCats.clear();
	visibleCats.push_back(skillCategory::all);
	for (int c = (int)skillCategory::weapon; c <= (int)skillCategory::divinity; c++)
	{
		for (auto& sd : PlayerInfo().skillList)
		{
			auto* bhv = SkillRegistry::get(sd.skillId);
			if (bhv && bhv->getCategory() == static_cast<skillCategory>(c))
			{
				visibleCats.push_back(static_cast<skillCategory>(c));
				break;
			}
		}
	}

	// 보고 있던 탭이 사라졌으면 전체로 복귀
	if (std::find(visibleCats.begin(), visibleCats.end(), categoryCursor) == visibleCats.end())
		categoryCursor = skillCategory::all;

	filteredSkills.clear();
	for (auto& sd : PlayerInfo().skillList)
	{
		if (categoryCursor == skillCategory::all)
		{
			filteredSkills.push_back(sd);
			continue;
		}

		auto* bhv = SkillRegistry::get(sd.skillId);
		if (bhv && bhv->getCategory() == categoryCursor) filteredSkills.push_back(sd);
	}
}

Skill::~Skill()
{
	ptr = nullptr;
}

void Skill::changeXY(int inputX, int inputY, bool center)
{
	skillBase = { 0, 0, 516, 609 };

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

	for (int i = 0; i < SKILL_GUI_MAX; i++)
	{
		skillBtn[i] = { skillBase.x + 13, skillBase.y + 121 + 61 * i, 480,48 };
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
		drawWindow(&skillBase, sysStr[120], 113);

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
					SDL_Rect bottomWhiteRect = { box.x + box.w / 2 - 20, box.y + 24, 40,3 };
					drawFillRect(bottomWhiteRect, col::white);
				}

				setFontSize(13);
				drawTextCenter(boxStr, box.x + box.w / 2, box.y + box.h / 2 - 1, letterColor);
			};

		drawLine(skillBase.x + 3, skillBase.y + 107, skillBase.x + 511, skillBase.y + 107, col::gray);
		// 왼쪽 그라데이션 (탭 여백 10px 안에서 페이드가 끝나도록 8px 폭)
		drawLine(skillBase.x + 3, skillBase.y + 107, skillBase.x + 3 + 8, skillBase.y + 107, { 0x4c,0x4c,0x4c });
		drawLine(skillBase.x + 3, skillBase.y + 107, skillBase.x + 3 + 6, skillBase.y + 107, { 0x34,0x34,0x34 });
		drawLine(skillBase.x + 3, skillBase.y + 107, skillBase.x + 3 + 4, skillBase.y + 107, { 0x26,0x26,0x26 });
		drawLine(skillBase.x + 3, skillBase.y + 107, skillBase.x + 3 + 2, skillBase.y + 107, { 0x1f,0x1f,0x1f });
		// 오른쪽 그라데이션
		drawLine(skillBase.x + 511 - 8, skillBase.y + 107, skillBase.x + 511, skillBase.y + 107, { 0x4c,0x4c,0x4c });
		drawLine(skillBase.x + 511 - 6, skillBase.y + 107, skillBase.x + 511, skillBase.y + 107, { 0x34,0x34,0x34 });
		drawLine(skillBase.x + 511 - 4, skillBase.y + 107, skillBase.x + 511, skillBase.y + 107, { 0x26,0x26,0x26 });
		drawLine(skillBase.x + 511 - 2, skillBase.y + 107, skillBase.x + 511, skillBase.y + 107, { 0x1f,0x1f,0x1f });


		setFontSize(14);
		drawText(sysStr[297]+col2Str(lowCol::green)+std::to_wstring(PlayerPtr->skillPoint), skillBase.x + skillBase.w - 162, skillBase.y + 43);


		// 탭 라벨: 전체/무기술/생존/행동/바이오닉/돌연변이/신성력 (skillCategory 순서)
		static constexpr int catStrIdx[7] = { 261, 262, 263, 264, 123, 122, 124 };
		for (int c = 0; c < (int)visibleCats.size(); c++)
		{
			SDL_Rect catBox = { skillBase.x + 13 + 70 * c, skillBase.y + 82, 70, 26 };
			drawSubcategoryBox(sysStr[catStrIdx[(int)visibleCats[c]]], catBox, categoryCursor == visibleCats[c], false);
		}


		// 스킬 스크롤 그리기
		if (filteredSkills.size() > SKILL_GUI_MAX)
		{
			SDL_Rect skillScrollBox = { skillBase.x + 503, skillBase.y + 121, 2, 475 };
			drawFillRect(skillScrollBox, { 120,120,120 });
			SDL_Rect inScrollBox = skillScrollBox; // 내부 스크롤 커서
			inScrollBox.h = skillScrollBox.h * myMin(1.0, (float)SKILL_GUI_MAX / (float)filteredSkills.size());
			inScrollBox.y = skillScrollBox.y + skillScrollBox.h * ((float)skillScroll / (float)filteredSkills.size());
			if (inScrollBox.y + inScrollBox.h > skillScrollBox.y + skillScrollBox.h) { inScrollBox.y = skillScrollBox.y + skillScrollBox.h - inScrollBox.h; }
			drawFillRect(inScrollBox, col::white);
		}

		//setFontSize(10);
		//std::wstring aquiredSkillText = sysStr[150] + L" : 13";
		//drawText(aquiredSkillText, skillBase.x + 272 - queryTextWidth(aquiredSkillText), skillBase.y +34);//습득한 스킬

		for (int i = 0; i < SKILL_GUI_MAX; i++)
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
				SkillData& tgtData = filteredSkills[skillScroll + i];
				auto* tgtBhv = SkillRegistry::get(tgtData.skillId);

				setZoom(2.0);
				drawSprite(spr::icon24, tgtBhv ? tgtBhv->iconIndex : 0, skillBtn[i].x, skillBtn[i].y);
				setZoom(1.0);

				// 사용 불가능한 스킬(패시브)은 아이콘 위에 테두리를 그려 구분
				if (tgtBhv && tgtBhv->type == skillType::PASSIVE)
					drawRect({ skillBtn[i].x, skillBtn[i].y, 48, 48 }, skillOutlineColor);

				// 토글 활성 이펙트
				if (tgtBhv && tgtBhv->type == skillType::TOGGLE && tgtData.toggle)
				{
					drawToggleSnakeEffect(skillBtn[i].x, skillBtn[i].y, 48, 48);
				}

				std::wstring skillName = tgtBhv ? tgtBhv->name : L"?";
				setFontSize(22);
				setFont(fontType::mainFontMedium);
				drawText(skillName, skillBtn[i].x + 58, skillBtn[i].y + 3);

				//실패율에 영향 미치는 참조 스킬들 (콤마 구분, 회색)
				std::wstring refText;
				if (tgtBhv)
				{
					for (size_t r = 0; r < tgtBhv->refSkills.size(); r++)
					{
						if (r > 0) refText += L", ";
						auto* refBhv = SkillRegistry::get(tgtBhv->refSkills[r]);
						refText += refBhv ? refBhv->name : L"?";
					}
				}
				if (!refText.empty())
				{
					setFont(fontType::mainFontMedium);
					setFontSize(12);
					drawText(col2Str(col::lightGray) + refText, skillBtn[i].x + 58, skillBtn[i].y + 29);
				}

				setFont(fontType::mainFontSemiBold);
				setFontSize(16);
				std::wstring rankText = sysStr[298]+L" " + tgtData.skillRank;
				drawText(rankText, skillBtn[i].x + skillBtn[i].w - 49 - queryTextWidth(rankText), skillBtn[i].y + 2);
				setFont(fontType::mainFont);

				// 바이오닉은 설치 시점에 랭크 고정 — 숙련치 게이지/승급 버튼 없음
				if (!tgtBhv || tgtBhv->src != skillSrc::BIONIC)
				{
					bool expFull = tgtData.skillExp >= 100.0f;

					//승급 버튼: 0=숙련치 가득(금색), 1=눌림, 2=숙련치 부족(회색)
					drawSprite(spr::skillRankUpBtn, expFull ? 0 : 2, skillBtn[i].x + skillBtn[i].w - 41, skillBtn[i].y + 6);

					//숙련치 게이지 (100에서 멈춤 — 초과 숙련치는 증발)
					drawRect({ skillBtn[i].x + skillBtn[i].w - 43 - 71, skillBtn[i].y + 22,67,8 }, col::white);
					int gaugeW = static_cast<int>(61.0f * myMin(tgtData.skillExp, 100.0f) / 100.0f);
					if (gaugeW > 0)
						drawFillRect(SDL_Rect{ skillBtn[i].x + skillBtn[i].w - 43 - 71 + 3, skillBtn[i].y + 22 + 3, gaugeW, 2 }, expFull ? SDL_Color{ 0xe1,0xb8,0x40 } : col::white);

					setFont(fontType::mainFont);
					setFontSize(12);
					std::wstring expText = std::format(L"{:.1f} / 100.0", myMin(tgtData.skillExp, 100.0f));
					drawText(expText, skillBtn[i].x + skillBtn[i].w - 43 - (expFull ? 73 : 71), skillBtn[i].y + 22 + 8);
				}

				drawLine(skillBtn[i].x + skillBtn[i].w - 120, skillBtn[i].y, skillBtn[i].x + skillBtn[i].w - 120, skillBtn[i].y + 47, skillOutlineColor);

				//액티브/토글 스킬은 사용 라벨과 실패율을 표시 (패시브는 없음)
				if (tgtBhv && tgtBhv->type != skillType::PASSIVE)
				{
					drawSprite(spr::skillActiveBtn, 0, skillBtn[i].x + skillBtn[i].w - 178, skillBtn[i].y + 4);
					setFont(fontType::mainFontMedium);
					setFontSize(15);
					drawTextCenter(tgtBhv->type == skillType::TOGGLE ? sysStr[299] : sysStr[300], skillBtn[i].x + skillBtn[i].w - 178 + 27, skillBtn[i].y + 4 + 12);

					int failRate = tgtBhv->calcFailRate(static_cast<Entity*>(PlayerPtr), tgtData);
					setFontSize(12);
					setFont(fontType::mainFontMedium);
					drawTextCenter(col2Str(col::lightGray) + sysStr[301] + col2Str(getFailColor(failRate)) + L" " + std::to_wstring(failRate) + L"%", skillBtn[i].x + skillBtn[i].w - 178 + 27, skillBtn[i].y + 4 + 28);
				}
			}
		}

		if (!dragSkillTarget.empty())
		{
			bool cursorIconDraw = true;
			for (int i = 0; i < SKILL_GUI_MAX; i++)
			{
				if (skillScroll + i < filteredSkills.size() && checkCursor(&skillBtn[i]))
				{
					if (filteredSkills[skillScroll + i].skillId == dragSkillTarget)
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
				auto* dragBhv = SkillRegistry::get(dragSkillTarget);
				drawSpriteCenter(spr::icon24, dragBhv ? dragBhv->iconIndex : 0, getMouseX(), getMouseY());
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
	else
	{
		bool onCategory = false;
		for (int c = 0; c < (int)visibleCats.size(); c++)
		{
			SDL_Rect catBox = { skillBase.x + 13 + 70 * c, skillBase.y + 82, 70, 26 };
			if (checkCursor(&catBox))
			{
				if (categoryCursor != visibleCats[c])
				{
					categoryCursor = visibleCats[c];
					skillScroll = 0;  //탭 변경 시 스크롤 초기화
				}
				onCategory = true;
				break;
			}
		}

		if (onCategory == false)
		{
			for (int i = 0; i < QUICK_SLOT_MAX; i++)
			{
				if (checkCursor(&quickSlotBtn[i]) && !dragSkillTarget.empty())
				{
					for (int j = 0; j < QUICK_SLOT_MAX; j++)
					{
						if (quickSlot[j].first == quickSlotFlag::SKILL && quickSlot[j].second == dragSkillTarget)
						{
							quickSlot[j].first = quickSlotFlag::NONE;
							quickSlot[j].second.clear();
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
					if (!dragSkillTarget.empty() && filteredSkills[skillScroll + i].skillId == dragSkillTarget)
					{
						auto* clickedBhv = SkillRegistry::get(dragSkillTarget);

						// 패시브 스킬은 사용 불가 → 무시
						if (clickedBhv && clickedBhv->type == skillType::PASSIVE)
							break;

						useSkill(dragSkillTarget);

						// 토글 스킬은 GUI 유지 (filteredSkills의 toggle 상태 동기화)
						if (clickedBhv && clickedBhv->type == skillType::TOGGLE)
						{
							for (auto& sd : PlayerInfo().skillList)
							{
								if (sd.skillId == dragSkillTarget)
								{
									filteredSkills[skillScroll + i].toggle = sd.toggle;
									break;
								}
							}
							break;
						}

						// 액티브 스킬: 실제 발동(코루틴 시작)됐을 때만 GUI 닫기
						if (!currentUsingSkill.empty())
						{
							delete this;
							return;
						}
					}
				}
			}
		}
	}

	refilterSkills();

	dragSkillTarget.clear();
}

void Skill::clickDownGUI()
{
	for (int i = 0; i < SKILL_GUI_MAX; i++)
	{
		if (dragSkillTarget.empty())
		{
			if (skillScroll + i < filteredSkills.size() && checkCursor(&skillBtn[i]))
			{
				dragSkillTarget = filteredSkills[skillScroll + i].skillId;
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
