module;
#include <SDL3/SDL.h>

module Status;

import std;
import util;
import GUI;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import constVar;
import Player;
import SkillData;
import SkillBehavior;
import SkillRegistry;
import statusEffect;
import GodRegistry;
import GodBehavior;
import Sprite;
import Entity;
import playerLevel;

void Status::drawGUI()
	{
		if (getStateDraw() == false) { return; }

		if (getFoldRatio() == 1.0)
		{
			setWindowAlpha(200);
			drawWindow(&statusBase, L"Status", 4);
			resetWindowAlpha();

			// 호버 시 푸른색 틴트 적용 람다
			constexpr SDL_Color hoverBlue = { 0x22, 0x3c, 0x68 };
			auto stadiumCol = [&](const SDL_Rect& r) -> SDL_Color {
				return checkCursor(&r) ? hoverBlue : col::black;
			};


			// 초상화: 배경 스프라이트 위에 플레이어 합성 스프라이트를 4배 확대하여 그림
			{
				const SDL_Rect portraitBox = { statusBase.x + 12, statusBase.y + 46, 110, 110 };
				drawSprite(spr::statusPortraitBackground, 0, portraitBox.x, portraitBox.y);

				SDL_SetRenderClipRect(renderer, &portraitBox);

				SDL_Texture* portraitTex = PlayerPtr->composePlayerTexture();
				Sprite portraitSpr(renderer, portraitTex, 48, 48, true);

				setZoom(4.0);
				drawSpriteCenter(&portraitSpr, 0,
					portraitBox.x + portraitBox.w / 2 + 2,
					portraitBox.y + portraitBox.h / 2 + 22);
				setZoom(1.0);

				SDL_SetRenderClipRect(renderer, nullptr);
			}



			setFontSize(24);
			setFont(fontType::mainFontBold);
			std::wstring nameText = PlayerPtr->entityInfo.name;
			int nameTextWIdth = queryTextWidth(nameText, true);
			drawText(col2Str(SDL_Color{ 0xff,0xd3,0x44 }) + nameText, statusBase.x + 139, statusBase.y + 44);
			setFontSize(16);
			setFont(fontType::mainFontSemiBold);
			if (loopCount > 1)
				drawText(col2Str(SDL_Color{ 0xff,0xd3,0x44 }) + L"(Loop #" + std::to_wstring(loopCount) + L")", statusBase.x + 139 + nameTextWIdth + 6, statusBase.y + 50);
			//drawText(col2Str(SDL_Color{ 0xff,0xd3,0x44 }) + L"Nekdol (Loop #7)", statusBase.x + 139, statusBase.y + 44);

			//drawText(col2Str(SDL_Color{ 0xff,0xd3,0x44 }) + L"Nekdol, Survivor", statusBase.x + 139, statusBase.y + 44);
			setFont(fontType::mainFont);

			setFontSize(16);
			setFont(fontType::mainFontBold);
			//좌측 열: Age, Race, God
			Point2 agePivot = { statusBase.x + 137, statusBase.y + 79 };
			drawText(L"#e1772eAge", agePivot.x + 5, agePivot.y + 1);
			SDL_Rect ageRect = { agePivot.x + 55, agePivot.y, 120, 23 };
			drawStadium(ageRect, stadiumCol(ageRect), 255, 4);
			setFont(fontType::mainFontMedium);
			drawTextCenter(std::to_wstring(PlayerPtr->entityInfo.age), ageRect.x + ageRect.w / 2, ageRect.y + ageRect.h / 2);

			Point2 racePivot = { statusBase.x + 137, statusBase.y + 79 + 27 * 1 };
			setFont(fontType::mainFontBold);
			drawText(L"#e1772eRace", racePivot.x + 5, racePivot.y + 1);
			SDL_Rect raceRect = { racePivot.x + 55, racePivot.y, 120, 23 };
			drawStadium(raceRect, stadiumCol(raceRect), 255, 4);
			setFont(fontType::mainFontMedium);
			drawTextCenter(L"Human", raceRect.x + raceRect.w / 2, raceRect.y + raceRect.h / 2);

			Point2 godPivot = { statusBase.x + 137, statusBase.y + 79 + 27 * 2 };
			setFont(fontType::mainFontBold);
			drawText(L"#e1772eGod", godPivot.x + 5, godPivot.y + 1);
			SDL_Rect godRect = { godPivot.x + 55, godPivot.y, 120, 23 };
			drawStadium(godRect, stadiumCol(godRect), 255, 4);
			setFont(fontType::mainFontMedium);
			std::wstring godName = L"-";
			if (playerGod != godFlag::none)
			{
				GodBehavior* godBhv = GodRegistry::get(playerGod);
				if (godBhv) godName = godBhv->name;
			}
			drawTextCenter(godName, godRect.x + godRect.w / 2, godRect.y + godRect.h / 2);


			//우측 열: Hunger, Thirsty, Fatigue (퍼센트 실시간 표시)
			auto makePercentStr = [](double percent) -> std::wstring {
				return std::to_wstring((int)std::round(percent)) + L"%";
			};
			auto percentColor = [](double percent) -> std::wstring {
				if (percent >= 75.0) return col2Str(SDL_Color{ 0xff, 0x44, 0x44 });
				if (percent >= 50.0) return col2Str(SDL_Color{ 0xff, 0xc1, 0x07 });
				if (percent >= 25.0) return col2Str(SDL_Color{ 0xff, 0xe0, 0x80 });
				return L"";
			};

			Point2 hungerPivot = { statusBase.x + 137 + 221, statusBase.y + 79 };
			setFont(fontType::mainFontBold);
			drawText(L"#e1772eHunger", hungerPivot.x + 5, hungerPivot.y + 1);
			SDL_Rect hungerRect = { hungerPivot.x + 75, hungerPivot.y, 77, 23 };
			drawStadium(hungerRect, stadiumCol(hungerRect), 255, 4);
			setFont(fontType::mainFontMedium);
			drawTextCenter(percentColor(hunger) + makePercentStr(hunger), hungerRect.x + hungerRect.w / 2, hungerRect.y + hungerRect.h / 2);

			Point2 thirstyPivot = { statusBase.x + 137 + 221, statusBase.y + 79 + 27 * 1 };
			setFont(fontType::mainFontBold);
			drawText(L"#e1772eThirsty", thirstyPivot.x + 5, thirstyPivot.y + 1);
			SDL_Rect thirstyRect = { thirstyPivot.x + 75, thirstyPivot.y, 77, 23 };
			drawStadium(thirstyRect, stadiumCol(thirstyRect), 255, 4);
			setFont(fontType::mainFontMedium);
			drawTextCenter(percentColor(thirst) + makePercentStr(thirst), thirstyRect.x + thirstyRect.w / 2, thirstyRect.y + thirstyRect.h / 2);

			Point2 fatiguePivot = { statusBase.x + 137 + 221, statusBase.y + 79 + 27 * 2 };
			setFont(fontType::mainFontBold);
			drawText(L"#e1772eFatigue", fatiguePivot.x + 5, fatiguePivot.y + 1);
			SDL_Rect fatigueRect = { fatiguePivot.x + 75, fatiguePivot.y, 77, 23 };
			drawStadium(fatigueRect, stadiumCol(fatigueRect), 255, 4);
			setFont(fontType::mainFontMedium);
			drawTextCenter(percentColor(fatigue) + makePercentStr(fatigue), fatigueRect.x + fatigueRect.w / 2, fatigueRect.y + fatigueRect.h / 2);
			setFont(fontType::mainFont);



			drawRadarChart(stadiumCol);


			//세로 구분선 (하단 실선 + 상단 페이드아웃)
			{
				int lx = statusBase.x + 294;
				int yBottom = statusBase.y + 490;
				int yFadeStart = statusBase.y + 490 - 275;
				int yTop = statusBase.y + 490 - 310;
				SDL_Color lineCol = { 0x63, 0x63, 0x63 };

				//하단 실선 구간
				drawLine(lx, yBottom, lx, yFadeStart, lineCol);

				//상단 페이드아웃 구간
				int fadeLen = yFadeStart - yTop;
				for (int py = yFadeStart; py >= yTop; py--)
				{
					Uint8 a = (Uint8)(255.0 * (py - yTop) / fadeLen);
					drawPoint(lx, py, lineCol, a);
				}
			}

			drawLine(statusBase.x + 1, statusBase.y + 416, statusBase.x + 1 + 293, statusBase.y + 416, { 0x63,0x63,0x63 });
			drawFillRect(statusBase.x + 1, statusBase.y + 385, 81, 32, { 0x63,0x63,0x63 });
			setFont(fontType::mainFontBold);
			setFontSize(22);
			drawTextCenter(L"Trait",statusBase.x + 1 + 40, statusBase.y + 385 + 16);
			setFont(fontType::mainFontMedium);
			setFontSize(20);
			drawText(L"Strategist", statusBase.x + 88, statusBase.y + 388);
			setFont(fontType::mainFont);
			setFontSize(15);
			drawTextWidth(L" quick fox jumps over the lazy dog. quick fox jumps over the lazy dog. quick fox jumps over the lazy dog.", statusBase.x + 1 + 7, statusBase.y + 416 + 8, 288, 18);

			setFontSize(12);
			
			std::wstring mbtiText = PlayerPtr->entityInfo.mbti;
			drawText(mbtiText, statusBase.x + 291 - queryTextWidth(mbtiText), statusBase.y + 398);


			drawLine(statusBase.x + 872, statusBase.y + 35, statusBase.x + 872, statusBase.y + 490, { 0x63,0x63,0x63 });

			setFontSize(15);
			setFont(fontType::mainFontBold);
			drawTextCenter(L"Status Effects", statusBase.x + 933, statusBase.y + 49);

			setFontSize(14);
			setFont(fontType::mainFont);
			{
				constexpr int efctX = 876;
				constexpr int efctW = 113;
				constexpr int efctH = 26;
				constexpr int efctGap = 30;
				constexpr int efctMaxVisible = 14; // 표시 영역에 들어가는 최대 개수

				// 표시 대상 상태이상 필터링
				struct FilteredEffect { int metaIdx; };
				std::vector<FilteredEffect> filtered;
				for (const auto& eff : PlayerPtr->entityInfo.statusEffectVec)
				{
					int idx = (int)eff.effectType;
					if (idx < 0 || idx >= (int)statusEffectFlag::STATUS_EFFECT_COUNT) continue;
					if (eff.effectType == statusEffectFlag::hungry
						|| eff.effectType == statusEffectFlag::dehydrated
						|| eff.effectType == statusEffectFlag::tired) continue;
					const auto& meta = statusEffectMeta[idx];
					if (meta.name == nullptr) continue;
					filtered.push_back({ idx });
				}

				int totalCount = (int)filtered.size();

				// 스크롤 범위 클램프
				int maxScroll = std::max(0, totalCount - efctMaxVisible);
				statusScroll = std::clamp(statusScroll, 0, maxScroll);

				// 보이는 범위만 그리기
				int drawEnd = std::min(statusScroll + efctMaxVisible, totalCount);
				for (int i = statusScroll; i < drawEnd; i++)
				{
					const auto& meta = statusEffectMeta[filtered[i].metaIdx];
					int drawIdx = i - statusScroll;
					SDL_Rect efctRect = { statusBase.x + efctX, statusBase.y + 67 + efctGap * drawIdx, efctW, efctH };
					drawStadium(efctRect, stadiumCol(efctRect), 255, 4);
					drawSprite(spr::statusIcon, meta.iconIndex, efctRect.x + 4, efctRect.y + 4);
					drawText(col2Str(meta.color) + meta.name, efctRect.x + 30, efctRect.y + 3);
				}

				// 스크롤이 필요할 때만 스크롤바 표시
				if (totalCount > efctMaxVisible)
				{
					constexpr int scrollX = 990;
					constexpr int scrollY = 67;
					constexpr int scrollW = 2;
					constexpr int scrollH = 416;

					// 썸 높이: 보이는 비율에 비례, 최소 20px
					int thumbH = std::max(20, scrollH * efctMaxVisible / totalCount);
					int thumbTravel = scrollH - thumbH;
					int thumbY = (maxScroll > 0) ? scrollY + thumbTravel * statusScroll / maxScroll : scrollY;

					SDL_Rect scrollTrack = { statusBase.x + scrollX, statusBase.y + scrollY, scrollW, scrollH };
					SDL_Rect scrollThumb = { statusBase.x + scrollX, statusBase.y + thumbY, scrollW, thumbH };
					drawFillRect(scrollTrack, col::gray);
					//drawFillRect(scrollThumb, col::white);
				}
			}

			//스탯 박스 3종 (Str/Int/Dex) — 분배할 AP가 남아있으면 +버튼과 함께 그림
			float mouseXf, mouseYf;
			bool mouseDown = (SDL_GetMouseState(&mouseXf, &mouseYf) & SDL_BUTTON_LMASK) != 0;

			const wchar_t* statLabels[3] = { L"Str", L"Int", L"Dex" };
			const int statValues[3] = { PlayerPtr->entityInfo.statStr, PlayerPtr->entityInfo.statInt, PlayerPtr->entityInfo.statDex };
			for (int i = 0; i < 3; i++)
			{
				setFontSize(16);
				setFont(fontType::mainFontSemiBold);
				SDL_Rect statBtn = { statusBase.x + 625 + 83 * i,statusBase.y + 82,60,60 };
				SDL_Rect upBtn = { statBtn.x + 40, statBtn.y + 27, 22, 22 };
				drawStadium(statBtn, col::black, 255, 4);
				//호버 틴트는 라벨 뒤의 좁은 스타디움에만 — 박스 전체가 변하면 반응 범위가 너무 넓게 읽힘
				SDL_Rect labelRect = { statBtn.x + 3, statBtn.y + 3, 54, 19 };
				drawStadium(labelRect, stadiumCol(labelRect), 255, 4);
				drawTextCenter(statLabels[i], statBtn.x + statBtn.w / 2, statBtn.y + 12);
				setFontSize(24);
				setFont(fontType::mainFontBold);

				if (playerLevel::ap == 0)
				{
					drawTextCenter(std::to_wstring(statValues[i]), statBtn.x + statBtn.w / 2, statBtn.y + statBtn.h / 2 + 8);
				}
				else //분배할 AP가 남아있을 경우
				{
					drawTextCenter(std::to_wstring(statValues[i]), statBtn.x + statBtn.w / 2 - 10, statBtn.y + statBtn.h / 2 + 8);
					drawSprite(spr::statusAbilityUpBtn, (checkCursor(&upBtn) && mouseDown) ? 1 : 0, upBtn.x, upBtn.y); //0번 일반, 1번 클릭 (호버링없는 2인덱스짜리)
				}
			}




			setFont(fontType::mainFont);


			SDL_Rect gaugeRect = { statusBase.x + 670,statusBase.y + 48,184,23 };

			drawRect(gaugeRect, { 0x5b,0x5b,0x5b });
			drawRect(SDL_Rect{ gaugeRect.x - 1,gaugeRect.y,gaugeRect.w + 2,gaugeRect.h }, {0x5b,0x5b,0x5b});

			int curExp = playerLevel::exp;
			int needExp = playerLevel::expToNext();
			double expRatio = (needExp > 0) ? static_cast<double>(curExp) / needExp : 0.0;
			int gaugeW = static_cast<int>(176 * expRatio);

			SDL_Rect inGaugeRect = { gaugeRect.x+4,gaugeRect.y + 4,gaugeW,15 };
			SDL_Rect inGaugeRectIn = { gaugeRect.x + 4,gaugeRect.y + 5,gaugeW,10 };
			SDL_Rect inGaugeRectInIn = { gaugeRect.x + 4,gaugeRect.y + 7,gaugeW,5 };
			SDL_Rect inGaugeRectInInIn = { gaugeRect.x + 4,gaugeRect.y + 9,gaugeW,1 };


			drawFillRect(inGaugeRect, {0x85,0x3d,0x9c});
			drawFillRect(inGaugeRectIn, { 0x9e,0x51,0xb7 });
			drawFillRect(inGaugeRectInIn, { 0xb0,0x66,0xc8 });
			drawFillRect(inGaugeRectInInIn, { 0xc3,0x79,0xdb });


			setFontSize(18);
			setFont(fontType::mainFontMedium);
			drawText(L"Level " + std::to_wstring(playerLevel::level), gaugeRect.x - 80, gaugeRect.y + 1);

			setFontSize(16);
			setFont(fontType::mainFontSemiBold);
			drawText(L"AP : "+col2Str(lowCol::green)+std::to_wstring(playerLevel::ap), gaugeRect.x + gaugeRect.w - 54, gaugeRect.y + 102);


			std::wstring expStr = std::to_wstring(curExp) + L" / " + std::to_wstring(needExp);

			setFontSize(15);
			setFont(fontType::mainFontMedium);
			for (int i = 0; i < 8; i++)
			{
				int dx, dy;
				dir2Coord(i, dx, dy);
				drawTextCenter(col2Str(col::black) + expStr, gaugeRect.x + gaugeRect.w / 2 + dx, gaugeRect.y + gaugeRect.h / 2 + dy);
			}

			drawTextCenter(expStr, gaugeRect.x + gaugeRect.w/2, gaugeRect.y + gaugeRect.h/2);

			//분배를 1포인트라도 해서 초기화가 가능한 경우에만 초기화 버튼 표시
			//0 일반, 1 호버링, 2 클릭
			if (sessionAlloc[0] + sessionAlloc[1] + sessionAlloc[2] > 0)
			{
				SDL_Rect resetBtn = { statusBase.x + 579, statusBase.y + 98, 26, 26 };
				int resetIdx = checkCursor(&resetBtn) ? (mouseDown ? 2 : 1) : 0;
				drawSprite(spr::statusResetBtn, resetIdx, resetBtn.x, resetBtn.y);
			}

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



			//세로 구분선 (하단 실선 + 상단 페이드아웃)
			{
				int lx = statusBase.x + 445;
				int yBottom = statusBase.y + 490;
				int yFadeStart = statusBase.y + 490 - 275;
				int yTop = statusBase.y + 490 - 310;
				SDL_Color lineCol = { 0x63, 0x63, 0x63 };

				//하단 실선 구간
				drawLine(lx, yBottom, lx, yFadeStart, lineCol);

				//상단 페이드아웃 구간
				int fadeLen = yFadeStart - yTop;
				for (int py = yFadeStart; py >= yTop; py--)
				{
					Uint8 a = (Uint8)(255.0 * (py - yTop) / fadeLen);
					drawPoint(lx, py, lineCol, a);
				}
			}

			setZoom(2.0);
			drawSprite(spr::icon16, 116, statusBase.x + 305, statusBase.y + 190);
			setZoom(1.0);
			setFont(fontType::mainFontSemiBold);
			setFontSize(18);
			drawText(L"Bionics", statusBase.x + 305 + 38, statusBase.y + 190 + 4);
			
			{
				constexpr int bionicMaxVisible = 13; // 252 / 19

				// 바이오닉 필터링 (skillSrc::BIONIC)
				std::vector<const SkillData*> bionicEntries;
				for (const auto& sd : PlayerPtr->entityInfo.skillList)
				{
					SkillBehavior* bhv = SkillRegistry::get(sd.skillId);
					if (bhv && bhv->src == skillSrc::BIONIC)
						bionicEntries.push_back(&sd);
				}

				int bionicTotal = (int)bionicEntries.size();

				SDL_Rect bionicRect = { statusBase.x + 302,statusBase.y + 229,136,252 };
				bool bionicNeedScroll = bionicTotal > bionicMaxVisible;

				//스크롤이 필요한 경우 박스를 살짝 좁혀 스크롤바 공간 확보
				if (bionicNeedScroll)
					bionicRect.x -= 3;

				drawStadium(bionicRect, col::black, 255, 4);

				if (bionicTotal == 0)
				{
					setFontSize(18);
					setFont(fontType::mainFontSemiBold);
					drawTextCenter(col2Str(col::gray) + L"No Data", bionicRect.x + bionicRect.w / 2, bionicRect.y + bionicRect.h / 2 - 26);
				}
				else
				{
					// 스크롤 범위 클램프
					int bionicMaxScroll = std::max(0, bionicTotal - bionicMaxVisible);
					bionicScroll = std::clamp(bionicScroll, 0, bionicMaxScroll);

					int drawEnd = std::min(bionicScroll + bionicMaxVisible, bionicTotal);
					setFont(fontType::mainFont);
					setFontSize(12);
					for (int i = bionicScroll; i < drawEnd; i++)
					{
						int drawIdx = i - bionicScroll;
						SkillBehavior* bhv = SkillRegistry::get(bionicEntries[i]->skillId);
						if (!bhv) continue;

						SDL_Rect eachBionicRect = { bionicRect.x + 5, bionicRect.y + 4 + 19 * drawIdx , 128, 16 };
						drawStadium(eachBionicRect, stadiumCol(eachBionicRect), 255, 4);

						drawSprite(spr::icon16, 116, bionicRect.x + 5, bionicRect.y + 4 + 19 * drawIdx);
						std::wstring bionicLabel = bhv->name;
						if (bionicEntries[i]->skillLevel > 1)
							bionicLabel += L"(" + std::to_wstring(bionicEntries[i]->skillLevel) + L")";
						drawText(bionicLabel, bionicRect.x + 5 + 20, bionicRect.y + 4 + 19 * drawIdx);
					}

					// 스크롤바
					if (bionicNeedScroll)
					{
						SDL_Rect scrollTrack = { bionicRect.x + 139, bionicRect.y, 2, 252 };
						int thumbH = std::max(20, 252 * bionicMaxVisible / bionicTotal);
						int thumbTravel = 252 - thumbH;
						int thumbY = (bionicMaxScroll > 0) ? scrollTrack.y + thumbTravel * bionicScroll / bionicMaxScroll : scrollTrack.y;
						SDL_Rect scrollThumb = { scrollTrack.x, thumbY, 2, thumbH };
						drawFillRect(scrollTrack, col::gray);
						drawFillRect(scrollThumb, col::white);
					}
				}
			}


			//세로 구분선 (하단 실선 + 상단 페이드아웃)
			{
				int lx = statusBase.x + 721;
				int yBottom = statusBase.y + 490;
				int yFadeStart = statusBase.y + 490 - 275;
				int yTop = statusBase.y + 490 - 310;
				SDL_Color lineCol = { 0x63, 0x63, 0x63 };

				//하단 실선 구간
				drawLine(lx, yBottom, lx, yFadeStart, lineCol);

				//상단 페이드아웃 구간
				int fadeLen = yFadeStart - yTop;
				for (int py = yFadeStart; py >= yTop; py--)
				{
					Uint8 a = (Uint8)(255.0 * (py - yTop) / fadeLen);
					drawPoint(lx, py, lineCol, a);
				}
			}


			setZoom(2.0);
			drawSprite(spr::icon16, 117, statusBase.x + 731, statusBase.y + 190);
			setZoom(1.0);
			setFont(fontType::mainFontSemiBold);
			setFontSize(18);
			drawText(L"Mutations", statusBase.x + 731 + 38, statusBase.y + 190 + 4);

			{
				constexpr int mutMaxVisible = 13; // 252 / 19

				// 돌연변이 필터링
				std::vector<const SkillData*> mutList;
				for (const auto& sd : PlayerPtr->entityInfo.skillList)
				{
					SkillBehavior* bhv = SkillRegistry::get(sd.skillId);
					if (bhv && bhv->src == skillSrc::MUTATION)
						mutList.push_back(&sd);
				}
				int mutTotal = (int)mutList.size();

				SDL_Rect mutationRect = { statusBase.x + 729,statusBase.y + 229,136,252 };
				bool mutNeedScroll = mutTotal > mutMaxVisible;

				if (mutNeedScroll)
					mutationRect.x -= 3;

				drawStadium(mutationRect, col::black, 255, 4);

				if (mutTotal == 0)
				{
					setFontSize(18);
					setFont(fontType::mainFontSemiBold);
					drawTextCenter(col2Str(col::gray) + L"No Data", mutationRect.x + mutationRect.w / 2, mutationRect.y + mutationRect.h / 2 - 26);
				}
				else
				{
					// 스크롤 범위 클램프
					int mutMaxScroll = std::max(0, mutTotal - mutMaxVisible);
					mutationScroll = std::clamp(mutationScroll, 0, mutMaxScroll);

					int drawEnd = std::min(mutationScroll + mutMaxVisible, mutTotal);
					setFont(fontType::mainFont);
					setFontSize(12);
					for (int i = mutationScroll; i < drawEnd; i++)
					{
						int drawIdx = i - mutationScroll;
						SkillBehavior* bhv = SkillRegistry::get(mutList[i]->skillId);
						if (!bhv) continue;

						SDL_Rect eachMutationRect = { mutationRect.x + 5, mutationRect.y + 4 + 19 * drawIdx , 128, 16 };
						drawStadium(eachMutationRect, stadiumCol(eachMutationRect), 255, 4);

						drawSprite(spr::icon16, 117, mutationRect.x + 5, mutationRect.y + 4 + 19 * drawIdx);
						drawText(bhv->name, mutationRect.x + 5 + 20, mutationRect.y + 4 + 19 * drawIdx);

					}

					// 스크롤바
					if (mutNeedScroll)
					{
						SDL_Rect scrollTrack = { mutationRect.x + 139, mutationRect.y, 2, 252 };
						int thumbH = std::max(20, 252 * mutMaxVisible / mutTotal);
						int thumbTravel = 252 - thumbH;
						int thumbY = (mutMaxScroll > 0) ? scrollTrack.y + thumbTravel * mutationScroll / mutMaxScroll : scrollTrack.y;
						SDL_Rect scrollThumb = { scrollTrack.x, thumbY, 2, thumbH };
						drawFillRect(scrollTrack, col::gray);
						drawFillRect(scrollThumb, col::white);
					}
				}
			}

			auto drawBlankGauge = [](int x, int y)
				{
					drawPoint(x + 1, y + 1, col::white);
					drawLine(x + 2, y, x + 83, y, col::white);
					drawPoint(x + 84, y + 1, col::white);
					drawLine(x + 85, y + 2, x + 85, y + 8, col::white);
					drawPoint(x + 84, y + 9, col::white);
					drawLine(x + 83, y + 10, x + 2, y + 10, col::white);
					drawPoint(x + 1, y + 9, col::white);
					drawLine(x, y + 8, x, y + 2, col::white);
				};

			// 부위별 HP 박스 그리기 람다
			auto drawPartBox = [&](int px, int py, const wchar_t* partName, int curHP, int maxHP)
			{
				SDL_Rect partRect = { statusBase.x + px, statusBase.y + py, 90, 48 };
				drawStadium(partRect, stadiumCol(partRect), 255, 4);
				setFont(fontType::mainFontBold);
				setFontSize(14);
				drawText(partName, partRect.x + 4, partRect.y + 1);
				drawBlankGauge(partRect.x + 2, partRect.y + 20);

				float hpRatio = (maxHP > 0) ? std::clamp((float)curHP / (float)maxHP, 0.0f, 1.0f) : 0.0f;
				SDL_Color gaugeCol = { 0x5b, 0xbf, 0x75 };
				if (hpRatio <= 0.25f) gaugeCol = { 0xff, 0x44, 0x44 };
				else if (hpRatio <= 0.5f) gaugeCol = { 0xff, 0xc1, 0x07 };
				int fillW = (int)(80 * hpRatio);
				drawFillRect(SDL_Rect{ partRect.x + 5, partRect.y + 23, fillW, 5 }, gaugeCol);

				std::wstring hpText = std::to_wstring(curHP) + L" / " + std::to_wstring(maxHP);
				setFont(fontType::mainFont);
				setFontSize(12);
				drawTextCenter(hpText, partRect.x + 59, partRect.y + 38);
			};

			drawPartBox(467, 209, L"Torso", PlayerPtr->entityInfo.HP, PlayerPtr->entityInfo.maxHP);
			drawPartBox(608, 194, L"Head", PlayerPtr->headHP, PART_MAX_HP);
			drawPartBox(451, 287, L"R.Arm", PlayerPtr->rArmHP, PART_MAX_HP);
			drawPartBox(465, 395, L"R.Leg", PlayerPtr->rLegHP, PART_MAX_HP);
			drawPartBox(625, 287, L"L.Arm", PlayerPtr->lArmHP, PART_MAX_HP);
			drawPartBox(610, 393, L"L.Leg", PlayerPtr->lLegHP, PART_MAX_HP);

			setFont(fontType::mainFont);

			
			SDL_SetTextureAlphaMod(spr::bodyShape->getTexture(), 70);
			drawSpriteCenter(spr::bodyShape, 0, statusBase.x + 583, statusBase.y + 340);
			SDL_SetTextureAlphaMod(spr::bodyShape->getTexture(), 255);

		}
		else
		{
			SDL_Rect vRect = statusBase;
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
