module;
#include <SDL3/SDL.h>

export module Aim;

import std;
import util;
import GUI;
import constVar;
import textureVar;
import drawText;
import drawSprite;
import globalVar;
import checkCursor;
import drawWindow;
import Player;
import World;
import Entity;
import log;
import ItemData;
import ItemPocket;
import replaceStr;
import turnWait;

enum class fireSelector
{
	AUTO,
	SINGLE,
	BURST,
};

static fireSelector currentSelector = fireSelector::SINGLE;

enum class pistolAimMode
{
	NONE,
	TWO_HAND,   // 한 손 권총 + 빈 손 (양손 사격)
	ONE_HAND,   // 한 손 권총 + 다른 물건
	DUAL,       // 쌍권총
};

static bool dualNextLeft = false;
static equipHandFlag dualSelected = equipHandFlag::right;

export class Aim : public GUI
{
private:
	inline static Aim* ptr = nullptr;
	SDL_Rect aimBase;
	std::vector<Point3> aimTrailRev;
	Point3 aimCoord = { 0,0,0 };

	double aimAcc = 0;
	double fakeAimAcc = aimAcc;
	double aimProgress = 0; //sqrt 곡선 조준 진행도 (매 턴 0.7~1.3씩 랜덤 증가)
	int aimStack = 0;
	atkType targetAtkType = atkType::shot;

	SDL_Texture* aimInfoTex = nullptr;
	static constexpr int AIM_INFO_W = 200;
	static constexpr int AIM_INFO_H = 34;

	bool deactClickUp = false;

	bool isRifle = false;
	pistolAimMode pistolMode = pistolAimMode::NONE;
	equipHandFlag pistolHand = equipHandFlag::none;
	int leftPistolIdx = -1;
	int rightPistolIdx = -1;
	std::wstring leftPistolName;
	std::wstring rightPistolName;
public:
	Aim() : GUI(false)
	{
		//1개 이상의 메시지 객체 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"중복된 GUI 인스턴스가 생성되었다.");
		ptr = this;

		//메세지 박스 렌더링
		changeXY(cameraW / 2, cameraH / 2, true);

		aimInfoTex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, AIM_INFO_W, AIM_INFO_H);
		SDL_SetTextureScaleMode(aimInfoTex, SDL_SCALEMODE_NEAREST);
		SDL_SetTextureBlendMode(aimInfoTex, SDL_BLENDMODE_BLEND);

		


		ItemPocket* pEquip = PlayerEquip();

		// 한손 권총 감지
		bool hasLeftItem = false, hasRightItem = false;
		for (int i = 0; i < pEquip->itemInfo.size(); ++i)
		{
			auto& item = pEquip->itemInfo[i];
			if (item.equipState == equipHandFlag::none) continue;
			bool isOneHandGun = item.checkFlag(itemFlag::GUN) && !item.checkFlag(itemFlag::SPR_TH_WEAPON);
			if (item.equipState == equipHandFlag::left)
			{
				hasLeftItem = true;
				if (isOneHandGun) leftPistolIdx = i;
			}
			else if (item.equipState == equipHandFlag::right)
			{
				hasRightItem = true;
				if (isOneHandGun) rightPistolIdx = i;
			}
		}

