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
    int displayHPBarCount = 0; //��� 200���� ������ ���� �������ٰ� 1�� �Ǹ� alpha�� ��� ����. alpha���� ��� �ٸ� 0����
    int alphaHPBar = 0;

    Prop(int inputX, int inputY, int inputZ, int leadItemCode);

    ~Prop();

    void updateSprIndex();

    bool runAI();

    bool runAnimation(bool shutdown);

    void drawSelf() override;
};

