#include <SDL.h>

export module GUI;

import std;
import util;
import Ani;
import Coord;
import globalVar;
import wrapVar;
import Player;


export class GUI : public Ani
{
private:
	inline static std::vector<GUI*> activeGUIList;
	bool stateDraw = true;
	bool stateInput = true;
	int slipDir = 6;
	float foldRatio = 1.0;
	bool isCorouter = false;

	int distCounter = 100;
	float distSpd = 1.0;
public:
	int x, y;
	GUI(bool corouterInput) : isCorouter(corouterInput)
	{
		if (isCorouter) coAnswer = L"";
		activeGUIList.push_back(this);
		prt(L"★ GUI : 생성자가 호출되었습니다. actvieGUIList의 크기는 %d입니다.\n", activeGUIList.size());
	}
	virtual ~GUI() //가상 소멸자(없으면 자식 객체의 소멸자 미실행으로 메모리 누수 일어남)
	{
		activeGUIList.erase(std::find(activeGUIList.begin(), activeGUIList.end(), this));
		prt(L"★ GUI : 소멸자가 호출되었습니다.. actvieGUIList의 크기는 %d입니다.\n", activeGUIList.size());
		if (isCorouter) (*coFunc).run();
	};
	static std::vector<GUI*> getActiveGUIList() { return activeGUIList; }
	static GUI* getLastGUI() { return activeGUIList[activeGUIList.size() - 1]; }
	void setDistCounter(float val)
	{
		distCounter = val;
	}
	float getFoldRatio() { return foldRatio; }
    void setAniSlipDir(int inputDir) { slipDir = inputDir; }
    void actDraw() { stateDraw = true; }
    void deactDraw() { stateDraw = false; }
    void actInput() { stateInput = true; }
    void deactInput() { stateInput = false; }
    bool getStateDraw() { return stateDraw; }
    bool getStateInput() { return stateInput; }
	void close(aniFlag aniType)
	{
		if (aniType == aniFlag::winUnfoldClose)
		{
			deactInput();
			addAniUSetPlayer(this, aniFlag::winUnfoldClose);
		}
		else if (aniType == aniFlag::winSlipClose)
		{
			deactInput();
			addAniUSetPlayer(this, aniFlag::winSlipClose);
		}
		else if (aniType == aniFlag::null)
		{
			delete this;
		}
		else//잘못된 창 닫기 애니메이션 입력
		{
			errorBox("wrong aniFlag parameter in GUI close function");
		}
	}
    virtual void changeXY(int inputX, int inputY, bool center) = 0;
    virtual void drawGUI() = 0;
    virtual void clickUpGUI() = 0;
	virtual void clickMotionGUI(int dx, int dy) = 0;
	virtual void clickDownGUI() = 0;
	virtual void clickRightGUI() = 0;
	virtual void clickHoldGUI() = 0;
	virtual void mouseWheel() = 0;
	virtual void gamepadBtnDown() = 0;
	virtual void gamepadBtnMotion() = 0;
	virtual void gamepadBtnUp() = 0;
	virtual void step() = 0;
    virtual bool runAnimation(bool shutdown)
	{
		const int acc = 20;
		const int initSpeed = 4;
		int sgnX;
		int sgnY;
		switch (slipDir)
		{
			default:
				errorBox("Diagonal GUI movement occured");
				break;
			case 0:
				sgnX = 1;
				sgnY = 0;
				break;
			case 2:
				sgnX = 0;
				sgnY = -1;
				break;
			case 4:
				sgnX = -1;
				sgnY = 0;
				break;
			case 6:
				sgnX = 0;
				sgnY = 1;
				break;
		}

		if (getAniType() == aniFlag::winSlipOpen)//만약 플레이어 인스턴스의 좌표와 목적좌표가 다를 경우
		{
			static std::array<int, 10> spd = { 0, };//spd[0]은 0프레임일 때의 속도
			static int initDist = 0;
			addTimer();
			switch (getTimer())
			{
				case 1:
					actDraw();
					initDist = 0;
					for (int i = 9; i >= 0; i--)
					{
						spd[9 - i] = acc * i + initSpeed;
						initDist += spd[9 - i];
					}
					changeXY(x + sgnX * initDist, y + sgnY * initDist, false);
					break;
				default:
					changeXY(x - sgnX * spd[getTimer() - 2], y - sgnY * spd[getTimer() - 2], false);
					break;
				case 12:
					resetTimer();
					actInput();
					setAniType(aniFlag::null);
					return true;
					break;
			}
		}
		else if (getAniType() == aniFlag::winSlipClose)
		{
			static std::array<int, 10> spd = { 0, };//spd[0]은 0프레임일 때의 속도
			addTimer();
			switch (getTimer())
			{
				case 1:
					for (int i = 9; i >= 0; i--)
					{
						spd[i] = acc * i + initSpeed;
					}
					break;
				default:
					changeXY(x + sgnX * spd[getTimer() - 2], y + sgnY * spd[getTimer() - 2], false);
					break;
				case 12:
					resetTimer();
					setAniType(aniFlag::null);
					delete this;
					return true;
					break;
			}
		}
		else if (getAniType() == aniFlag::winUnfoldOpen)
		{
			
			const float acc = 0.06;
			addTimer();
			if (getTimer() == 1)
			{
				actDraw();
				foldRatio = 0;
			}
			else
			{
				foldRatio += acc * getTimer();
				if (foldRatio >= 1.0)
				{
					foldRatio = 1.0;
					resetTimer();
					actInput();
					setAniType(aniFlag::null);
					return true;
				}
			}
		}
		else if (getAniType() == aniFlag::winUnfoldClose)
		{
			const float acc = 0.06;
			addTimer();
			if (getTimer() == 1)
			{
				actDraw();
				foldRatio = 1.0;
			}
			else
			{
				foldRatio -= acc * getTimer();
				if (foldRatio <= 0)
				{
					foldRatio = 0;
					resetTimer();
					setAniType(aniFlag::null);
					delete this;
					return true;
				}
			}
		}


		return false;
	}
};