#include <SDL.h>
#include <SDL_image.h>

export module textureLoader;

import std;
import globalVar;
import constVar;
import textureVar;
import Sprite;

export void textureLoader()
{
	//load texture
	texture::minimap = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, minimapDiameter, minimapDiameter);
	texture::worldmap = IMG_LoadTexture(renderer, "image/worldmap.png");

	spr::charsetHero = new Sprite(renderer, "image/charset/baseCharset.png", 48, 48);//new Sprite(renderer, "image/charset/baseCharset.png");
	spr::defaultItem = new Sprite(renderer, "image/orbTexture.png", 48, 48);
	spr::defaultMonster = new Sprite(renderer, "image/charset/zombie1.png", 48, 48);
	spr::zombie1 = new Sprite(renderer, "image/charset/zombie1.png", 48, 48);
	spr::blockTexture = new Sprite(renderer, "image/block.png", 16, 32);
	spr::tileTexture = new Sprite(renderer, "image/tile.png", 16, 16);
	spr::effectBash1 = new Sprite(renderer, "image/effect/effectBash1.png", 48, 48);
	spr::effectCut1 = new Sprite(renderer, "image/effect/effectCut1.png", 48, 48);
	spr::effectCut2 = new Sprite(renderer, "image/effect/effectCut2.png", 48, 48);
	spr::effectPierce1 = new Sprite(renderer, "image/effect/effectPierce1.png", 48, 48);
	spr::effectBite1 = new Sprite(renderer, "image/effect/effectBite1.png", 48, 48);
	spr::effectClaw1 = new Sprite(renderer, "image/effect/effectClaw1.png", 48, 48);
	spr::icon13 = new Sprite(renderer, "image/UI/icon13.png", 13, 13);
	spr::icon48 = new Sprite(renderer, "image/UI/icon48.png", 48, 48);
	spr::batteryGauge = new Sprite(renderer, "image/UI/batteryGauge.png", 32, 46);
	spr::ecliptic = new Sprite(renderer, "image/UI/ecliptic.png", 72, 35);
	spr::weather = new Sprite(renderer, "image/UI/weather.png", 72, 35);
	spr::weatherCloud = new Sprite(renderer, "image/UI/weatherCloud.png", 72, 35);
	spr::itemset = new Sprite(renderer, "image/item/itemset.png", 48, 48);
	spr::windowArrow = new Sprite(renderer, "image/UI/windowArrow.png", 16, 16);
	spr::cursorMarker = new Sprite(renderer, "image/UI/blueMarker.png", 16, 16);
	spr::yellowMarker = new Sprite(renderer, "image/UI/yellowMarker.png", 16, 16);
	spr::itemCursorLong = new Sprite(renderer, "image/UI/itemCursorLong.png", 304, 64);
	spr::itemCursorShort = new Sprite(renderer, "image/UI/itemCursorShort.png", 304, 64);
	spr::lootBagArrow = new Sprite(renderer, "image/UI/lootBagArrow.png", 32, 32);
	spr::icon16 = new Sprite(renderer, "image/UI/icon16.png", 16, 16);
	spr::guideBtn = new Sprite(renderer, "image/UI/guideBtn.png", 87, 33);
	spr::buttons = new Sprite(renderer, "image/UI/buttons.png", 48, 48);
	spr::buttonsPressed = new Sprite(renderer, "image/UI/buttonsPressed.png", 48, 48);
	spr::menuPopUp = new Sprite(renderer, "image/UI/menuPopUp.png", 35, 36);
	spr::floatLog = new Sprite(renderer, "image/UI/floatLog.png", 500, 40);
	spr::letterboxBtnMarker = new Sprite(renderer, "image/UI/barButtonMarker.png", 72, 72);
	spr::talentIcon = new Sprite(renderer, "image/UI/talentIcon.png", 16, 16);
	spr::talentIconGold = new Sprite(renderer, "image/UI/talentIconGold.png", 16, 16);
	spr::bionicGauge = new Sprite(renderer, "image/UI/bionicGauge.png", 72, 72);
	spr::bionicSkeleton = new Sprite(renderer, "image/UI/bionicSkeleton.png", 36, 72);
	spr::bionicSlotGauge = new Sprite(renderer, "image/UI/bionicSlotGauge.png", 64, 10);
	spr::mutationIcon = new Sprite(renderer, "image/UI/mutationIcon.png", 16, 16);
	spr::constructPreviewBase = new Sprite(renderer, "image/UI/construct/constructPreviewBase.png", 100, 100);
	spr::constructPreviewWall = new Sprite(renderer, "image/UI/construct/constructPreviewWall.png", 100, 100);
	spr::constructPreviewFloor = new Sprite(renderer, "image/UI/construct/constructPreviewFloor.png", 100, 100);
	spr::constructPreviewCeil = new Sprite(renderer, "image/UI/construct/constructPreviewCeil.png", 100, 100);
	spr::constructPreviewEtc = new Sprite(renderer, "image/UI/construct/constructPreviewEtc.png", 100, 100);
	spr::constructCategory = new Sprite(renderer, "image/UI/construct/constructCategory.png", 86, 22);
	spr::staminaGauge = new Sprite(renderer, "image/UI/staminaGauge.png", 48, 48);
	spr::segment = new Sprite(renderer, "image/UI/segment.png", 12, 16);
	spr::tileset = new Sprite(renderer, "image/tileset/tileset.png", 16, 16);
	spr::propset = new Sprite(renderer, "image/tileset/propset.png", 48, 48);
	spr::icon32 = new Sprite(renderer, "image/UI/icon32.png", 32, 32);
	spr::ring24 = new Sprite(renderer, "image/UI/ring24.png", 24, 24);
	spr::bloodM = new Sprite(renderer, "image/effect/bloodM1.png", 48, 48);
	spr::bodyTmpZombie = new Sprite(renderer, "image/charFrame/bodyTmpZombie.png", 240, 128);
	spr::aimMarker = new Sprite(renderer, "image/UI/aimMarker.png", 48, 48);
	spr::aimMarkerTmp = new Sprite(renderer, "image/UI/aimMarkerTmp.png", 48, 48);
	spr::aimAtkTypeMarker = new Sprite(renderer, "image/UI/aimAtkTypeMarker.png", 64, 64);
	spr::bodyTmpHuman = new Sprite(renderer, "image/charFrame/bodyTmpHuman.png", 240, 128);
	spr::partsSlotGauge = new Sprite(renderer, "image/UI/partsSlotGauge.png", 64,10);
	spr::tab = new Sprite(renderer, "image/UI/tabBox.png", 122, 122);
	spr::aimLRChange = new Sprite(renderer, "image/UI/aimLRChange.png", 18, 18);
	spr::floatWarning = new Sprite(renderer, "image/UI/floatWarning.png", 128, 24);
	spr::epsilonFont = new Sprite(renderer, "image/epsilonFont.png", 4, 6);
	spr::loadingAnime = new Sprite(renderer, "image/UI/loadingAnime.png", 32, 32);
	spr::buildCursor = new Sprite(renderer, "image/UI/buildCursor.png", 48, 48);
	spr::propset = new Sprite(renderer, "image/tileset/propset.png", 48, 48);
	spr::msgChoiceBtn = new Sprite(renderer, "image/UI/msgChoiceBtn.png",  90, 46);

	spr::itemSlotBtn = new Sprite(renderer, "image/UI/item/itemSlotBtn.png", 210, 18);
	spr::itemSlotPocketArrow = new Sprite(renderer, "image/UI/item/itemSlotBtn.png", 8, 40);
	spr::vehicleTemplate = new Sprite(renderer, "image/UI/GUI/vehicleTemplate.png", 710, 376);
	spr::vehicleCenterCursor = new Sprite(renderer, "image/UI/GUI/vehicleCenterCursor.png", 16, 16);
	spr::lstSelectBox = new Sprite(renderer, "image/UI/GUI/lstSelectBox.png", 240, 29);
	spr::alchemyArrow = new Sprite(renderer, "image/UI/GUI/Alchemy/alchemyArrow.png", 228, 52);
	spr::alchemyPicture = new Sprite(renderer, "image/UI/GUI/Alchemy/alchemyPicture.png", 200, 100);
	spr::alchemyStart = new Sprite(renderer, "image/UI/GUI/Alchemy/alchemyStart.png", 202, 74);
	spr::alchemyMaterialEdge = new Sprite(renderer, "image/UI/GUI/Alchemy/alchemyMaterialEdge.png", 226, 290);

	spr::skinYellow = new Sprite(renderer, "image/charset/body/skinYellow.png", 48, 48);
	spr::eyesBlue = new Sprite(renderer, "image/charset/body/eyesBlue.png", 48, 48);
	spr::eyesRed = new Sprite(renderer, "image/charset/body/eyesRed.png", 48, 48);
	spr::beardMustacheBlack = new Sprite(renderer, "image/charset/body/beardMustacheBlack.png", 48, 48);

	spr::hairCommaBlack = new Sprite(renderer, "image/charset/body/hairCommaBlack.png", 48, 48);
	spr::hairBob1Black = new Sprite(renderer, "image/charset/body/hairBob1Black.png", 48, 48);
	spr::hairPonytailBlack = new Sprite(renderer, "image/charset/body/hairPonytailBlack.png", 48, 48);
	spr::hairMiddlePart = new Sprite(renderer, "image/charset/body/hairMiddlePart.png", 48, 48);

	spr::shadow = new Sprite(renderer, "image/charset/shadow.png", 48, 48);

	spr::vehicleHUD = new Sprite(renderer, "image/UI/GUI/Vehicle/vehicleHUD.png", 720, 580);
	spr::vehicleHUDParts = new Sprite(renderer, "image/UI/GUI/Vehicle/vehicleHUDParts.png", 64, 64);
	spr::vehicleHUDSteeringWheel = new Sprite(renderer, "image/UI/GUI/Vehicle/vehicleHUDSteeringWheel.png", 256, 256);
	spr::vehicleActCursor = new Sprite(renderer, "image/UI/GUI/Vehicle/vehicleActCursor.png", 72, 72);

	spr::dirMarker = new Sprite(renderer, "image/UI/dirMarker.png", 48, 48);
	spr::windowArrow2 = new Sprite(renderer, "image/UI/GUI/windowArrow2.png", 48, 48);
	spr::mapHereMarker = new Sprite(renderer, "image/UI/GUI/Map/mapHereMarker.png", 23, 23);

	spr::screenRain = new Sprite(renderer, "image/weather/screenRain.png",720,720);
	spr::screenSnow = new Sprite(renderer, "image/weather/screenSnow.png",720,720);

	spr::symbolSunny = new Sprite(renderer, "image/UI/GUI/HUD/symbolSunny.png",48,48);
	spr::symbolCloudy = new Sprite(renderer, "image/UI/GUI/HUD/symbolCloudy.png",48,48);
	spr::symbolMoon = new Sprite(renderer, "image/UI/GUI/HUD/symbolMoon.png",48,48);
	spr::symbolRain = new Sprite(renderer, "image/UI/GUI/HUD/symbolRain.png",48,48);
	spr::symbolStorm = new Sprite(renderer, "image/UI/GUI/HUD/symbolStorm.png",48,48);
	spr::symbolSnow = new Sprite(renderer, "image/UI/GUI/HUD/symbolSnow.png",48,48);

	spr::tailPedalL = new Sprite(renderer, "image/UI/GUI/HUD/tailPedalL.png", 64, 64);
	spr::tailPedalR = new Sprite(renderer, "image/UI/GUI/HUD/tailPedalR.png", 64, 64);
	spr::trainBrake = new Sprite(renderer, "image/UI/GUI/HUD/trainBrake.png", 80, 80);

	spr::mainRotor = new Sprite(renderer, "image/mainRotor.png", 128, 128);

	for (const auto& entry : std::filesystem::directory_iterator("image/charset/equip"))
	{
		spr::equipMapper[entry.path().stem()] = new Sprite(renderer, entry.path().string(), 48, 48);
	}

	spr::singleQuickSlot = new Sprite(renderer, "image/UI/GUI/HUD/singleQuickSlot.png", 180, 38);
	spr::topQuickSlotBtn = new Sprite(renderer, "image/UI/GUI/HUD/topQuickSlotBtn.png", 43, 38);

	spr::skillSet = new Sprite(renderer, "image/UI/skillSet.png", 16, 16);

	spr::fireStorm = new Sprite(renderer, "image/effect/skill/fireStorm.png", 80, 96);

	spr::flameSet = new Sprite(renderer, "image/tileset/flameSet.png", 48, 48);

	spr::gasEffect1 = new Sprite(renderer, "image/gas/gasEffect1.png", 48, 48);
	spr::steamEffect1 = new Sprite(renderer, "image/gas/steamEffect1.png", 48, 48);

}