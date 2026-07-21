module;
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

export module TitleScreen;

import std;
import util;
import GUI;
import constVar;
import drawText;
import globalVar;

export class TitleScreen : public GUI
{
private:
	inline static TitleScreen* ptr = nullptr;
	bool savedDrawHUD = true;
	SDL_Texture* logoTex = nullptr; //image/titleLogo.png 픽셀아트 로고 (첫 프레임에 지연 로드)
	bool logoTried = false;
public:
	TitleScreen() : GUI(false)
	{
		//1개 이상의 타이틀 화면 생성 시의 예외 처리
		errorBox(ptr != nullptr, L"More than one TitleScreen instance was generated.");
		ptr = this;

		changeXY(0, 0, false);

		savedDrawHUD = drawHUD;
		drawHUD = false;
	}
	~TitleScreen()
	{
		if (logoTex != nullptr) SDL_DestroyTexture(logoTex);
		drawHUD = savedDrawHUD;
		ptr = nullptr;
	}
	static TitleScreen* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center)
	{
		x = inputX;
		y = inputY;
	}
	void drawGUI()
	{
		drawFillRect(SDL_Rect{ 0, 0, cameraW, cameraH }, col::black);

		if (!logoTried)
		{
			logoTried = true;
			logoTex = IMG_LoadTexture(renderer, "image/titleLogo.png");
			if (logoTex != nullptr) SDL_SetTextureScaleMode(logoTex, SDL_SCALEMODE_NEAREST);
		}
		if (logoTex != nullptr)
		{
			//로고: 정수배 NEAREST 확대로 픽셀 격자 보존, 세로 중심은 화면 30% 지점
			float lw = 0.0f, lh = 0.0f;
			SDL_GetTextureSize(logoTex, &lw, &lh);
			const int scale = std::max(1, int(cameraW * 0.95f / lw));
			const float w = lw * scale, h = lh * scale;
			SDL_FRect dst{ (cameraW - w) / 2.0f, cameraH * 0.30f - h / 2.0f, w, h };
			SDL_RenderTexture(renderer, logoTex, nullptr, &dst);
		}
		else
		{
			//로고 PNG 로드 실패 시 텍스트 타이틀 폴백
			setFont(fontType::mainFontExtraBold);
			setFontSize(48);
			drawTextCenter(L"NECROCLYSM", cameraW / 2, cameraH / 4, col::white);
		}

		setFont(fontType::mainFont);
		setFontSize(15);
	}
	void clickUpGUI()
	{
		//임시: 아무 곳이나 클릭하면 타이틀 화면 닫기
		close(aniFlag::null);
	}
};
