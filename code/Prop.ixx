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
    int alphaFakeHPBar = 0;
public:

    ItemData leadItem;
    float turnResource = 0;
    Light* myLight = nullptr;
    int displayHPBarCount = 0; //양수 200으로 설정시 점점 떨어지다가 1이 되면 alpha를 대신 줄임. alpha마저 모두 줄면 0으로
    int alphaHPBar = 0;

    Prop(int inputX, int inputY, int inputZ, int leadItemCode);

    ~Prop();

    void updateSprIndex();

    bool runAI();

    bool runAnimation(bool shutdown);

    void drawSelf() override;
};

