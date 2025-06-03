#include <SDL3/SDL.h>
#include <sol/sol.hpp>

import Monster;
import std;
import Entity;
import globalVar;
import wrapVar;
import textureVar;
import Player;
import World;
import log;
import Corpse;
import util;
import AI;
import ItemData;

import drawSprite;
import drawText;
import drawPrimitive;


Monster::Monster(int index, int gridX, int gridY, int gridZ) : Entity(index, gridX, gridY, gridZ)
{
	entityCode = index;
	if (entityCode == entityRefCode::zombieA)
	{
		entityFlip = true;

		parts.emplace_back(PartData{ .partName = L"몸통", .accRate = 1.0f, .maxHP = 100, .currentHP = 100, .resPierce = 0, .resCut = 0, .resBash = 0 });
		parts.emplace_back(PartData{ .partName = L"머리", .accRate = 0.6f, .maxHP = 100, .currentHP = 100, .resPierce = 0, .resCut = 0, .resBash = 0 });
		parts.emplace_back(PartData{ .partName = L"왼팔", .accRate = 0.9f, .maxHP = 100, .currentHP = 100, .resPierce = 0, .resCut = 0, .resBash = 0 });
		parts.emplace_back(PartData{ .partName = L"오른팔", .accRate = 0.9f, .maxHP = 100, .currentHP = 100, .resPierce = 0, .resCut = 0, .resBash = 0 });
		parts.emplace_back(PartData{ .partName = L"왼다리", .accRate = 0.7f, .maxHP = 100, .currentHP = 100, .resPierce = 0, .resCut = 0, .resBash = 0 });
		parts.emplace_back(PartData{ .partName = L"오른다리", .accRate = 0.7f, .maxHP = 100, .currentHP = 100, .resPierce = 0, .resCut = 0, .resBash = 0 });
	}


	//AI 로드
	{
		std::string filepath = "testAI.lua";
		std::ifstream file(filepath);
		if (!file.is_open()) throw std::runtime_error("Could not open file: " + filepath);
		std::stringstream buffer;
		buffer << file.rdbuf();
		scriptAI = buffer.str();
	}

	prt(name.c_str());
	prt(lowCol::red, L"\nMonster : 생성자가 호출되었습니다! ID : %p\n", this);
}
Monster::~Monster()
{
	prt(lowCol::red, L"Monster : 소멸자가 호출되었습니다..\n");
}

void Monster::startAtk(int inputGridX, int inputGridY, int inputGridZ, aniFlag inputAniType)
{
	Entity::startAtk(inputGridX, inputGridY, inputGridZ, inputAniType);
	addAniUSet(this, inputAniType);
}

void Monster::startAtk(int inputGridX, int inputGridY, int inputGridZ) { startAtk(inputGridX, inputGridY, inputGridZ, aniFlag::atk); }

void Monster::endMove()
{
}

bool Monster::runAI()
{
	lua["selfX"] = [this]()->int {return this->getGridX(); };
	lua["selfY"] = [this]()->int {return this->getGridY(); };
	lua["selfZ"] = [this]()->int {return this->getGridZ(); };

	lua["getMyTurn"] = [this]()->int {return this->getTurnResource(); };
	lua["addMyTurn"] = [this](double val)->void { this->addTurnResource(val); };
	lua["clearMyTurn"] = [this]()->void { this->clearTurnResource(); };
	lua["useMyTurn"] = [this](double val)->void { this->useTurnResource(val); };

	lua["isHostile"] = [this]()->bool { return this->relation == relationFlag::hostile; };
	lua["getEntityCode"] = [this]()->int { return this->entityCode; };
	lua["updateWalkable"] = [this](int x, int y)->void { updateWalkable(x, y); };



	lua["setDirection"] = [this](int dir)->void { this->setDirection(dir); };
	lua["moveStart"] = [this](int dir, bool jump)->void { this->move(dir, jump); };
	lua["attackStart"] = [this](int x, int y, int z)->void { this->startAtk(x, y, z); };



	try
	{
		sol::object result = lua.script(scriptAI);
		return result.as<bool>();
	}
	catch (const std::exception& e) { std::cerr << "Error: " << e.what() << std::endl; }

}

void Monster::death()
{
	if (aniUSet.find(this) != aniUSet.end())
	{
		prt(lowCol::red, L"[Monster] runAnimation 강제 종료\n");
		runAnimation(true);
	}
	new Corpse(corpseType::bloodM, getX(), getY(), getGridZ());
	std::unique_ptr<ItemPocket> dropItems = std::make_unique<ItemPocket>(storageType::temp);
	dropItems.get()->addItemFromDex(20, 1);
	drop(dropItems.get());
	World::ins()->getTile({ getGridX(),getGridY(),getGridZ() }).EntityPtr.reset();
}


