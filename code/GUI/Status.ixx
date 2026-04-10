module;
#include <SDL3/SDL.h>

export module Status;

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

export class Status : public GUI
{
private:
	inline static Status* ptr = nullptr;
	SDL_Rect statusBase;
	int statusCursor = -1;
	int statusScroll = 0;
	int bionicScroll = 0;
	int mutationScroll = 0;
	int partScroll[6] = { 0, }; // 부위별 바이오닉/돌연변이 스크롤
public:
	Status() : GUI(false)
	{
		//1개 이상의 Status 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one Status instance was generated.");
		ptr = this;

		changeXY(cameraW / 2, cameraH / 2, true);

		deactInput();
		deactDraw();
		addAniToPlayerTurn(this, aniFlag::winUnfoldOpen);
	}
	~Status()
	{
		ptr = nullptr;
	}
	static Status* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		statusBase = { 0, 0, 993, 492 };

		if (center == false)
		{
			statusBase.x += inputX;
			statusBase.y += inputY;
		}
		else
		{
			statusBase.x += inputX - statusBase.w / 2;
			statusBase.y += inputY - statusBase.h / 2;
		}

		if (statusBase.x < 87) { statusBase.x = 87; }
		if (statusBase.y >= 275) { statusBase.y = 274; }

		x = statusBase.x;
		y = statusBase.y;
	}
	void drawGUI()
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

			drawFillRect(SDL_Rect{ statusBase.x + 12,statusBase.y + 46, 118, 110 }, col::black);
			drawRect(SDL_Rect{ statusBase.x+12,statusBase.y + 46, 118, 110 }, col::white);



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



			//육각형 스테이터스 레이더 차트 (flat-top)
			{
				int cx = statusBase.x + 151;
				int cy = statusBase.y + 271;
				int r = 44;
				double hexAngle[6];
				for (int i = 0; i < 6; i++)
					hexAngle[i] = std::numbers::pi / 180.0 * (60.0 * i);

				// ratio 값 (0.0~1.0): 각 꼭짓점의 실제 데이터 비율
				// i=0: Bionic Capacity, i=1: Progress, i=2: Profic Avg
				// i=3: Status Avg, i=4: Locked, i=5: Mutation Threshold
				double proficSum = 0.0;
				for (int i = 0; i < TALENT_SIZE; i++)
					proficSum += PlayerPtr->getProficLevel(i);
				double statAvgVal = (PlayerPtr->entityInfo.statStr
					+ PlayerPtr->entityInfo.statInt
					+ PlayerPtr->entityInfo.statDex) / 3.0;

				double ratio[6] = {
					0.9,                                                       // Bionic Capacity (미구현)
					0.6,                                                       // Progress (미구현)
					std::clamp(proficSum / TALENT_SIZE / MAX_PROFIC_LEVEL, 0.0, 1.0), // Profic Avg
					std::clamp(statAvgVal / 10.0, 0.0, 1.0),                  // Status Avg
					0.0,                                                       // Locked
					0.9,                                                       // Mutation Threshold (미구현)
				};

				//삼각형 팬으로 채워진 육각형을 그리는 람다
				auto drawFilledHex = [&](float* vx, float* vy, SDL_Color c, Uint8 a)
					{
						SDL_Vertex vertices[8];
						int indices[18];

						//중심 정점
						vertices[0].position = { (float)cx, (float)cy };
						vertices[0].color = { c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, a / 255.0f };
						for (int i = 0; i < 6; i++)
						{
							vertices[i + 1].position = { vx[i], vy[i] };
							vertices[i + 1].color = vertices[0].color;
						}
						//삼각형 팬 인덱스
						for (int i = 0; i < 6; i++)
						{
							indices[i * 3] = 0;
							indices[i * 3 + 1] = i + 1;
							indices[i * 3 + 2] = (i + 1) % 6 + 1;
						}
						SDL_RenderGeometry(renderer, nullptr, vertices, 7, indices, 18);
					};

				//1) 검은 배경 육각형
				float bx[6], by[6];
				for (int i = 0; i < 6; i++)
				{
					bx[i] = cx + (float)(r * std::cos(hexAngle[i]));
					by[i] = cy + (float)(r * std::sin(hexAngle[i]));
				}
				drawFilledHex(bx, by, { 0, 0, 0 }, 255);

				//2) 스탯 레이더 (ratio 반영, 최소 비율 보장)
				// 0.0→minR, 1.0→1.0 으로 선형 보간하여 죽는 구간 없이 매핑
				constexpr double minR = 0.12;
				float sx[6], sy[6];
				for (int i = 0; i < 6; i++)
				{
					double mapped = minR + ratio[i] * (1.0 - minR);
					sx[i] = cx + (float)(r * mapped * std::cos(hexAngle[i]));
					sy[i] = cy + (float)(r * mapped * std::sin(hexAngle[i]));
				}
				drawFilledHex(sx, sy, { 0x44, 0xaa, 0xff }, 160);

				//3) 스탯 레이더 테두리
				for (int i = 0; i < 6; i++)
					drawLine((int)sx[i], (int)sy[i], (int)sx[(i + 1) % 6], (int)sy[(i + 1) % 6], SDL_Color{ 0x44, 0xaa, 0xff }, 220);

				//4) 흰색 외곽 테두리
				for (int i = 0; i < 6; i++)
					drawLine((int)bx[i], (int)by[i], (int)bx[(i + 1) % 6], (int)by[(i + 1) % 6], col::white);
			}

			//vertex 버튼 타이틀+값 그리기 람다
			auto drawVertexBtn = [](const SDL_Rect& btn, const std::wstring& title, const std::wstring& value, SDL_Color bgCol = col::black)
			{
				drawStadium(btn, bgCol, 150, 4);
				int cx = btn.x + btn.w / 2;

				setFontSize(12);
				setFont(fontType::mainFontMedium);
				if (queryTextWidth(removeColorCodes(title)) > btn.w)
				{
					auto split = wordSplitter(title, btn.w);
					// 컬러코드가 첫 줄에만 있으면 둘째 줄에도 적용
					std::wstring colorPrefix;
					if (split[0].size() >= 7 && split[0][0] == L'#')
						colorPrefix = split[0].substr(0, 7);
					if (!split[1].empty() && split[1][0] != L'#' && !colorPrefix.empty())
						split[1] = colorPrefix + split[1];
					drawTextCenter(split[0], cx, btn.y + 6);
					drawTextCenter(split[1], cx, btn.y + 6 + 12);
				}
				else
				{
					drawTextCenter(title, cx, btn.y + 12);
				}

				setFont(fontType::mainFontBold);
				setFontSize(18);
				drawTextCenter(value, cx, btn.y + 38);
				setFont(fontType::mainFont);
			};

			SDL_Rect vertex1Btn = { statusBase.x + 181,statusBase.y + 176,70,56 };
			drawVertexBtn(vertex1Btn, L"#e1772eMutation Threshold", L"72%", stadiumCol(vertex1Btn));

			SDL_Rect vertex2Btn = { statusBase.x + 208,statusBase.y + 244,70,56 };
			drawVertexBtn(vertex2Btn, L"#e1772eBionic Capacity", L"6 / 10", stadiumCol(vertex2Btn));

			SDL_Rect vertex3Btn = { statusBase.x + 181,statusBase.y + 312,70,56 };
			drawVertexBtn(vertex3Btn, L"#e1772eProgress", L"20%", stadiumCol(vertex3Btn));

			SDL_Rect vertex4Btn = { statusBase.x + 52,statusBase.y + 312,70,56 };
			{
				// Profic Avg: 전체 숙련도 레벨 평균 (최대 27)
				double proficSum = 0.0;
				for (int i = 0; i < TALENT_SIZE; i++)
					proficSum += PlayerPtr->getProficLevel(i);
				double proficAvg = proficSum / TALENT_SIZE;
				std::wstringstream wss;
				wss << std::fixed << std::setprecision(1) << proficAvg;
				drawVertexBtn(vertex4Btn, L"#e1772eProfic Avg", wss.str(), stadiumCol(vertex4Btn));
			}

			SDL_Rect vertex5Btn = { statusBase.x + 25,statusBase.y + 244,70,56 };
			{
				// Status Avg: STR/INT/DEX 평균 (최대 10)
				double statAvg = (PlayerPtr->entityInfo.statStr
					+ PlayerPtr->entityInfo.statInt
					+ PlayerPtr->entityInfo.statDex) / 3.0;
				std::wstringstream wss;
				wss << std::fixed << std::setprecision(1) << statAvg;
				drawVertexBtn(vertex5Btn, L"#e1772eStatus Avg", wss.str(), stadiumCol(vertex5Btn));
			}

			SDL_Rect vertex6Btn = { statusBase.x + 52,statusBase.y + 176,70,56 };
			drawStadium(vertex6Btn, col::black, 150, 4);
			setFontSize(14);
			setFont(fontType::mainFontMedium);
			drawTextCenter(col2Str(col::gray) + L"Locked", vertex6Btn.x + vertex6Btn.w / 2, vertex6Btn.y + vertex6Btn.h / 2);
			setFont(fontType::mainFont);

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

			setFontSize(16);
			setFont(fontType::mainFontSemiBold);
			SDL_Rect strBtn = { statusBase.x + 625,statusBase.y + 96,60,60 };
			drawStadium(strBtn, stadiumCol(strBtn), 255, 4);
			drawTextCenter(L"Str", strBtn.x + strBtn.w/2, strBtn.y + 12);
			setFontSize(24);
			setFont(fontType::mainFontBold);
			drawTextCenter(std::to_wstring(PlayerPtr->entityInfo.statStr), strBtn.x + strBtn.w / 2, strBtn.y + strBtn.h / 2 + 8);


			setFontSize(16);
			setFont(fontType::mainFontSemiBold);
			SDL_Rect intBtn = { statusBase.x + 625 + 83*1,statusBase.y + 96,60,60 };
			drawStadium(intBtn, stadiumCol(intBtn), 255, 4);
			drawTextCenter(L"Int", intBtn.x + intBtn.w / 2, intBtn.y + 12);
			setFontSize(24);
			setFont(fontType::mainFontBold);
			drawTextCenter(std::to_wstring(PlayerPtr->entityInfo.statInt), intBtn.x + intBtn.w / 2, intBtn.y + intBtn.h / 2 + 8);

			setFontSize(16);
			setFont(fontType::mainFontSemiBold);
			SDL_Rect dexBtn = { statusBase.x + 625 + 83 * 2,statusBase.y + 96,60,60 };
			drawStadium(dexBtn, stadiumCol(dexBtn), 255, 4);
			drawTextCenter(L"Dex", dexBtn.x + dexBtn.w / 2, dexBtn.y + 12);
			setFontSize(24);
			setFont(fontType::mainFontBold);
			drawTextCenter(std::to_wstring(PlayerPtr->entityInfo.statDex), dexBtn.x + dexBtn.w / 2, dexBtn.y + dexBtn.h / 2 + 8);


			setFont(fontType::mainFont);


			SDL_Rect energyIcon = { statusBase.x + 624,statusBase.y + 48,32,32 };
			drawStadium(energyIcon, stadiumCol(energyIcon), 255, 4);
			setZoom(2.0);
			drawSprite(spr::icon16, 118, energyIcon.x, energyIcon.y);
			setZoom(1.0);

			SDL_Rect gaugeRect = { statusBase.x + 662,statusBase.y + 53,184,23 };

			drawRect(gaugeRect, col::white);
			drawRect(SDL_Rect{ gaugeRect.x-1,gaugeRect.y,gaugeRect.w+2,gaugeRect.h }, col::white);
			drawFillRect(SDL_Rect{ statusBase.x + 847,statusBase.y + 60,5,9 }, col::white);

			SDL_Rect inGaugeRect = { statusBase.x + 666,statusBase.y + 57,134,15 };
			drawFillRect(inGaugeRect, {0x5b,0xbf,0x75});
			

			setFontSize(15);
			setFont(fontType::mainFontMedium);
			for (int i = 0; i < 8; i++)
			{
				int dx, dy;
				dir2Coord(i, dx, dy);
				drawTextCenter(col2Str(col::black) + L"123 / 500 kJ", gaugeRect.x + gaugeRect.w / 2 + dx, gaugeRect.y + gaugeRect.h / 2 + dy);
			}

			drawTextCenter(L"123 / 500 kJ", gaugeRect.x + gaugeRect.w/2, gaugeRect.y + gaugeRect.h/2);


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
					SkillBehavior* bhv = SkillRegistry::get(sd.skillCode);
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
						SkillBehavior* bhv = SkillRegistry::get(bionicEntries[i]->skillCode);
						if (!bhv) continue;

						SDL_Rect eachBionicRect = { bionicRect.x + 5, bionicRect.y + 4 + 19 * drawIdx , 128, 16 };
						drawStadium(eachBionicRect, stadiumCol(eachBionicRect), 255, 4);

						drawSprite(spr::icon16, 116, bionicRect.x + 5, bionicRect.y + 4 + 19 * drawIdx);
						drawText(bhv->name, bionicRect.x + 5 + 20, bionicRect.y + 4 + 19 * drawIdx);
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
					SkillBehavior* bhv = SkillRegistry::get(sd.skillCode);
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
						SkillBehavior* bhv = SkillRegistry::get(mutList[i]->skillCode);
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
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
		}
		else
		{
		}
	}

	void mouseWheel()
	{
		// 상태이상 패널 영역 위에서만 스크롤 반응
		SDL_Rect efctArea = { statusBase.x + 872, statusBase.y + 35, 121, 455 };
		if (checkCursor(&efctArea))
		{
			if (event.wheel.y > 0 && statusScroll > 0) statusScroll--;
			else if (event.wheel.y < 0) statusScroll++;
			return;
		}

		// 바이오닉 박스 스크롤
		SDL_Rect bionicArea = { statusBase.x + 299, statusBase.y + 229, 142, 252 };
		if (checkCursor(&bionicArea))
		{
			if (event.wheel.y > 0 && bionicScroll > 0) bionicScroll--;
			else if (event.wheel.y < 0) bionicScroll++;
			return;
		}

		// 돌연변이 박스 스크롤
		SDL_Rect mutationArea = { statusBase.x + 726, statusBase.y + 229, 142, 252 };
		if (checkCursor(&mutationArea))
		{
			if (event.wheel.y > 0 && mutationScroll > 0) mutationScroll--;
			else if (event.wheel.y < 0) mutationScroll++;
			return;
		}

		// 바디파트 스크롤: 각 파트 영역 위에서 휠 반응
		struct PartArea { SDL_Rect rect; humanPartFlag part; };
		PartArea parts[] = {
			{ { statusBase.x + 298, statusBase.y + 206, 236, 83 }, humanPartFlag::head },
			{ { statusBase.x + 298, statusBase.y + 206 + 91, 236, 83 }, humanPartFlag::rArm },
			{ { statusBase.x + 298, statusBase.y + 206 + 91 * 2, 236, 83 }, humanPartFlag::rLeg },
			{ { statusBase.x + 298 + 335, statusBase.y + 206, 236, 83 }, humanPartFlag::torso },
			{ { statusBase.x + 298 + 335, statusBase.y + 206 + 91, 236, 83 }, humanPartFlag::lArm },
			{ { statusBase.x + 298 + 335, statusBase.y + 206 + 91 * 2, 236, 83 }, humanPartFlag::lLeg },
		};
		for (auto& p : parts)
		{
			if (checkCursor(&p.rect))
			{
				int& s = partScroll[(int)p.part];
				if (event.wheel.y > 0 && s > 0) s--;
				else if (event.wheel.y < 0) s++;
				return;
			}
		}
	}

	void step()
	{
		tabType = tabFlag::back;
	}
};
