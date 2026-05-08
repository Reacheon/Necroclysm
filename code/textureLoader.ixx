module;
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

export module textureLoader;

import std;
import util;
import globalVar;
import constVar;
import textureVar;
import Sprite;
import paletteLoader; // PaletteTable, loadPaletteTable
import nervedriveFilter;
import procGen;       // procGen::shoreSplineMask 데이터 채우기 위함

// source PNG의 픽셀을 from 팔레트 -> to 팔레트로 치환한 새 SDL_Texture 반환.
// 매칭 안 되는 픽셀/투명 픽셀은 그대로 유지.
static SDL_Texture* paletteSwapTexture(SDL_Renderer* r, SDL_Surface* src, const std::vector<SDL_Color>& from, const std::vector<SDL_Color>& to)
{
	SDL_Surface* dst = SDL_ConvertSurface(src, SDL_PIXELFORMAT_RGBA32);
	if (dst == nullptr) return nullptr;

	SDL_LockSurface(dst);
	Uint8* bytes = (Uint8*)dst->pixels;
	int slotCount = (int)std::min(from.size(), to.size());
	for (int y = 0; y < dst->h; y++)
	{
		Uint8* row = bytes + y * dst->pitch;
		for (int x = 0; x < dst->w; x++)
		{
			Uint8* p = row + x * 4; // RGBA32: R,G,B,A
			if (p[3] == 0) continue; // 완전 투명은 건너뜀
			for (int s = 0; s < slotCount; s++)
			{
				if (p[0] == from[s].r && p[1] == from[s].g && p[2] == from[s].b)
				{
					p[0] = to[s].r;
					p[1] = to[s].g;
					p[2] = to[s].b;
					break;
				}
			}
		}
	}
	SDL_UnlockSurface(dst);

	SDL_Texture* tex = SDL_CreateTextureFromSurface(r, dst);
	SDL_DestroySurface(dst);
	if (tex) SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
	return tex;
}

// 지정 폴더(재귀)의 모든 PNG를 sourceColor 팔레트로 가정하고, palette TSV의 모든 색상 컬럼에
// 대해 팔레트 스왑 결과를 spriteMapper[L"<stem>_<색>"]로 등록.
// (sourceColor 컬럼은 원본과 동일한 결과를 만들지만 일관성 위해 동일 경로로 처리.)
//   - 헤어:  imageDir="image/charset/body/hair", palettePath="palette/hair.tsv", sourceColor=L"BLACK"
//   - 눈:    imageDir="image/charset/body/eyes", palettePath="palette/eyes.tsv", sourceColor=L"BLUE"
//   - 피부:  imageDir="image/charset/body/skin", palettePath="palette/skin.tsv", sourceColor=L"LIGHT"
static void loadPaletteSwappedSprites(SDL_Renderer* renderer, const std::string& imageDir, const std::string& palettePath, const std::wstring& sourceColor)
{
	PaletteTable pal = loadPaletteTable(palettePath);
	auto fromIt = pal.table.find(sourceColor);
	if (fromIt == pal.table.end()) return;

	namespace fs = std::filesystem;
	for (const auto& entry : fs::recursive_directory_iterator(imageDir))
	{
		if (!entry.is_regular_file()) continue;
		if (entry.path().extension() != ".png") continue;

		std::wstring stem = entry.path().stem();
		SDL_Surface* src = IMG_Load(entry.path().string().c_str());
		if (src == nullptr) continue;

		for (const auto& colorName : pal.colorNames)
		{
			auto toIt = pal.table.find(colorName);
			if (toIt == pal.table.end()) continue;

			SDL_Texture* tex = paletteSwapTexture(renderer, src, fromIt->second, toIt->second);
			if (tex == nullptr) continue;
			spr::spriteMapper[stem + L"_" + colorName] = new Sprite(renderer, tex, 48, 48, true);
		}
		SDL_DestroySurface(src);
	}
}