void Monster::drawSelf()
{
	setZoom(zoomScale);
	if (entityFlip == false) setFlip(SDL_FLIP_NONE);
	else setFlip(SDL_FLIP_HORIZONTAL);
	int drawingX = (cameraW / 2) + zoomScale * (getX() - cameraX + getIntegerFakeX());
	int drawingY = (cameraH / 2) + zoomScale * (getY() - cameraY + getIntegerFakeY());
	if (entityCode == entityRefCode::zombieA)
	{
		int sprIndex = 0;
		if (entityFlip == false)
		{
			if (sprState == sprFlag::walkRight) sprIndex = 2;
			else if (sprState == sprFlag::walkLeft) sprIndex = 1;
		}
		else
		{
			if (sprState == sprFlag::walkRight) sprIndex = 1;
			else if (sprState == sprFlag::walkLeft) sprIndex = 2;
		}
		if (sprState == sprFlag::attack1) sprIndex = 7;
		else if (sprState == sprFlag::attack2) sprIndex = 8;

		bool partSelect = TileEntity(getAbsMouseGrid().x, getAbsMouseGrid().y, PlayerZ()) != nullptr&& (std::abs(getAbsMouseGrid().x - PlayerX()) == 1 || std::abs(getAbsMouseGrid().y - PlayerY()) == 1);
		static Uint64 flashStartTime = 0;
		static bool wasFlashing = false;
		static Uint64 flashEndTime = 0;
		static bool flashJustEnded = false;

		if (flash.a > 0) {
			flashJustEnded = true;
		}
		else if (flashJustEnded) {
			flashEndTime = SDL_GetTicks();
			flashJustEnded = false;
		}

		SDL_Color targetFlashColor = { 255, 255, 255, 0 };
		Uint64 currentTime = SDL_GetTicks();
		if (partSelect && selectedPart != -1 && turnCycle == turn::playerInput && flash.a == 0 && (currentTime - flashEndTime) > 500)
		{
			if (!wasFlashing) flashStartTime = currentTime;
			wasFlashing = true;
			Uint64 elapsedTime = currentTime - flashStartTime;
			float pulseSpeed = 0.0025f;
			float pulse = (sin(elapsedTime * pulseSpeed) + 1.0f) * 0.5f;
			Uint8 alpha = (Uint8)(0 + pulse * 120);
			targetFlashColor = { 255, 255, 255, alpha };
		}
		else wasFlashing = false;

		auto drawPartWithFlash = [&](auto sprite, const std::wstring& partName) {
			if (selectedPart != -1 && parts[selectedPart].partName == partName)
			{
				if (flash.a > 0)
					drawFlashEffectCenter(sprite, sprIndex, drawingX, drawingY, flash);
				else if (partSelect && turnCycle == turn::playerInput)
					drawFlashEffectCenter(sprite, sprIndex, drawingX, drawingY, targetFlashColor);
			}
			};

		auto drawLimbWithFlash = [&](const std::wstring& partName, auto normalSprite, auto flippedSprite) {
			if (getPart(partName)->currentHP > 0)
			{
				auto spriteToUse = entityFlip ? flippedSprite : normalSprite;
				drawSpriteCenter(spriteToUse, sprIndex, drawingX, drawingY);
				drawPartWithFlash(spriteToUse, partName);
			}
			};

		drawSpriteCenter(spr::shadow, 1, drawingX, drawingY);

		drawSpriteCenter(spr::zombieA::torso, sprIndex, drawingX, drawingY);
		drawPartWithFlash(spr::zombieA::torso, L"몸통");

		if (getPart(L"머리")->currentHP > 0)
		{
			drawSpriteCenter(spr::zombieA::head, sprIndex, drawingX, drawingY);
			drawPartWithFlash(spr::zombieA::head, L"머리");
		}
		drawLimbWithFlash(L"왼다리", spr::zombieA::lLeg, spr::zombieA::rLeg);
		drawLimbWithFlash(L"오른다리", spr::zombieA::rLeg, spr::zombieA::lLeg);
		drawLimbWithFlash(L"왼팔", spr::zombieA::lArm, spr::zombieA::rArm);
		drawLimbWithFlash(L"오른팔", spr::zombieA::rArm, spr::zombieA::lArm);

		if (flash.a > 0)
		{
			if (selectedPart == -1) drawFlashEffectCenter(spr::zombieA::whole, sprIndex, drawingX, drawingY, flash);
			SDL_Color tgtCol = { 255, 0, 0, flash.a };
			float speed = 0.15f;
			flash.r = flash.r + (tgtCol.r - flash.r) * speed;
			flash.g = flash.g + (tgtCol.g - flash.g) * speed;
			flash.b = flash.b + (tgtCol.b - flash.b) * speed;
			flash.a = (Uint8)(flash.a * 0.91f);
			if (flash.a < 5) flash.a = 0;
		}
	}
	setZoom(1.0);
	setFlip(SDL_FLIP_NONE);
}