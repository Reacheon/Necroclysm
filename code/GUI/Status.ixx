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

export class Status : public GUI
{
private:
	inline static Status* ptr = nullptr;
	SDL_Rect statusBase;
	int statusCursor = -1;
	int statusScroll = 0;
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

			drawFillRect(SDL_Rect{ statusBase.x + 12,statusBase.y + 46, 118, 110 }, col::black);
			drawRect(SDL_Rect{ statusBase.x+12,statusBase.y + 46, 118, 110 }, col::white);



			setFontSize(24);
			setFont(fontType::mainFontBold);
			//drawText(col2Str(SDL_Color{ 0xff,0xd3,0x44 }) + L"Nekbung (Loop #7)", statusBase.x + 139, statusBase.y + 44);
			drawText(col2Str(SDL_Color{ 0xff,0xd3,0x44 }) + L"Nekbung, Survivor", statusBase.x + 139, statusBase.y + 44);
			setFont(fontType::mainFont);

			setFontSize(16);
			setFont(fontType::mainFontBold);
			//좌측 열: Age, Race, God
			SDL_Rect ageRect = { statusBase.x + 137,statusBase.y + 79,151,23 };
			drawStadium(ageRect, col::black, 255, 4);
			drawText(L"Age", ageRect.x + 5, ageRect.y + 1);

			SDL_Rect raceRect = { statusBase.x + 137,statusBase.y + 79 + 27 * 1,151,23 };
			drawStadium(raceRect, col::black, 255, 4);
			drawText(L"Race", raceRect.x + 5, raceRect.y + 1);

			SDL_Rect godRect = { statusBase.x + 137,statusBase.y + 79 + 27 * 2,151,23 };
			drawStadium(godRect, col::black, 255, 4);
			drawText(L"God", godRect.x + 5, godRect.y + 1);

			//우측 열: Hunger, Thirsty, Fatigue
			SDL_Rect hungerRect = { statusBase.x + 137 + 221,statusBase.y + 79,151,23 };
			drawStadium(hungerRect, col::black, 255, 4);
			drawText(L"Hunger", hungerRect.x + 5, hungerRect.y + 1);

			SDL_Rect thirstyRect = { statusBase.x + 137 + 221,statusBase.y + 79 + 27 * 1,151,23 };
			drawStadium(thirstyRect, col::black, 255, 4);
			drawText(L"Thirsty", thirstyRect.x + 5, thirstyRect.y + 1);

			SDL_Rect fatigueRect = { statusBase.x + 137 + 221,statusBase.y + 79 + 27 * 2,151,23 };
			drawStadium(fatigueRect, col::black, 255, 4);
			drawText(L"Fatigue", fatigueRect.x + 5, fatigueRect.y + 1);
			setFont(fontType::mainFont);



			//육각형 스테이터스 레이더 차트 (flat-top)
			{
				int cx = statusBase.x + 151;
				int cy = statusBase.y + 271;
				int r = 44;
				double hexAngle[6];
				for (int i = 0; i < 6; i++)
					hexAngle[i] = std::numbers::pi / 180.0 * (60.0 * i);

				//임의의 ratio 값 (0.0~1.0)
				double ratio[6] = { 0.9, 0.6, 0.75, 0.4, 0.85, 0.5 };

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

				//2) 스탯 레이더 (ratio 반영)
				float sx[6], sy[6];
				for (int i = 0; i < 6; i++)
				{
					sx[i] = cx + (float)(r * ratio[i] * std::cos(hexAngle[i]));
					sy[i] = cy + (float)(r * ratio[i] * std::sin(hexAngle[i]));
				}
				drawFilledHex(sx, sy, { 0x44, 0xaa, 0xff }, 160);

				//3) 스탯 레이더 테두리
				for (int i = 0; i < 6; i++)
					drawLine((int)sx[i], (int)sy[i], (int)sx[(i + 1) % 6], (int)sy[(i + 1) % 6], SDL_Color{ 0x44, 0xaa, 0xff }, 220);

				//4) 흰색 외곽 테두리
				for (int i = 0; i < 6; i++)
					drawLine((int)bx[i], (int)by[i], (int)bx[(i + 1) % 6], (int)by[(i + 1) % 6], col::white);
			}

			SDL_Rect vertex1Btn = { statusBase.x+ 181,statusBase.y+176,70,56 };
			drawStadium(vertex1Btn, col::black, 150, 4);