// 돌연변이 폴더 재귀 로드. _FURCOL / _HORNCOL 접미사면 팔레트 스왑하여 색상별 변종 등록.
// 나머지는 일반 로드. 수동 그린 색상 변종(예: MUT_HORN_RED.png)은 팔레트 생성 변종과 충돌 시 수동 우선.
static void loadMutationSprites(SDL_Renderer* renderer)
{
	PaletteTable furPal = loadPaletteTable("palette/fur.tsv");
	PaletteTable hornPal = loadPaletteTable("palette/horn.tsv");

	namespace fs = std::filesystem;
	std::vector<fs::path> templates; // _FURCOL / _HORNCOL 파일들

	// 1패스: 템플릿이 아닌 PNG들을 먼저 로드 (수동 색상 변종 포함)
	for (const auto& entry : fs::recursive_directory_iterator("image/charset/mutation"))
	{
		if (!entry.is_regular_file()) continue;
		if (entry.path().extension() != ".png") continue;

		std::wstring stem = entry.path().stem();
		if (stem.ends_with(L"_FURCOL") || stem.ends_with(L"_HORNCOL"))
		{
			templates.push_back(entry.path());
			continue;
		}
		spr::spriteMapper[stem] = new Sprite(renderer, entry.path().string(), 48, 48);
	}

	// 2패스: 템플릿 PNG들을 팔레트 스왑하여 색상별 변종 생성
	auto processTemplate = [&](const fs::path& path, const PaletteTable& pal, const std::wstring& suffix, const std::wstring& sourceColor)
	{
		std::wstring stem = path.stem();
		std::wstring base = stem.substr(0, stem.size() - suffix.size()); // "_FURCOL" 제거

		auto fromIt = pal.table.find(sourceColor);
		if (fromIt == pal.table.end()) return; // 소스 팔레트 없음

		SDL_Surface* src = IMG_Load(path.string().c_str());
		if (src == nullptr) return;

		for (const auto& colorName : pal.colorNames)
		{
			std::wstring key = base + colorName; // base는 이미 '_'로 끝남
			if (spr::spriteMapper.find(key) != spr::spriteMapper.end()) continue; // 수동 변종 우선

			auto toIt = pal.table.find(colorName);
			if (toIt == pal.table.end()) continue;

			SDL_Texture* tex = paletteSwapTexture(renderer, src, fromIt->second, toIt->second);
			if (tex == nullptr) continue;
			spr::spriteMapper[key] = new Sprite(renderer, tex, 48, 48, true);
		}
		SDL_DestroySurface(src);
	};

	for (const auto& tpath : templates)
	{
		std::wstring stem = tpath.stem();
		if (stem.ends_with(L"_FURCOL"))
			processTemplate(tpath, furPal, L"FURCOL", L"GRAY");
		else if (stem.ends_with(L"_HORNCOL"))
			processTemplate(tpath, hornPal, L"HORNCOL", L"BROWN");
	}
}

