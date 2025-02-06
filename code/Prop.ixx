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
public:
    ItemData leadItem;
    float turnResource = 0;
    Light* myLight = nullptr;

    Prop(int inputX, int inputY, int inputZ, int leadItemCode);

    ~Prop();

    void updateSprIndex();


    bool runAI();

    bool runAnimation(bool shutdown);

    void drawSelf() override;
};