		if (leftPistolIdx != -1 && rightPistolIdx != -1)
		{
			// 쌍권총
			bool leftAmmo = getBulletNumber(pEquip->itemInfo[leftPistolIdx]) > 0;
			bool rightAmmo = getBulletNumber(pEquip->itemInfo[rightPistolIdx]) > 0;

			if (leftAmmo && rightAmmo)
			{
				pistolMode = pistolAimMode::DUAL;
				dualSelected = dualNextLeft ? equipHandFlag::left : equipHandFlag::right;
			}
			else if (leftAmmo)
			{
				pistolMode = pistolAimMode::ONE_HAND;
				pistolHand = equipHandFlag::left;
			}
			else if (rightAmmo)
			{
				pistolMode = pistolAimMode::ONE_HAND;
				pistolHand = equipHandFlag::right;
			}
			else
			{
				pistolMode = pistolAimMode::DUAL;
				dualSelected = dualNextLeft ? equipHandFlag::left : equipHandFlag::right;
			}

			leftPistolName = pEquip->itemInfo[leftPistolIdx].name;
			rightPistolName = pEquip->itemInfo[rightPistolIdx].name;

			if (pistolMode == pistolAimMode::DUAL)
				PlayerPtr->setSpriteIndex(charSprIndex::BCAST);
			else if (pistolHand == equipHandFlag::right)
				PlayerPtr->setSpriteIndex(charSprIndex::RCAST);
			else
				PlayerPtr->setSpriteIndex(charSprIndex::LCAST);
		}
		else if (leftPistolIdx != -1 || rightPistolIdx != -1)
		{
			equipHandFlag gunHand = (leftPistolIdx != -1) ? equipHandFlag::left : equipHandFlag::right;
			bool otherHasItem = (gunHand == equipHandFlag::left) ? hasRightItem : hasLeftItem;

			if (!otherHasItem)
			{
				pistolMode = pistolAimMode::TWO_HAND;
				pistolHand = gunHand;
				PlayerPtr->setSpriteIndex(charSprIndex::AIM_PISTOL);
			}
			else
			{
				pistolMode = pistolAimMode::ONE_HAND;
				pistolHand = gunHand;
				if (gunHand == equipHandFlag::right)
					PlayerPtr->setSpriteIndex(charSprIndex::RCAST);
				else
					PlayerPtr->setSpriteIndex(charSprIndex::LCAST);
			}
		}
		else
		{
			// 기존 로직: 소총/활/석궁
			if (pEquip->itemInfo[0].checkFlag(itemFlag::CROSSBOW)) PlayerPtr->setSpriteIndex(charSprIndex::AIM_RIFLE);
			else if (pEquip->itemInfo[0].checkFlag(itemFlag::BOW)) PlayerPtr->setSpriteIndex(charSprIndex::AIM_RIFLE);
			else if (pEquip->itemInfo[0].checkFlag(itemFlag::GUN) && pEquip->itemInfo[0].checkFlag(itemFlag::SPR_TH_WEAPON)) PlayerPtr->setSpriteIndex(charSprIndex::AIM_RIFLE);
		}

		// 돌격소총 조정간 활성화 여부
		for (int i = 0; i < pEquip->itemInfo.size(); ++i)
		{
			if (pEquip->itemInfo[i].equipState == equipHandFlag::none) continue;
			if (pEquip->itemInfo[i].itemCode == itemID::assaultRifle) isRifle = true;
			break;
		}

		

		int pX = PlayerX(), pY = PlayerY(), pZ = PlayerZ();
		
		if(PlayerInfo().sprFlip == false) aimCoord = { pX + 1, pY, pZ };
		else  aimCoord = { pX - 1, pY, pZ };

		Entity* nearTarget = nullptr;
		double hiDist = 99999;
		const int searchRange = 10;
		for (int dx = -searchRange; dx <= searchRange; dx++)
		{
			for (int dy = -searchRange; dy <= searchRange; dy++)
			{
				if (TileFov(pX + dx, pY + dy, pZ) == fovFlag::white)
				{
					if (TileEntity(pX + dx, pY + dy, pZ) != nullptr)
					{
						if (TileEntity(pX + dx, pY + dy, pZ) != PlayerPtr)
						{
							if (std::sqrt(dx * dx + dy * dy) < hiDist)
							{
								nearTarget = TileEntity(pX + dx, pY + dy, pZ);
								hiDist = std::sqrt(dx * dx + dy * dy);
							}
						}
					}
				}
			}
		}