			SDL_Rect vertex2Btn = { statusBase.x + 208,statusBase.y + 244,70,56 };
			drawStadium(vertex2Btn, col::black, 150, 4);

			SDL_Rect vertex3Btn = { statusBase.x + 181,statusBase.y + 312,70,56 };
			drawStadium(vertex3Btn, col::black, 150, 4);

			SDL_Rect vertex4Btn = { statusBase.x + 52,statusBase.y + 312,70,56 };
			drawStadium(vertex4Btn, col::black, 150, 4);

			SDL_Rect vertex5Btn = { statusBase.x + 25,statusBase.y + 244,70,56 };
			drawStadium(vertex5Btn, col::black, 150, 4);

			SDL_Rect vertex6Btn = { statusBase.x + 52,statusBase.y + 176,70,56 };
			drawStadium(vertex6Btn, col::black, 150, 4);

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

			setFontSize(12);
			
			std::wstring mbtiText = L"INTJ";
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

				//예시 상태이상 목록 (아이콘 인덱스, 이름, 색상)
				struct EffectEntry { int icon; const wchar_t* name; SDL_Color col; };
				EffectEntry effects[] = {
					{ 56, L"Hungry",     { 0xff, 0xc1, 0x07 } },
					{ 53, L"Thirsty",    { 0x44, 0xaa, 0xff } },
					{ 58, L"Tired",      { 0xaa, 0xaa, 0xaa } },
				};

