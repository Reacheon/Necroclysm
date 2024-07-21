#include <SDL.h>
#define DEC_SPR(name) Sprite* name = nullptr;

export module textureVar;

import std;
import Sprite;

export namespace texture
{
	SDL_Texture* minimap = nullptr;
	SDL_Texture* worldmap = nullptr;
}

export namespace spr
{
	Sprite* charsetHero = nullptr;
	Sprite* zombie1 = nullptr;
	Sprite* defaultMonster = nullptr;
	Sprite* defaultItem = nullptr;
	Sprite* blockTexture = nullptr;
	Sprite* tileTexture = nullptr;
	Sprite* equipIcon = nullptr;
	Sprite* effectBash1 = nullptr;
	Sprite* effectCut1 = nullptr;
	Sprite* effectCut2 = nullptr;
	Sprite* effectPierce1 = nullptr;
	Sprite* effectBite1 = nullptr;
	Sprite* effectClaw1 = nullptr;
	Sprite* icon13 = nullptr;
	Sprite* icon48 = nullptr;
	Sprite* batteryGauge = nullptr;
	Sprite* ecliptic = nullptr;
	Sprite* weather = nullptr;
	Sprite* weatherCloud = nullptr;
	Sprite* itemset = nullptr;
	Sprite* windowArrow = nullptr;
	Sprite* cursorMarker = nullptr;
	Sprite* yellowMarker = nullptr;
	Sprite* equipRoll = nullptr;
	Sprite* lootRoll = nullptr;
	Sprite* detailRoll = nullptr;
	Sprite* pocketRoll = nullptr;
	Sprite* itemCursorLong = nullptr;
	Sprite* itemCursorShort = nullptr;
	Sprite* lootBagArrow = nullptr;
	Sprite* icon16 = nullptr;
	Sprite* guideBtn = nullptr;
	Sprite* buttons = nullptr;
	Sprite* buttonsPressed = nullptr;
	Sprite* menuPopUp = nullptr;
	Sprite* floatLog = nullptr;
	Sprite* letterboxBtnMarker = nullptr;
	Sprite* talentIcon = nullptr;
	Sprite* talentIconGold = nullptr;
	Sprite* bionicGauge = nullptr;
	Sprite* bionicSkeleton = nullptr;
	Sprite* bionicSlotGauge = nullptr;
	Sprite* mutationIcon = nullptr;
	Sprite* constructPreviewBase = nullptr;
	Sprite* constructPreviewWall = nullptr;
	Sprite* constructPreviewFloor = nullptr;
	Sprite* constructPreviewCeil = nullptr;
	Sprite* constructPreviewEtc = nullptr;
	Sprite* constructCategory = nullptr;
	Sprite* staminaGauge = nullptr;
	Sprite* segment = nullptr;
	Sprite* tileset = nullptr;
	Sprite* wallset = nullptr;
	Sprite* propset = nullptr;
	Sprite* icon32 = nullptr;
	Sprite* ring24 = nullptr;
	Sprite* bloodM = nullptr;
	Sprite* bodyTmpZombie = nullptr;
	Sprite* aimMarker = nullptr;
	Sprite* aimMarkerTmp = nullptr;
	Sprite* aimAtkTypeMarker = nullptr;
	Sprite* bodyTmpHuman = nullptr;
	Sprite* partsSlotGauge = nullptr;
	Sprite* tab = nullptr;
	Sprite* aimLRChange = nullptr;
	Sprite* floatWarning = nullptr;
	Sprite* epsilonFont = nullptr;
	Sprite* loadingAnime = nullptr;
	Sprite* buildCursor = nullptr;
	Sprite* msgChoiceBtn = nullptr;
	Sprite* itemSlotBtn = nullptr;
	Sprite* itemSlotPocketArrow = nullptr;
	Sprite* vehicleTemplate = nullptr;
	Sprite* vehicleCenterCursor = nullptr;
	Sprite* lstSelectBox = nullptr;

	Sprite* alchemyArrow = nullptr;
	Sprite* alchemyPicture = nullptr;
	Sprite* alchemyStart = nullptr;
	Sprite* alchemyMaterialEdge = nullptr;

	Sprite* skinYellow = nullptr;
	Sprite* eyesBlue = nullptr;
	Sprite* eyesRed = nullptr;
	Sprite* beardMustacheBlack = nullptr;

	Sprite* hairCommaBlack = nullptr;
	Sprite* hairBob1Black = nullptr;
	Sprite* hairPonytailBlack = nullptr;
	Sprite* hairMiddlePart = nullptr;

	Sprite* shadow = nullptr;

	Sprite* vehicleHUD = nullptr;
	Sprite* vehicleHUDParts = nullptr;
	Sprite* vehicleHUDSteeringWheel = nullptr;
	Sprite* vehicleActCursor = nullptr;

	Sprite* dirMarker = nullptr;
	Sprite* craftEdge = nullptr;

	Sprite* craftSelectWindow = nullptr;
	Sprite* windowArrow2 = nullptr;
	Sprite* mapHereMarker = nullptr;

	Sprite* screenRain = nullptr;
	Sprite* screenSnow = nullptr;

	Sprite* symbolSunny = nullptr;
	Sprite* symbolCloudy = nullptr;
	Sprite* symbolMoon = nullptr;
	Sprite* symbolRain = nullptr;
	Sprite* symbolStorm = nullptr;
	Sprite* symbolSnow = nullptr;

	Sprite* tailPedalL = nullptr;
	Sprite* tailPedalR = nullptr;
	Sprite* trainBrake = nullptr;
	Sprite* mainRotor = nullptr;

	std::unordered_map<std::wstring, Sprite*> equipMapper;

	Sprite* singleQuickSlot = nullptr;
	Sprite* topQuickSlotBtn = nullptr;
	Sprite* skillSet = nullptr;

	Sprite* fireStorm = nullptr;
	Sprite* flameSet = nullptr;

	Sprite* gasEffect1 = nullptr;
	Sprite* steamEffect1 = nullptr;
	
	Sprite* inventoryItemRect = nullptr;
}