import GodPanel;

#include <SDL3/SDL.h>

import Msg;
import globalVar;
import constVar;
import textureVar;
import util;
import Sprite;
import drawWindow;
import drawSprite;
import checkCursor;
import drawText;
import drawPrimitive;
import GodBehavior;
import GodRegistry;
import SkillBehavior;
import SkillRegistry;

void GodPanel::drawGUI()
{
	if (getStateDraw() == false) { return; }

	if (getFoldRatio() == 1.0)
	{
		auto* behavior = GodRegistry::get(targetGod);
		if (!behavior) return;

		const int px = panelBase.x;
		const int py = panelBase.y;
		const int pw = panelBase.w;

		setWindowAlpha(230);
		drawWindow(&panelBase, L"God", 114);
		resetWindowAlpha();

		// ── 화살표 ──
		if (arrowDir == dir16::left) drawSprite(spr::newWindowArrow, 0, px - 26, py + arrowOffsetY);
		else if (arrowDir == dir16::right)
		{
			setFlip(SDL_FLIP_HORIZONTAL);
			drawSprite(spr::newWindowArrow, 0, px + pw - 4, py + arrowOffsetY);
			setFlip(SDL_FLIP_NONE);
		}

		// ── 제단 아이콘 + 신 이름 ──
		setZoom(2.0);
		drawSprite(spr::propset, itemDex[behavior->altarItemCode].propSprIndex, px + 12, py + 30);
		setZoom(1.0);

		setFontSize(24);
		setFont(fontType::mainFontBold);
		drawTextCenter(behavior->name + L", " + behavior->title, px + pw / 2, py + 68);

		// ── 설명문 (Lore) ──
		int curY = py + 95;
		if (!behavior->descript.empty())
		{
			setFont(fontType::mainFont);
			setFontSize(15);
			setFontGap(3);
			int lines = drawTextWidth(behavior->descript, px + 17, curY, pw - 34, -1);
			curY += lines * (15 + 3) + 10;
			setFontGap(0);
		}

		// ── 구분선 ──
		drawLine(px + 14, curY, px + pw - 14, curY, col::gray, 100);
		curY += 10;

		// ════════════════════════════════════
		// ── Granted Skills 섹션 ──
		// ════════════════════════════════════
		setFont(fontType::mainFontSemiBold);
		setFontSize(16);
		drawText(L"#e9c900Granted Skills", px + 17, curY);
		curY += 24;

		// rankSkills를 flat한 리스트로 전개 (rank 순서대로)
		struct SkillEntry { std::wstring skillId; int rank; };
		std::vector<SkillEntry> skillList;
		for (auto& [rank, ids] : behavior->rankSkills)
		{
			for (const std::wstring& id : ids)
				skillList.push_back({ id, rank });
		}

		setFont(fontType::mainFont);
		setFontSize(14);

		const int colWidth = (pw - 34) / 2;	// 한 열 너비
		const int rowH = 30;					// 한 행 높이

		for (int i = 0; i < (int)skillList.size(); i++)
		{
			int col = i % 2;
			int row = i / 2;
			int sx = px + 17 + col * colWidth;
			int sy = curY + row * rowH;

			auto* skill = SkillRegistry::get(skillList[i].skillId);
			if (!skill) continue;

			// 스킬 아이콘
			drawSprite(spr::icon24, skill->iconIndex, sx, sy);

			// 스킬 이름 + 필요 랭크 (★N) 또는 패시브 표시
			std::wstring skillText;
			if (skill->type == skillType::PASSIVE) skillText = skill->name + L" #e9c900(Passive)";
			else
				skillText = skill->name + L" #e9c900\u2605" + std::to_wstring(skillList[i].rank) + L"";
			setFontSize(14);
			if (queryTextWidth(skillText, true) >= 156) setFontSize(13);
			drawText(skillText, sx + 28, sy + 4);
		}

		int skillRows = ((int)skillList.size() + 1) / 2;
		curY += skillRows * rowH + 10;

		// ── 구분선 ──
		drawLine(px + 14, curY, px + pw - 14, curY, col::gray, 100);
		curY += 10;

		// ════════════════════════════════════
		// ── Prohibitions 섹션 ──
		// ════════════════════════════════════
		setFont(fontType::mainFontSemiBold);
		setFontSize(16);
		drawText(L"#E04040Prohibitions", px + 17, curY);
		curY += 24;

		setFont(fontType::mainFont);
		setFontSize(14);
		for (auto& item : behavior->prohibitions)
		{
			drawText(L"#E04040- #FFFFFF" + item, px + 22, curY);
			curY += 20;
		}
		curY += 6;

		// ── 구분선 ──
		drawLine(px + 14, curY, px + pw - 14, curY, col::gray, 100);
		curY += 10;

		// ════════════════════════════════════
		// ── Gaining Piety 섹션 ──
		// ════════════════════════════════════
		setFont(fontType::mainFontSemiBold);
		setFontSize(16);
		drawText(L"#59cb65Gaining Piety", px + 17, curY);
		curY += 24;

		setFont(fontType::mainFont);
		setFontSize(14);
		setFontGap(2);
		for (auto& item : behavior->pietyGains)
		{
			std::wstring bulletText = L"#59cb65- #FFFFFF" + item;
			int lines = drawTextWidth(bulletText, px + 22, curY, pw - 44, -1);
			curY += lines * (14 + 2) + 4;
		}
		setFontGap(0);

		// ── 하단 버튼 (Devote / Renounce) ──
		{
			bool isFollowing = (playerGod == targetGod);
			bool canDevote = (playerGod == godFlag::none);

			int btnColorSprIndex = 0;
			if (Msg::ins() == nullptr && checkCursor(&actionBtn))
			{
				if (click) btnColorSprIndex = 2;
				else btnColorSprIndex = 1;
			}

			drawSprite(spr::msgChoiceBtn, btnColorSprIndex, actionBtn.x, actionBtn.y);

			setFont(fontType::mainFontSemiBold);
			setFontSize(24);

			if (isFollowing)
			{
				drawTextCenter(L"Renounce", actionBtn.x + actionBtn.w / 2, actionBtn.y + actionBtn.h / 2, { 0xE0, 0x40, 0x40 });
			}
			else if (canDevote)
			{
				drawTextCenter(L"Devote", actionBtn.x + actionBtn.w / 2, actionBtn.y + actionBtn.h / 2);
			}
			else
			{
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
