export module Prop;

import std;
import constVar;
import util;
import ItemData;
import Ani;
import AI;
import Light;
import Coord;
import Drawable;

export class Prop : public Ani, public AI, public Coord, public Drawable
{
private:
    
public:

    ItemData leadItem;
    int displayHPBarCount = 0; //양수 200으로 설정시 점점 떨어지다가 1이 되면 alpha를 대신 줄임. alpha마저 모두 줄면 0으로
    int alphaHPBar = 0;
    int alphaFakeHPBar = 0;
    float treeAngle = 0.0; //벌목 때 나무들이 가지는 앵글, 0이 아닐 경우 활성화됨

    bool runUsed = false; //runProp
    
    double current = 0.0f; //전류
	double elecResistance = 0.0f; //전기저항

	double flow = 0.0f; //유량
	double flowResistance = 0.0f; //흐름저항
	int fluidCode = 0; //유체 아이템 코드

    Prop(Point3 inputCoor, int leadItemCode);

    ~Prop();

    void setGrid(int inputGridX, int inputGridY, int inputGridZ) override;

    void updateSprIndex();

    bool runAI();

    void runPropFunc()
    {
        //prt(L"[Prop:runProp] ID : %p의 runProp를 실행시켰다.\n", this);

		if (runUsed) return;

        if (leadItem.checkFlag(itemFlag::CIRCUIT))
        {
        }

		runUsed = true;
    }

    bool runAnimation(bool shutdown);

    void drawSelf() override;
};

