﻿#include <SDL.h>
#include <SDL_image.h>

export module Entity;

import std;
import constVar;
import textureVar;
import Ani;
import Coord;
import Sprite;
import Drawable;
import ItemStack;
import ItemPocket;
import EntityData;
import util;
import Vehicle;
import Light;

export class Entity : public Ani, public Coord, public Drawable 
{
private:
    std::unique_ptr<Sprite> customSprite = nullptr;
    std::unique_ptr<Sprite> spriteFlash = nullptr; //플래시용 흰색 마스킹 스프라이트
    SDL_Color flash = { 0,0,0,0 }; //플래시 컬러
    int flashType = 0; // 0 : NULL, 1 : white, 2 : white->red
    bool hasAStarDst = false;
    Point2 aStarDst = { 0, 0 };
    Point3 atkTarget = { 0,0,0 };
    Point3 skillTarget = { 0,0,0 };
    atkType nextAtkType = atkType::bash; //다음 공격에 사용할 공격의 타입
    int atkTargetPart = -1;
    equipHandFlag aimWeaponHand = equipHandFlag::right;//현재 적에게 겨누는 주무기
    unsigned __int8 aimStack = 0; //다음 공격에 가산될 조준 스택
    bool footChanged = false;
    bool leftFoot = true; //걷기 애니메이션에서의 왼발, 오른발 순서

public:
    std::unique_ptr<Entity> ridingEntity = nullptr; //탑승중인 엔티티
    ridingFlag ridingType = ridingFlag::none;

    EntityData entityInfo;
    Entity(int newEntityIndex, int gridX, int gridY, int gridZ);
    virtual ~Entity();

    Vehicle* pulledCart = nullptr;


    void setSkillTarget(int gridX, int gridY, int gridZ);
    Point3 getSkillTarget();
    void addSkill(int index);

    unsigned __int8 getAimStack();
    void initAimStack();
    void addAimStack();

    void setNextAtkType(atkType inputAtkType);
    atkType getNextAtkType();
    void setAtkTarget(int inputX, int inputY, int inputZ, int inputPart);
    void setAtkTarget(int inputX, int inputY, int inputZ);
    ItemPocket* getEquipPtr();
    //void addEquipFromDex(int index, equip inputState);
    void updateSpriteFlash();
    Sprite* getSpriteFlash();
    void setFlashType(int inputType);
    int getFlashType();
    bool getLeftFoot();
    void setLeftFoot(bool input);
    void setSpriteInfimum(int inputVal);
    int getSpriteInfimum();
    void setSpriteIndex(int index);
    int getSpriteIndex();
    void setDirection(int dir);

    virtual void startAtk(int inputGridX, int inputGridY, int inputGridZ, aniFlag inputAniType);
    float endAtk();
    void loadDataFromDex(int index);
    void addDmg(int inputDmg);
    void updateStatus();
    int getRPierce(int inputPartIndex);
    int getRCut(int inputPartIndex);
    int getRBash(int inputPartIndex);
    int getSH();
    int getEV();
    int getEnc(int inputPartIndex);
    bool getHasAStarDst();
    void deactAStarDst();
    int getAStarDstX();
    int getAStarDstY();
    void setAStarDst(int inputX, int inputY);

    void move(int dir, bool jump);
    void attack(int gridX, int gridY);
    void updateWalkable(int gridX, int gridY);
    void rayCasting(int x1, int y1, int x2, int y2);
    void rayCastingDark(int x1, int y1, int x2, int y2);
    void stepEvent();
    void startFlash(int inputFlashType);
    void setFlashRGBA(Uint8 inputR, Uint8 inputG, Uint8 inputB, Uint8 inputAlpha);
    void getFlashRGBA(Uint8& targetR, Uint8& targetG, Uint8& targetB, Uint8& targetAlpha);
    void drop(ItemPocket* txPtr);
    void throwing(ItemPocket* txPtr, int gridX, int gridY);

    float getProficLevel(int index);
    void addProficExp(int expVal);

    virtual void endMove() = 0;
    bool runAnimation(bool shutdown);

    virtual void death() = 0;

    void aimWeaponRight();
    void aimWeaponLeft();
    equipHandFlag getAimHand();
    int getAimWeaponIndex();

    void updateCustomSpriteHuman();

    void pullEquipLights();

    virtual void drawSelf() override;
};