export void textureLoader()
{
	//load texture
	texture::minimap = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, MINIMAP_DIAMETER, MINIMAP_DIAMETER);
	SDL_SetTextureScaleMode(texture::minimap, SDL_SCALEMODE_NEAREST);
	texture::navimap = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, NAVIMAP_WIDTH, NAVIMAP_HEIGHT);
	SDL_SetTextureScaleMode(texture::navimap, SDL_SCALEMODE_NEAREST);
	texture::worldmap = IMG_LoadTexture(renderer, "image/worldmap.png");
	SDL_SetTextureScaleMode(texture::worldmap, SDL_SCALEMODE_NEAREST);
	texture::mainGaugeWhiteShadow = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 170, 16);
	SDL_SetTextureScaleMode(texture::mainGaugeWhiteShadow, SDL_SCALEMODE_NEAREST);

	texture::hpGaugeWhiteShadow = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 95, 13);
	SDL_SetTextureScaleMode(texture::hpGaugeWhiteShadow, SDL_SCALEMODE_NEAREST);

	texture::circuitInfo = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 450, 69);

	texture::shoreSpline[0] = IMG_LoadTexture(renderer, "image/spline/shoreSpline0.png");

	// shoreSpline PNG 픽셀 → procGen::shoreSplineMask bool 마스크로 변환 (Sector_procGenerate 페이즈 2가 룩업).
	//   #5b4940 = land (B=0x40), #3899ff = water (B=0xff). B 채널만 비교 (안티앨리어싱 견고).
	//   8×6 그리드 47 셀 (마지막 1칸 공백).
	{
		SDL_Surface* surf = IMG_Load("image/spline/shoreSpline0.png");
		if (surf == nullptr)
		{
			prt(L"[shoreSpline] IMG_Load FAILED: %S\n", SDL_GetError());
		}
		else
		{
			// 항상 RGBA32로 변환 (포맷 균일화). 이미 RGBA32여도 문제 없음.
			SDL_Surface* rgba = SDL_ConvertSurface(surf, SDL_PIXELFORMAT_RGBA32);
			SDL_DestroySurface(surf);
			if (rgba == nullptr)
			{
				prt(L"[shoreSpline] SDL_ConvertSurface FAILED: %S\n", SDL_GetError());
			}
			else
			{
				SDL_LockSurface(rgba);
				const std::uint8_t* bytes = static_cast<const std::uint8_t*>(rgba->pixels);
				const int pitch = rgba->pitch;
				int landCount = 0;

				for (int idx = 0; idx < procGen::SHORE_INDEX_COUNT; ++idx)
				{
					const int cellX = idx % 8;
					const int cellY = idx / 8;
					const int baseX = cellX * procGen::SHORE_TILE_SIZE;
					const int baseY = cellY * procGen::SHORE_TILE_SIZE;

					for (int ly = 0; ly < procGen::SHORE_TILE_SIZE; ++ly)
					{
						for (int lx = 0; lx < procGen::SHORE_TILE_SIZE; ++lx)
						{
							const std::uint8_t* p = bytes + (baseY + ly) * pitch + (baseX + lx) * 4;
							// B 채널 단순 비교: land(B=0x40) vs water(B=0xff). 차이가 커 안티앨리어싱 견고.
							const bool isLand = (p[2] < 128);
							procGen::shoreSplineMask[idx][ly * procGen::SHORE_TILE_SIZE + lx] = isLand;
							if (isLand) ++landCount;
						}
					}
				}
				const int imgW = rgba->w;
				const int imgH = rgba->h;
				SDL_UnlockSurface(rgba);
				prt(L"[shoreSpline] loaded: %d land tiles in 47 cells (image %dx%d, pitch %d)\n", landCount, imgW, imgH, pitch);
				SDL_DestroySurface(rgba);
			}
		}
	}

	// 너브드라이브 초록 틴트용 오프스크린 RT 생성
	nervedriveFilter::init();

	spr::charsetHero = new Sprite(renderer, "image/charset/baseCharset.png", 48, 48);//new Sprite(renderer, "image/charset/baseCharset.png");
	spr::defaultMonster = new Sprite(renderer, "image/charset/zombie1.png", 48, 48);
	spr::effectBash1 = new Sprite(renderer, "image/effect/effectBash1.png", 48, 48);
	spr::effectCut1 = new Sprite(renderer, "image/effect/effectCut1.png", 48, 48);
	spr::effectCut2 = new Sprite(renderer, "image/effect/effectCut2.png", 48, 48);
	spr::effectPierce1 = new Sprite(renderer, "image/effect/effectPierce1.png", 48, 48);
	spr::effectBite1 = new Sprite(renderer, "image/effect/effectBite1.png", 48, 48);
	spr::effectClaw1 = new Sprite(renderer, "image/effect/effectClaw1.png", 48, 48);
	spr::icon13 = new Sprite(renderer, "image/UI/icon13.png", 13, 13);
	spr::icon48 = new Sprite(renderer, "image/UI/icon48.png", 48, 48);
	spr::batteryGauge = new Sprite(renderer, "image/UI/batteryGauge.png", 32, 46);
	spr::ecliptic = new Sprite(renderer, "image/UI/GUI/HUD/ecliptic.png", 76, 36);
	spr::weather = new Sprite(renderer, "image/UI/weather.png", 48, 48);
	spr::weatherCloud = new Sprite(renderer, "image/UI/weatherCloud.png", 48, 48);
	spr::itemset = new Sprite(renderer, "image/item/itemset.png", 48, 48);
	spr::colorPaletteOption = new Sprite(renderer, "image/UI/GUI/colorPaletteOption.png", 16, 16);
	spr::windowArrow = new Sprite(renderer, "image/UI/windowArrow.png", 16, 16);
	spr::whiteMarker = new Sprite(renderer, "image/UI/whiteMarker.png", 16, 16);
	spr::yellowMarker = new Sprite(renderer, "image/UI/yellowMarker.png", 16, 16);
	spr::itemCursorLong = new Sprite(renderer, "image/UI/itemCursorLong.png", 360, 64);
	spr::itemCursorShort = new Sprite(renderer, "image/UI/itemCursorShort.png", 304, 64);
	spr::lootBagArrow = new Sprite(renderer, "image/UI/lootBagArrow.png", 32, 32);
	spr::icon16 = new Sprite(renderer, "image/UI/icon16.png", 16, 16);
	spr::guideBtn = new Sprite(renderer, "image/UI/guideBtn.png", 87, 33);
	spr::buttons = new Sprite(renderer, "image/UI/buttons.png", 48, 48);
	spr::buttonsPressed = new Sprite(renderer, "image/UI/buttonsPressed.png", 48, 48);
	spr::menuPopUp = new Sprite(renderer, "image/UI/menuPopUp.png", 35, 36);
	spr::letterboxBtnMarker = new Sprite(renderer, "image/UI/barButtonMarker.png", 108, 108);
	spr::proficIcon = new Sprite(renderer, "image/UI/proficIcon.png", 16, 16);
	spr::proficIconGold = new Sprite(renderer, "image/UI/proficIconGold.png", 16, 16);
	spr::bionicSkeleton = new Sprite(renderer, "image/UI/bionicSkeleton.png", 36, 72);
	spr::bionicSlotGauge = new Sprite(renderer, "image/UI/bionicSlotGauge.png", 64, 10);
	spr::mutationIcon = new Sprite(renderer, "image/UI/mutationIcon.png", 16, 16);
	spr::staminaGauge = new Sprite(renderer, "image/UI/staminaGauge.png", 48, 48);
	spr::segment = new Sprite(renderer, "image/UI/segment.png", 12, 16);
	spr::tileset = new Sprite(renderer, "image/tileset/tileset.png", 16, 16);
	spr::propset = new Sprite(renderer, "image/tileset/propset.png", 48, 48);
	spr::icon32 = new Sprite(renderer, "image/UI/icon32.png", 32, 32);
	spr::ring24 = new Sprite(renderer, "image/UI/ring24.png", 24, 24);
	spr::bloodM = new Sprite(renderer, "image/effect/bloodM1.png", 48, 48);
	spr::aimMarker = new Sprite(renderer, "image/UI/aimMarker.png", 48, 48);
	spr::aimMarkerTmp = new Sprite(renderer, "image/UI/aimMarkerTmp.png", 48, 48);

	spr::aimAtkTypeMarker = new Sprite(renderer, "image/UI/aimAtkTypeMarker.png", 64, 64);
	spr::tab = new Sprite(renderer, "image/UI/GUI/HUD/tab.png", 180, 180);
	spr::aimLRChange = new Sprite(renderer, "image/UI/aimLRChange.png", 18, 18);
	spr::epsilonFont = new Sprite(renderer, "image/epsilonFont.png", 4, 6);
	spr::loadingAnime = new Sprite(renderer, "image/UI/loadingAnime.png", 32, 32);
	spr::buildCursor = new Sprite(renderer, "image/UI/buildCursor.png", 48, 48);
	spr::msgChoiceBtn = new Sprite(renderer, "image/UI/msgChoiceBtn.png", 134, 68);

	spr::itemSlotBtn = new Sprite(renderer, "image/UI/item/itemSlotBtn.png", 210, 18);
	spr::itemSlotPocketArrow = new Sprite(renderer, "image/UI/item/itemSlotBtn.png", 8, 40);
	spr::lstSelectBox = new Sprite(renderer, "image/UI/GUI/lstSelectBox.png", 360, 44);// (240, 29)->(360,44)

	spr::beardMustacheBlack = new Sprite(renderer, "image/charset/body/beardMustacheBlack.png", 48, 48);

	spr::hornCoverRed = new Sprite(renderer, "image/charset/body/hornCoverRed.png", 48, 48);

	spr::shadow = new Sprite(renderer, "image/charset/shadow.png", 48, 48);

	spr::vehicleHUD = new Sprite(renderer, "image/UI/GUI/Vehicle/vehicleHUD.png", 900, 580);
	spr::dashboard = new Sprite(renderer, "image/UI/GUI/Vehicle/dashboard.png", 720, 580);
	spr::vehicleHUDParts = new Sprite(renderer, "image/UI/GUI/Vehicle/vehicleHUDParts.png", 64, 64);
	spr::vehicleHUDSteeringWheel = new Sprite(renderer, "image/UI/GUI/Vehicle/vehicleHUDSteeringWheel.png", 256, 256);
	spr::vehicleActCursor = new Sprite(renderer, "image/UI/GUI/Vehicle/vehicleActCursor.png", 72, 72);

	spr::dirMarker = new Sprite(renderer, "image/UI/dirMarker.png", 48, 48);
	spr::windowArrow2 = new Sprite(renderer, "image/UI/GUI/windowArrow2.png", 48, 48);
	spr::mapHereMarker = new Sprite(renderer, "image/UI/GUI/Map/mapHereMarker.png", 23, 23);

	spr::screenRain = new Sprite(renderer, "image/weather/screenRain.png", 720, 720);
	spr::screenSnow = new Sprite(renderer, "image/weather/screenSnow.png", 720, 720);

	spr::symbolSunny = new Sprite(renderer, "image/UI/GUI/HUD/symbolSunny.png", 48, 48);
	spr::symbolCloudy = new Sprite(renderer, "image/UI/GUI/HUD/symbolCloudy.png", 48, 48);
	spr::symbolMoon = new Sprite(renderer, "image/UI/GUI/HUD/symbolMoon.png", 48, 48);
	spr::symbolRain = new Sprite(renderer, "image/UI/GUI/HUD/symbolRain.png", 48, 48);
	spr::symbolStorm = new Sprite(renderer, "image/UI/GUI/HUD/symbolStorm.png", 48, 48);
	spr::symbolSnow = new Sprite(renderer, "image/UI/GUI/HUD/symbolSnow.png", 48, 48);

	spr::tailPedalL = new Sprite(renderer, "image/UI/GUI/HUD/tailPedalL.png", 64, 64);
	spr::tailPedalR = new Sprite(renderer, "image/UI/GUI/HUD/tailPedalR.png", 64, 64);
	spr::trainBrake = new Sprite(renderer, "image/UI/GUI/HUD/trainBrake.png", 80, 80);

	spr::mainRotor = new Sprite(renderer, "image/mainRotor3.png", 128, 128);

	for (const auto& entry : std::filesystem::directory_iterator("image/charset/equip"))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".png")
		{
			spr::spriteMapper[entry.path().stem()] = new Sprite(renderer, entry.path().string(), 48, 48);
		}
	}

	loadPaletteSwappedSprites(renderer, "image/charset/body/hair", "palette/hair.tsv", L"BLACK");
	loadPaletteSwappedSprites(renderer, "image/charset/body/eyes", "palette/eyes.tsv", L"BLUE");
	loadPaletteSwappedSprites(renderer, "image/charset/body/skin", "palette/skin.tsv", L"LIGHT");
	loadMutationSprites(renderer);

	for (const auto& entry : std::filesystem::directory_iterator("image/charset"))
	{
		if (entry.is_regular_file() && entry.path().extension() == ".png")
		{
			spr::spriteMapper[entry.path().stem()] = new Sprite(renderer, entry.path().string(), 48, 48);
		}
	}

	spr::singleQuickSlot = new Sprite(renderer, "image/UI/GUI/HUD/singleQuickSlot.png", 180, 38);
	spr::topQuickSlotBtn = new Sprite(renderer, "image/UI/GUI/HUD/topQuickSlotBtn.png", 43, 38);

	spr::skillSet = new Sprite(renderer, "image/UI/skillSet.png", 16, 16);

	spr::fireStorm = new Sprite(renderer, "image/effect/skill/fireStorm.png", 80, 96);

	spr::flameSet = new Sprite(renderer, "image/tileset/flameSet.png", 48, 48);

	spr::gasEffect1 = new Sprite(renderer, "image/gas/gasEffect1.png", 48, 48);
	spr::steamEffect1 = new Sprite(renderer, "image/gas/steamEffect1.png", 48, 48);

	spr::trail = new Sprite(renderer, "image/trail.png", 16, 16);

	spr::statusEffectRect = new Sprite(renderer, "image/UI/GUI/HUD//statusEffectRect.png", 130, 34);
	spr::statusIcon = new Sprite(renderer, "image/UI/statusIcon.png", 16, 16);

	spr::minimapEdge = new Sprite(renderer, "image/UI/GUI/HUD//minimapEdge.png", 244, 244);

	spr::bulletset = new Sprite(renderer, "image/item/bulletset.png", 48, 48);
	spr::aimMarkerWhite = new Sprite(renderer, "image/UI/aimMarkerWhite.png", 48, 48);

	spr::mapMagnifyIcon = new Sprite(renderer, "image/UI/GUI/Map/mapMagnifyIcon.png", 128, 40);
	spr::staminaGaugeCircle = new Sprite(renderer, "image/UI/GUI/HUD/staminaGaugeCircle.png", 48, 48);
	spr::speechBubble = new Sprite(renderer, "image/UI/GUI/speechBubble.png", 320, 137);
	spr::particle = new Sprite(renderer, "image/particle.png", 16, 16);
	spr::footprint = new Sprite(renderer, "image/footprint.png", 16, 16);
	spr::craftItemRect = new Sprite(renderer, "image/UI/craftItemRect.png", 58, 58);
	spr::msgBox = new Sprite(renderer, "image/UI/msgBox.png", 300, 236);

	spr::skillRect = new Sprite(renderer, "image/UI/GUI/Skill/skillRect.png", 250, 34);

	spr::gridMarker = new Sprite(renderer, "image/UI/gridMarker.png", 16, 16);

	spr::tabBoxAim = new Sprite(renderer, "image/UI/tabBoxAim.png", 122, 122);

	spr::youDied = new Sprite(renderer, "image/UI/youDied.png", 1440, 184);

	spr::gameOverOptionRect = new Sprite(renderer, "image/UI/gameOverOptionRect.png", 220, 160);
	spr::gameOverOptionMarker = new Sprite(renderer, "image/UI/gameOverOptionMarker.png", 156, 48);

	spr::statusEffectGaugeCircle = new Sprite(renderer, "image/UI/GUI/HUD/gauge28.png", 28, 28);
	spr::thoughtBubble = new Sprite(renderer, "image/thoughtBubble.png", 16, 16);
	spr::waveFoam = new Sprite(renderer, "image/waveFoam.png", 16, 16);
	spr::seaFoam = new Sprite(renderer, "image/seaFoam.png", 16, 16);
	spr::waterFoam = new Sprite(renderer, "image/waterFoam.png", 16, 16);

    spr::itemBackgroundRect = new Sprite(renderer, "image/UI/GUI/itemBackgroundRect.png", 68, 68);

    spr::newWindowArrow = new Sprite(renderer, "image/UI/GUI/newWindowArrow.png", 30, 52);

    spr::icon28 = new Sprite(renderer, "image/UI/icon28.png", 28, 28);
    spr::downRightLetterbox = new Sprite(renderer, "image/UI/GUI/HUD/downRightLetterbox.png", 782, 176);
	spr::mainGauge = new Sprite(renderer, "image/UI/GUI/HUD/mainGauge.png", 170, 16);

    spr::bodyShape = new Sprite(renderer, "image/UI/GUI/HUD/bodyShape.png", 100, 256);
    spr::hpGauge = new Sprite(renderer, "image/UI/GUI/HUD/hpGauge.png", 95, 13);

    spr::icon24 = new Sprite(renderer, "image/UI/icon24.png", 24, 24);

    spr::gamepadInstruction = new Sprite(renderer, "image/UI/gamepadInstruction.png", 176, 112);
	spr::gamepadButtons = new Sprite(renderer, "image/UI/gamepadButtons.png", 48, 48);
    spr::keyboardButtons = new Sprite(renderer, "image/UI/keyboardButtons.png", 48, 48);

	spr::icon80 = new Sprite(renderer, "image/UI/icon80.png", 80, 80);

    spr::gearStick = new Sprite(renderer, "image/UI/GUI/Vehicle/gearStick.png", 80, 144);

	spr::coordCraftBox = new Sprite(renderer, "image/UI/GUI/coordCraftBox.png", 234, 134);
	spr::coordCraftMarker = new Sprite(renderer, "image/UI/GUI/coordCraftMarker.png", 80, 80);

	spr::fluxArrow = new Sprite(renderer, "image/UI/GUI/HUD/fluxArrow.png", 48, 48);

	spr::sprinkler33 = new Sprite(renderer, "image/sprinkler33.png", 112, 112);
	spr::sprinkler55 = new Sprite(renderer, "image/sprinkler55.png", 112, 112);

	spr::logBackground = new Sprite(renderer, "image/UI/GUI/HUD/logBackground.png", 378, 80);
	spr::floatGuideLog = new Sprite(renderer, "image/UI/GUI/HUD/floatGuideLog.png", 378, 80);

	spr::fryingPan = new Sprite(renderer, "image/UI/GUI/Cook/fryingPan.png", 160, 160);
	spr::cookingPot = new Sprite(renderer, "image/UI/GUI/Cook/cookingPot.png", 160, 160);

	spr::bodyPartEncLine = new Sprite(renderer, "image/UI/GUI/HUD/bodyPartEncLine.png", 106, 28);

	spr::btnGuideBackground = new  Sprite(renderer, "image/UI/GUI/btnGuideBackground.png", 426, 66);
	spr::statusPortraitBackground = new  Sprite(renderer, "image/UI/GUI/statusPortraitBackground.png", 110, 110);

}