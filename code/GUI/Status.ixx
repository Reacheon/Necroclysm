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
import Entity;
import playerLevel;

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
	int sessionAlloc[3] = { 0, }; // 이번 창 세션에서 Str/Int/Dex에 분배한 AP (창을 닫기 전까지 초기화 버튼으로 환불 가능)
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

		// y 상한: 중앙-20px. (1080 기준 274 — 절대값이면 세로로 긴 화면에서 상단에 붙음)
		const int maxY = cameraH / 2 - 266;
		if (statusBase.y > maxY) { statusBase.y = maxY; }

		// x는 좌측 저항 스타디움 박스(HUD_draw: x -10~87, y cameraH-665~cameraH-314)와
		// 세로로 겹칠 때만 87로 밀어낸다. 세로로 긴 화면에서는 안 겹치므로 진짜 중앙 정렬.
		const int resBoxTop = cameraH - 665;
		const int resBoxBottom = cameraH - 314;
		const bool overlapResBox = statusBase.y < resBoxBottom && statusBase.y + statusBase.h > resBoxTop;
		if (overlapResBox && statusBase.x < 87) { statusBase.x = 87; }

		x = statusBase.x;
		y = statusBase.y;
	}
	void drawGUI(); // Status_draw.cpp에서 구현
	void drawRadarChart(const std::function<SDL_Color(const SDL_Rect&)>& stadiumCol); // Status_radar.cpp에서 구현
	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::winUnfoldClose);
		}
		else
		{
			//AP 분배: +버튼은 분배할 AP가 남아있을 때만 그려지므로 클릭도 그때만 유효
			if (playerLevel::ap > 0)
			{
				unsigned __int8* stats[3] = { &PlayerPtr->entityInfo.statStr, &PlayerPtr->entityInfo.statInt, &PlayerPtr->entityInfo.statDex };
				for (int i = 0; i < 3; i++)
				{
					SDL_Rect upBtn = { statusBase.x + 625 + 83 * i + 40, statusBase.y + 82 + 27, 22, 22 };
					if (checkCursor(&upBtn))
					{
						(*stats[i])++;
						sessionAlloc[i]++;
						playerLevel::ap--;
						return;
					}
				}
			}

			//초기화: 이번 창 세션에서 분배한 포인트 전액 환불 (창을 닫으면 확정)
			int allocated = sessionAlloc[0] + sessionAlloc[1] + sessionAlloc[2];
			if (allocated > 0)
			{
				SDL_Rect resetBtn = { statusBase.x + 579, statusBase.y + 98, 26, 26 };
				if (checkCursor(&resetBtn))
				{
					PlayerPtr->entityInfo.statStr -= sessionAlloc[0];
					PlayerPtr->entityInfo.statInt -= sessionAlloc[1];
					PlayerPtr->entityInfo.statDex -= sessionAlloc[2];
					playerLevel::ap += allocated;
					sessionAlloc[0] = sessionAlloc[1] = sessionAlloc[2] = 0;
				}
			}
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