				for (int i = 0; i < 3; i++)
				{
					SDL_Rect efctRect = { statusBase.x + efctX, statusBase.y + 67 + efctGap * i, efctW, efctH };
					drawStadium(efctRect, col::black, 255, 4);
					drawSprite(spr::statusIcon, effects[i].icon, efctRect.x + 4, efctRect.y + 4);
					drawText(col2Str(effects[i].col) + effects[i].name, efctRect.x + 30, efctRect.y + 3);
				}
			}

			setFontSize(16);
			setFont(fontType::mainFontSemiBold);
			SDL_Rect strBtn = { statusBase.x + 625,statusBase.y + 96,60,60 };
			drawStadium(strBtn, col::black, 255, 4);
			drawTextCenter(L"Str", strBtn.x + strBtn.w/2, strBtn.y + 12);
			setFontSize(24);
			setFont(fontType::mainFontBold);
			drawTextCenter(L"4", strBtn.x + strBtn.w / 2, strBtn.y + strBtn.h / 2 + 8);


			setFontSize(16);
			setFont(fontType::mainFontSemiBold);
			SDL_Rect intBtn = { statusBase.x + 625 + 83*1,statusBase.y + 96,60,60 };
			drawStadium(intBtn, col::black, 255, 4);
			drawTextCenter(L"Int", intBtn.x + intBtn.w / 2, intBtn.y + 12);
			setFontSize(24);
			setFont(fontType::mainFontBold);
			drawTextCenter(L"4", intBtn.x + intBtn.w / 2, intBtn.y + intBtn.h / 2 + 8);

			setFontSize(16);
			setFont(fontType::mainFontSemiBold);
			SDL_Rect dexBtn = { statusBase.x + 625 + 83 * 2,statusBase.y + 96,60,60 };
			drawStadium(dexBtn, col::black, 255, 4);
			drawTextCenter(L"Dex", dexBtn.x + dexBtn.w / 2, dexBtn.y + 12);
			setFontSize(24);
			setFont(fontType::mainFontBold);
			drawTextCenter(L"3", dexBtn.x + dexBtn.w / 2, dexBtn.y + dexBtn.h / 2 + 8);


			setFont(fontType::mainFont);


			SDL_Rect energyIcon = { statusBase.x + 624,statusBase.y + 48,32,32 };
			drawStadium(energyIcon, col::black, 255, 4);
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





			//좌측 열: Head, R.Arm, R.Leg
			{
				SDL_Rect headRect = { statusBase.x + 298,statusBase.y + 206,236,83 };
				drawStadium(headRect, col::black, 150, 4);
				setFontSize(18);
				setFont(fontType::mainFontSemiBold);
				drawText(L"Head", headRect.x + 6, headRect.y - 2);

				SDL_Rect headGaugeRect = { headRect.x + 109, headRect.y + 3, 122, 15 };
				SDL_Rect headInGaugeRect = { headRect.x + 112, headRect.y + 6, 116, 9 };
				drawRect(headGaugeRect, col::white);
				drawFillRect(headInGaugeRect, { 0x5b,0xbf,0x75 });

				setFont(fontType::mainFont);
				setFontSize(12);
				for (int i = 0; i < 8; i++)
				{
					int dx, dy;
					dir2Coord(i, dx, dy);
					drawTextCenter(col2Str(col::black)+L"100 / 100"
						, headInGaugeRect.x + headInGaugeRect.w / 2 + dx
						, headInGaugeRect.y + headInGaugeRect.h / 2 + dy);

				}

				drawTextCenter(L"100 / 100", headInGaugeRect.x+ headInGaugeRect.w/2, headInGaugeRect.y + headInGaugeRect.h / 2);
			
				//구분선 (오른쪽으로 페이드아웃)
				for (int px = 0; px <= 98; px++)
				{
					Uint8 a = (Uint8)(255.0 * (1.0 - (double)px / 98.0));
					drawPoint(headRect.x + 3 + px, headRect.y + 21, col::gray, a);
				}

				
				drawSprite(spr::icon16, 116, headRect.x + 3, headRect.y + 26);
				setFontSize(12);
				drawText(L"Nerve Boost", headRect.x + 3 + 19, headRect.y + 26);

				drawSprite(spr::icon16, 116, headRect.x + 3, headRect.y + 26 + 19);
				setFontSize(12);
				drawText(L"Nerve Boost", headRect.x + 3 + 19, headRect.y + 26 + 19);

				drawSprite(spr::icon16, 116, headRect.x + 3, headRect.y + 26 + 38);
				setFontSize(12);
				drawText(L"Nerve Boost", headRect.x + 3 + 19, headRect.y + 26 + 38);


				drawSprite(spr::icon16, 117, headRect.x + 3 + 118, headRect.y + 26);
				setFontSize(12);
				drawText(L"Spore Emitter", headRect.x + 3 + 19 + 118, headRect.y + 26);

				drawSprite(spr::icon16, 117, headRect.x + 3 + 118, headRect.y + 26 + 19);
				setFontSize(12);
				drawText(L"Spore Emitter", headRect.x + 3 + 19 + 118, headRect.y + 26 + 19);

			}


			SDL_Rect rArmRect = { statusBase.x + 298,statusBase.y + 206 + 91*1,236,83 };
			drawStadium(rArmRect, col::black, 150, 4);
			setFontSize(18);
			setFont(fontType::mainFontSemiBold);
			drawText(L"Right Arm", rArmRect.x + 6, rArmRect.y-2);

			setFontSize(18);
			setFont(fontType::mainFontSemiBold);
			drawTextCenter(col2Str(col::gray)+L"No Data", rArmRect.x + rArmRect.w/2, rArmRect.y + 45);

			SDL_Rect rLegRect = { statusBase.x + 298,statusBase.y + 206 + 91*2,236,83 };
			drawStadium(rLegRect, col::black, 150, 4);
			setFontSize(18);
			setFont(fontType::mainFontSemiBold);
			drawText(L"Right Leg", rLegRect.x + 6, rLegRect.y - 2);

			//우측 열: Torso, L.Arm, L.Leg
			SDL_Rect torsoRect = { statusBase.x + 298 + 335,statusBase.y + 206,236,83 };
			drawStadium(torsoRect, col::black, 150, 4);
			setFontSize(18);
			setFont(fontType::mainFontSemiBold);
			drawText(L"Torso", torsoRect.x + 6, torsoRect.y - 2);

			SDL_Rect lArmRect = { statusBase.x + 298 + 335,statusBase.y + 206 + 91 * 1,236,83 };
			drawStadium(lArmRect, col::black, 150, 4);
			setFontSize(18);
			setFont(fontType::mainFontSemiBold);
			drawText(L"Left Arm", lArmRect.x + 6, lArmRect.y - 2);

			SDL_Rect lLegRect = { statusBase.x + 298 + 335,statusBase.y + 206 + 91 * 2,236,83 };
			drawStadium(lLegRect, col::black, 150, 4);
			setFontSize(18);
			setFont(fontType::mainFontSemiBold);
			drawText(L"Left Leg", lLegRect.x + 6, lLegRect.y - 2);

			setFont(fontType::mainFont);

			
			SDL_SetTextureAlphaMod(spr::bodyShape->getTexture(), 70);
			drawSpriteCenter(spr::bodyShape, 0, statusBase.x + 583, statusBase.y + 338);
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

	void step()
	{
		tabType = tabFlag::back;
	}
};
