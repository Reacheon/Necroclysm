import GodPanel;

#include <SDL3/SDL.h>

import globalVar;
import constVar;
import textureVar;
import util;
import Sprite;
import drawWindow;
import drawSprite;
import checkCursor;
import drawText;
import GodBehavior;
import GodRegistry;

void GodPanel::drawGUI()
{
	if (getStateDraw() == false) { return; }

	if (getFoldRatio() == 1.0)
	{
		auto* behavior = GodRegistry::get(targetGod);
		std::wstring windowTitle = behavior ? behavior->name : L"Unknown God";

		setWindowAlpha(230);
		drawWindow(&panelBase, L"God", 1);
		resetWindowAlpha();

		// 화살표
		if (arrowDir == dir16::left) drawSprite(spr::newWindowArrow, 0, panelBase.x - 26, panelBase.y + arrowOffsetY);
		else if (arrowDir == dir16::right)
		{
			setFlip(SDL_FLIP_HORIZONTAL);
			drawSprite(spr::newWindowArrow, 0, panelBase.x + panelBase.w - 4, panelBase.y + arrowOffsetY);
			setFlip(SDL_FLIP_NONE);
		}


		setFontSize(24);
		setFont(fontType::mainFontBold);
		drawTextCenter(L"Rehylion, the Healer", panelBase.x + panelBase.w/2, panelBase.y + 68);
		setFont(fontType::mainFont);
		setZoom(2.0);
		drawSprite(spr::propset, itemDex[behavior->altarItemCode].propSprIndex, panelBase.x + 12, panelBase.y + 30);
		setZoom(1.0);

		// ── 하단 버튼 (Devote / Renounce) ──
		// Msg 스타일 버튼 (spr::msgChoiceBtn)
		{
			bool isFollowing = (playerGod == targetGod);
			bool canDevote = (playerGod == godFlag::none);

			int btnColorSprIndex = 0;
			if (checkCursor(&actionBtn))
			{
				if (click) btnColorSprIndex = 2;
				else btnColorSprIndex = 1;
			}

			drawSprite(spr::msgChoiceBtn, btnColorSprIndex, actionBtn.x, actionBtn.y);

			setFont(fontType::mainFontSemiBold);
			setFontSize(24);

			if (isFollowing)
			{
				// 빨간색 Renounce
				drawTextCenter(L"Renounce", actionBtn.x + actionBtn.w / 2, actionBtn.y + actionBtn.h / 2, { 0xE0, 0x40, 0x40 });
			}
			else if (canDevote)
			{
				drawTextCenter(L"Devote", actionBtn.x + actionBtn.w / 2, actionBtn.y + actionBtn.h / 2);
			}
			else
			{
				// 이미 다른 신을 믿고 있음 - 회색 비활성 표시
				drawTextCenter(L"Devote", actionBtn.x + actionBtn.w / 2, actionBtn.y + actionBtn.h / 2, col::gray);
			}

			setFont(fontType::mainFont);
		}
	}
	else
	{
		SDL_Rect vRect = panelBase;
		int type = 1;
		switch (type)
		{
		case 0:
			vRect.w = panelBase.w * getFoldRatio();
			vRect.h = panelBase.h * getFoldRatio();
			break;
		case 1:
			vRect.x = panelBase.x + panelBase.w * (1 - getFoldRatio()) / 2;
			vRect.w = panelBase.w * getFoldRatio();
			break;
		}
		drawWindow(&vRect);
	}
}
