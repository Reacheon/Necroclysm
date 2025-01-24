export module Prop;

import std;
import globalVar;
import wrapVar;
import textureVar;
import constVar;
import util;
import World;
import ItemPocket;
import ItemData;
import Ani;
import AI;
import Light;
import Coord;
import log;
import Drawable;
import drawSprite;
import globalTime;
import Player;

export class Prop : public Ani, public AI, public Coord, public Drawable
{
public:
    ItemData leadItem;
    float turnResource = 0;
    Light* myLight = nullptr;

    Prop(int inputX, int inputY, int inputZ, int leadItemCode);

    ~Prop();

    void updateSprIndex();

    void updateTile();

    bool runAI();

    bool runAnimation(bool shutdown);

    void drawSelf() override;
};