		if (nearTarget != nullptr)
		{
			changeAimTarget(nearTarget->getGridX(), nearTarget->getGridY());
		}
	}
	~Aim()
	{
		if (aimInfoTex) { SDL_DestroyTexture(aimInfoTex); aimInfoTex = nullptr; }
		PlayerPtr->setSpriteIndex(charSprIndex::WALK);
		ptr = nullptr;
	}
	static Aim* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		aimBase = { 0, 0, 650, 376 };

		if (center == false)
		{
			aimBase.x += inputX;
			aimBase.y += inputY;
		}
		else
		{
			aimBase.x += inputX - aimBase.w / 2;
			aimBase.y += inputY - aimBase.h / 2;
		}



		if (center == false)
		{
			x = inputX;
			y = inputY;
		}
		else
		{
			x = inputX - aimBase.w / 2;
			y = inputY - aimBase.h / 2;
		}

	}
	void drawGUI()
	{
		if (getStateDraw() == false) { return; }

		SDL_Rect dst;

		if (aimAcc != 0 && turnCycle == turn::playerInput)
		{
			std::wstring accStr = decimalCutter(fakeAimAcc * 100.0, 1);

			int iconIndex = 0;
			int currentBullet = 0;
			int maxBullet = 0;
			std::vector<ItemData>& equipInfo = PlayerEquip()->itemInfo;
			//손에 든 장비 찾기
			int weaponIdx = -1;
			if (pistolMode == pistolAimMode::DUAL)
			{
				weaponIdx = (dualSelected == equipHandFlag::left) ? leftPistolIdx : rightPistolIdx;
				iconIndex = 97;
			}
			else if (pistolMode != pistolAimMode::NONE)
			{
				weaponIdx = (pistolHand == equipHandFlag::left) ? leftPistolIdx : rightPistolIdx;
				iconIndex = 97;
			}
			else
			{
				for (int i = 0; i < equipInfo.size(); ++i)
				{
					if (equipInfo[i].equipState == equipHandFlag::none) continue;
					weaponIdx = i;

					if (equipInfo[i].checkFlag(itemFlag::BOW))      iconIndex = 95;
					else if (equipInfo[i].checkFlag(itemFlag::CROSSBOW)) iconIndex = 96;
					else if (equipInfo[i].checkFlag(itemFlag::GUN))      iconIndex = 97;
					break;
				}
			}


			if (weaponIdx != -1)
			{
				//무기 내부의 탄 계산
				ItemData& weapon = equipInfo[weaponIdx];
				currentBullet += getBulletNumber(weapon);
				for (unsigned short code : weapon.pocketOnlyItem)
				{
					if (itemDex[code].checkFlag(itemFlag::AMMO)) maxBullet += weapon.pocketMaxNumber;
					else if (itemDex[code].checkFlag(itemFlag::MAGAZINE)) 
					{
						if (weapon.pocketPtr && !weapon.pocketPtr->itemInfo.empty()) maxBullet += weapon.pocketPtr->itemInfo[0].pocketMaxNumber;
						else maxBullet += itemDex[code].pocketMaxNumber;
					}
				}

				//무기를 제외한 장비들의 탄 계산
				for (int j = 0; j < equipInfo.size(); ++j)
				{
					if (j == weaponIdx)                    continue;
					ItemData& itm = equipInfo[j];

					if (!itm.checkFlag(itemFlag::MAGAZINE)) continue;
					if (itm.pocketOnlyItem.empty())        continue;

					bool sameAmmo = std::any_of(
						weapon.pocketOnlyItem.begin(), weapon.pocketOnlyItem.end(),
						[&](unsigned short c) { return c == itm.pocketOnlyItem[0]; });

					if (!sameAmmo) continue;

					currentBullet += getBulletNumber(itm);
					maxBullet += itm.pocketMaxNumber;
				}
			}

			std::wstring bulletStr = std::to_wstring(currentBullet)+L"/" + std::to_wstring(maxBullet);
			accStr += L"%";
			//if(aimStack>0) accStr += L" (" + std::to_wstring(aimStack) + L")";

			// ── 텍스쳐에 조준 정보 렌더링 ──
			setFont(fontType::pixel);
			setFontSize(12);

			SDL_Texture* prevTarget = SDL_GetRenderTarget(renderer);
			SDL_SetRenderTarget(renderer, aimInfoTex);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
			SDL_RenderClear(renderer);

			setZoom(1.0);
			int cx = AIM_INFO_W / 2;

			// 잔탄 (상단)
			int bulletY = 9;
			drawTextOutlineCenter(bulletStr, cx, bulletY, col::white);
			setZoom(2.0);
			drawSpriteCenter(spr::icon16, iconIndex, cx - queryTextWidth(bulletStr) / 2 - 9, bulletY + 1);
			setZoom(1.0);

			// 명중률 (하단)
			int accY = AIM_INFO_H - 9;
			drawTextOutlineCenter(accStr, cx, accY, col::white);

			// 조정간 표시 (돌격소총만)
			if (isRifle)
			{
				setFontSize(11);
				auto selectorLabel = [&](const std::wstring& label, fireSelector sel, int yPos)
				{
					if (currentSelector == sel)
					{
						std::wstring text = label + L"◀";
						drawTextOutline(text.c_str(), cx + 22, yPos, col::white);
					}
					else
					{
						drawTextOutline(label.c_str(), cx + 22, yPos, col::lightGray);
					}
				};
				selectorLabel(sysStr[406],   fireSelector::AUTO,   accY - 4 - 22);
				selectorLabel(sysStr[407], fireSelector::SINGLE, accY - 4 - 11);
				selectorLabel(sysStr[408],  fireSelector::BURST,  accY - 4);
			}
			// 쌍권총 선택 표시
			else if (pistolMode == pistolAimMode::DUAL)
			{
				setFontSize(11);
				std::wstring labelL = L"[L] " + leftPistolName;
				std::wstring labelR = L"[R] " + rightPistolName;

				if (dualSelected == equipHandFlag::left)
				{
					drawTextOutline((labelL + L"◀").c_str(), cx + 22, accY - 4 - 11, col::white);
					drawTextOutline(labelR.c_str(), cx + 22, accY - 4, col::lightGray);
				}
				else
				{
					drawTextOutline(labelL.c_str(), cx + 22, accY - 4 - 11, col::lightGray);
					drawTextOutline((labelR + L"◀").c_str(), cx + 22, accY - 4, col::white);
				}
			}


			SDL_SetRenderTarget(renderer, prevTarget);

			// zoomScale에 따라 확대하여 출력
			if(zoomScale==2.0) setZoom(1.5);
			else setZoom(2.0);
			int yOffset = 0;
			if (zoomScale == 2.0) yOffset = -12;
			else if (zoomScale == 3.0)  yOffset = -8;
			else if (zoomScale == 4.0)  yOffset = -2;
			else if (zoomScale == 5.0)  yOffset = +8;
			int drawY = (int)(cameraH / 2.0 - (8.0 + AIM_INFO_H / 2.0) * zoomScale + yOffset);
			drawTextureCenter(aimInfoTex, cameraW / 2, drawY);
			setZoom(1.0);
		}


		if(checkCursor(&tab)==false && checkCursor(&letterbox) == false && checkCursor(&quickSlotRegion) == false)
		{
			double tileSize = 16 * zoomScale;
			int tgtX = getAbsMouseGrid().x;
			int tgtY = getAbsMouseGrid().y;
			dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
			dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
			dst.w = tileSize;
			dst.h = tileSize;



			setZoom(zoomScale);
			SDL_SetTextureAlphaMod(spr::aimMarkerWhite->getTexture(), 150); //텍스쳐 투명도 설정
			drawSpriteCenter
			(
				spr::aimMarkerWhite,
				0,
				dst.x + dst.w / 2,
				dst.y + dst.h / 2
			);
			SDL_SetTextureAlphaMod(spr::aimMarkerWhite->getTexture(), 255); //텍스쳐 투명도 설정
			setZoom(1.0);
		}

		{
			int tgtX = aimCoord.x;
			int tgtY = aimCoord.y;
			dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
			dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
			dst.w = 16.0 * zoomScale;
			dst.h = 16.0 * zoomScale;
			setZoom(zoomScale);


			int sprIndex = 0;
			if (SDL_GetTicks() % 800 < 200) sprIndex = 0;
			else if (SDL_GetTicks() % 800 < 400) sprIndex = 1;
			else if (SDL_GetTicks() % 800 < 600) sprIndex = 2;
			else sprIndex = 1;

			drawSpriteCenter
			(
				spr::aimMarkerTmp,
				sprIndex,
				dst.x + dst.w / 2,
				dst.y + dst.h / 2
			);
			setZoom(1.0);
		}

		for (int i = 0; i < aimTrailRev.size(); i++)
		{
			int tgtX = aimTrailRev[i].x;
			int tgtY = aimTrailRev[i].y;
			dst.x = cameraW / 2 + zoomScale * ((16 * tgtX + 8) - cameraX) - ((16 * zoomScale) / 2);
			dst.y = cameraH / 2 + zoomScale * ((16 * tgtY + 8) - cameraY) - ((16 * zoomScale) / 2);
			dst.w = 16.0 * zoomScale;
			dst.h = 16.0 * zoomScale;
			setZoom(zoomScale);
			drawSpriteCenter
			(
				spr::trail,
				1,
				dst.x + dst.w / 2,
				dst.y + dst.h / 2
			);
			setZoom(1.0);
		}

		//사념파
		{

			if (isRifle || pistolMode == pistolAimMode::DUAL)
			{
				setFontSize(24);
				setFont(fontType::mainFont);
				int pivotX = cameraW - 426;
				int pivotY = cameraH - 280;
				drawSprite(spr::btnGuideBackground, pivotX, pivotY);
				drawSpriteCenter(spr::keyboardButtons, keyboardIndex::mouseLeft, pivotX + 112, pivotY + 30);
				drawText(sysStr[409], pivotX + 132, pivotY + 15);

				drawSpriteCenter(spr::keyboardButtons, keyboardIndex::mouseRight, pivotX + 112 + 100, pivotY + 30);
				drawText(sysStr[410], pivotX + 132 + 100, pivotY + 15);

				drawSpriteCenter(spr::keyboardButtons, keyboardIndex::mouseWheel, pivotX + 112 + 190, pivotY + 30);
				drawText(isRifle ? sysStr[411] : sysStr[412], pivotX + 132 + 190, pivotY + 15);
			}
			else
			{
				setFontSize(24);
				setFont(fontType::mainFont);
				int pivotX = cameraW - 426 + 90;
				int pivotY = cameraH - 280;
				drawSprite(spr::btnGuideBackground, pivotX, pivotY);
				drawSpriteCenter(spr::keyboardButtons, keyboardIndex::mouseLeft, pivotX + 122, pivotY + 30);
				drawText(sysStr[409], pivotX + 142, pivotY + 15);

				drawSpriteCenter(spr::keyboardButtons, keyboardIndex::mouseRight, pivotX + 132 + 100, pivotY + 30);
				drawText(sysStr[410], pivotX + 152 + 100, pivotY + 15);
			}
		}
	}

	//현재 조준 중인 무기의 itemCode를 반환 (무기 없으면 0)
	unsigned __int16 getAimWeaponCode()
	{
		auto& equipInfo = PlayerEquip()->itemInfo;
		if (pistolMode == pistolAimMode::DUAL)
		{
			int idx = (dualSelected == equipHandFlag::left) ? leftPistolIdx : rightPistolIdx;
			if (idx >= 0 && idx < equipInfo.size()) return equipInfo[idx].itemCode;
		}
		else if (pistolMode != pistolAimMode::NONE)
		{
			int idx = (pistolHand == equipHandFlag::left) ? leftPistolIdx : rightPistolIdx;
			if (idx >= 0 && idx < equipInfo.size()) return equipInfo[idx].itemCode;
		}
		else
		{
			if (equipInfo.size() > 0) return equipInfo[0].itemCode;
		}
		return 0;
	}

	//sqrt 곡선으로 명중률 계산: baseAim + (maxAim - baseAim) * sqrt(min(1, aimProgress / fullAimTurns))
	double calcAimAcc()
	{
		unsigned __int16 weaponCode = getAimWeaponCode();
		float baseAim = itemDex[weaponCode].gunAccInit;
		float maxAim = itemDex[weaponCode].gunAccMax;
		int fullAimTurns = itemDex[weaponCode].gunFullAimTurns;
		int optRange = itemDex[weaponCode].gunOptRange;

		//거리 보정: 적정거리에서 벗어날수록 baseAim만 감소 (maxAim은 유지)
		int distance = myMax(abs(PlayerX() - aimCoord.x), abs(PlayerY() - aimCoord.y));
		float distPenalty = (float)abs(distance - optRange) * 0.03f;
		float effectiveBaseAim = baseAim - distPenalty;
		if (effectiveBaseAim < 0.01f) effectiveBaseAim = 0.01f;

		//sqrt 곡선 적용
		double ratio = (fullAimTurns > 0) ? myMin(1.0, aimProgress / (double)fullAimTurns) : 1.0;
		double acc = effectiveBaseAim + (maxAim - effectiveBaseAim) * sqrt(ratio);

		if (acc < 0.01) acc = 0.01;
		if (acc > maxAim) acc = maxAim;
		return acc;
	}

	void changeAimTarget(int tgtX, int tgtY)
	{
		aimCoord.x = tgtX;
		aimCoord.y = tgtY;

		if (TileEntity(aimCoord.x, aimCoord.y, aimCoord.z) != nullptr)
		{
			aimProgress = 0;
			aimAcc = calcAimAcc();
		}
		else aimAcc = 0;
		fakeAimAcc = aimAcc;
		aimStack = 0;

		if (aimCoord.x < PlayerX()) PlayerPtr->setDirection(4);
		else if (aimCoord.x > PlayerX()) PlayerPtr->setDirection(0);

		std::vector<Point2> lineCoord;
		makeLine(lineCoord, aimCoord.x - PlayerX(), aimCoord.y - PlayerY());
		aimTrailRev.clear();
		for (int i = 0; i < lineCoord.size(); i++)
		{
			if (lineCoord[i].x != 0 || lineCoord[i].y != 0)
			{
				aimTrailRev.push_back({ PlayerX() + lineCoord[i].x,PlayerY() + lineCoord[i].y,PlayerZ() });
			}
		}
	}
	void aimAddAcc()
	{
		if (aimAcc != 0)
		{
			aimProgress += randomRangeFloat(0.7, 1.3); //매 턴 랜덤 진행도 증가
			aimAcc = calcAimAcc();
			PlayerPtr->flash = { 255, 255, 255, 255 };
			aimStack++;
		}
	}

	void clickUpGUI()
	{
		if (getStateInput() == false) { return; }

		if (checkCursor(&tab))
		{
			close(aniFlag::null); // Back 버튼
		}
		else if (checkCursor(&letterbox) || checkCursor(&quickSlotRegion))
		{
			close(aniFlag::null);
		}
		else
		{
			// 좌클릭: 현재 조준 대상에게 사격
			executeTabShot();
		}
	}
	void clickMotionGUI(int dx, int dy) {}
	void clickDownGUI() {}
	void clickRightGUI()
	{
		if (getStateInput() == false) { return; }
		if (checkCursor(&tab) || checkCursor(&letterbox) || checkCursor(&quickSlotRegion)) return;

		int tgtX = getAbsMouseGrid().x;
		int tgtY = getAbsMouseGrid().y;

		if (tgtX == aimCoord.x && tgtY == aimCoord.y)
		{
			// 같은 대상: 조준 집중 (명중률 상승)
			aimAddAcc();
		}
		else
		{
			// 다른 대상: 조준 대상 변경
			changeAimTarget(tgtX, tgtY);
		}
	}
	void clickHoldGUI() {}
	void mouseWheel()
	{
		// 쌍권총: 사격할 총 선택
		if (pistolMode == pistolAimMode::DUAL)
		{
			if (event.wheel.y != 0)
			{
				if (dualSelected == equipHandFlag::left) dualSelected = equipHandFlag::right;
				else dualSelected = equipHandFlag::left;
			}
			return;
		}

		if (!isRifle) return;

		// 순서: AUTO(0) - SINGLE(1) - BURST(2), 위 → 아래
		int sel = static_cast<int>(currentSelector);
		if (event.wheel.y > 0) sel--;  // 스크롤 위 → 위쪽 항목
		else if (event.wheel.y < 0) sel++;  // 스크롤 아래 → 아래쪽 항목

		if (sel < 0) sel = 0;
		if (sel > 2) sel = 2;

		fireSelector newSel = static_cast<fireSelector>(sel);
		if (newSel != currentSelector)
		{
			currentSelector = newSel;
			std::wstring name = L"";
			switch (currentSelector)
			{
			case fireSelector::AUTO:   name = sysStr[406];   break;
			case fireSelector::SINGLE: name = sysStr[407]; break;
			case fireSelector::BURST:  name = sysStr[408];  break;
			}
			updateLog(replaceStr(sysStr[413], L"(%mode)", col2Str(SDL_Color{ 0x58,0xD6,0x8D }) + name + col2Str(col::white)));
		}
	}
	void gamepadBtnDown() 
	{
		switch (event.gbutton.button)
		{
		case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
			aimAddAcc();
			break;
		case SDL_GAMEPAD_BUTTON_EAST:
			close(aniFlag::null);
			break;

		}
	}
	void gamepadBtnMotion() {}
	void gamepadBtnUp() {}
	void step() 
	{
		tabType = tabFlag::back;

		{
			if (PlayerPtr->flash.a > 0)
			{
				if (PlayerPtr->flash.a > 17) { PlayerPtr->flash.a -= 17; }
				else PlayerPtr->flash.a = 0;
			}
		}

		if (fabs(aimAcc - fakeAimAcc) > 0.001)
		{
			//목표까지 약 0.3초(18프레임)에 도달하도록 매 프레임 차이의 일정 비율 이동
			double delta = (aimAcc - fakeAimAcc) * 0.12;
			if (fabs(delta) < 0.001) delta = (aimAcc > fakeAimAcc) ? 0.001 : -0.001;
			fakeAimAcc += delta;
		}
		else fakeAimAcc = aimAcc;



		if (option::inputMethod == input::gamepad)
		{
			if (delayR2 <= 0 && SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) > 1000)
			{
				executeTabShot();
				delayR2 = 20;
			}
			else delayR2--;


			if (dpadDelay <= 0)
			{
				dpadDelay = 6;
				int dir = -1;
				bool dpadUpPressed = SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_DPAD_UP);
				bool dpadDownPressed = SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
				bool dpadLeftPressed = SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
				bool dpadRightPressed = SDL_GetGamepadButton(controller, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);

				if (dpadUpPressed && dpadLeftPressed) dir = 3;
				else if (dpadUpPressed && dpadRightPressed) dir = 1;
				else if (dpadDownPressed && dpadLeftPressed) dir = 5;
				else if (dpadDownPressed && dpadRightPressed) dir = 7;
				else if (dpadUpPressed) dir = 2;
				else if (dpadDownPressed) dir = 6;
				else if (dpadLeftPressed) dir = 4;
				else if (dpadRightPressed) dir = 0;

				if (dir != -1)
				{
					int dx, dy;
					dir2Coord(dir, dx, dy);
					changeAimTarget(aimCoord.x + dx, aimCoord.y + dy);
				}
			}
			else dpadDelay--;
		}
	}


	void executeTabShot()
	{
		Entity* victimEntity = TileEntity(aimCoord.x, aimCoord.y, aimCoord.z);
		int targetX = aimCoord.x;
		int targetY = aimCoord.y;
		int targetZ = aimCoord.z;
		int weaponRange = 10;

		// 사격 손 설정
		if (pistolMode == pistolAimMode::DUAL)
		{
			if (dualSelected == equipHandFlag::left) PlayerPtr->aimWeaponLeft();
			else PlayerPtr->aimWeaponRight();
		}
		else if (pistolMode != pistolAimMode::NONE)
		{
			if (pistolHand == equipHandFlag::left) PlayerPtr->aimWeaponLeft();
			else PlayerPtr->aimWeaponRight();
		}
		else
		{
			PlayerPtr->aimWeaponLeft();
		}

		if (victimEntity == nullptr)
		{
			return;
		}
		//맨손 사거리 이탈
		else if (PlayerPtr->getAimWeaponIndex() == -1)
		{
			if (1 < myMax(abs(PlayerX() - targetX), abs(PlayerY() - targetY)))
			{
				return;
			}

			PlayerPtr->startAtk(targetX, targetY, targetZ);
			turnWait(1.0);
			PlayerPtr->initAimStack();

		}
		//무기 사거리 이탈
		else
		{
			if (targetAtkType == atkType::throwing) //던지기
			{
				if (weaponRange < myMax(abs(PlayerX() - targetX), abs(PlayerY() - targetY)) ) { return; }

				//자기 자신에게 던지는 경우도 고려해야 되나?
				std::unique_ptr<ItemPocket> drop = std::make_unique<ItemPocket>(storageType::null);
				updateLog(replaceStr(sysStr[414], L"(%item)", PlayerEquip()->itemInfo[PlayerPtr->getAimWeaponIndex()].name));
				PlayerEquip()->transferItem(drop.get(), PlayerPtr->getAimWeaponIndex(), 1);
				PlayerPtr->throwing(std::move(drop), targetX, targetY);
				PlayerPtr->updateStatus();


				PlayerPtr->startAtk(targetX, targetY, targetZ, aniFlag::throwing);
				turnWait(1.0);
				PlayerPtr->initAimStack();
				PlayerPtr->setNextAtkType(targetAtkType);
			}
			else if (targetAtkType == atkType::shot) //사격
			{
				ItemData& tmpAimWeapon = PlayerEquip()->itemInfo[PlayerPtr->getAimWeaponIndex()];
				if (getBulletNumber(tmpAimWeapon) > 0) //사격인데 총알이 없을 경우
				{
					if (weaponRange < myMax(abs(PlayerX() - targetX), abs(PlayerY() - targetY))) { return; }
				}
				else return;

				//직탄식 총
				if (itemDex[tmpAimWeapon.pocketOnlyItem[0]].checkFlag(itemFlag::AMMO))
				{
					tmpAimWeapon.pocketPtr->popTopBullet();
				}
				//탄창식 총
				else if (itemDex[tmpAimWeapon.pocketOnlyItem[0]].checkFlag(itemFlag::MAGAZINE))
				{
					ItemData& magazineData = tmpAimWeapon.pocketPtr.get()->itemInfo[0];
					magazineData.pocketPtr->popTopBullet();
				}

				auto pEquip = PlayerEquip();
				if (pEquip->itemInfo[0].checkFlag(itemFlag::BOW)) PlayerPtr->setSpriteIndex(charSprIndex::WALK);

				PlayerPtr->startAtk(targetX, targetY, targetZ, aniFlag::shotSingle);
				turnWait(1.0);
				PlayerPtr->initAimStack();
				//사격 후 반동: 조준 진행도를 일부 되돌림
				aimProgress = myMax(0.0, aimProgress - (double)itemDex[getAimWeaponCode()].gunRebound * 0.3);
				aimAcc = calcAimAcc();
				PlayerPtr->setNextAtkType(targetAtkType);

				// 쌍권총: 사격 후 Aim 닫기 & 다음 손 전환
				if (pistolMode == pistolAimMode::DUAL)
				{
					dualNextLeft = (dualSelected != equipHandFlag::left);
					close(aniFlag::null);
					return;
				}
			}
			else//근접공격
			{
				if (weaponRange < myMax(abs(PlayerX() - targetX), abs(PlayerY() - targetY))) { return; }

				PlayerPtr->startAtk(targetX, targetY, targetZ);
				turnWait(1.0);
				PlayerPtr->initAimStack();
				PlayerPtr->setNextAtkType(targetAtkType);
			}
		}
	}
};



