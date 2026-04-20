module;
#include <SDL3/SDL.h>

export module Entity;

import std;
import util;
import constVar;
import Ani;
import Coord;
import Drawable;
import Sprite;
import Drawable;
import EntityData;
import Vehicle;
import ItemPocket;

export class Entity : public Ani, public Coord, public Drawable 
{
private:
    std::unique_ptr<Sprite> customSprite = nullptr;
    bool hasAStarDst = false;
    Point2 aStarDst = { 0, 0 };
    Point3 atkTarget = { 0,0,0 };
    Point3 skillTarget = { 0,0,0 };
    atkType nextAtkType = atkType::bash; //다음 공격에 사용할 공격의 타입
    int atkTargetPart = -1;
    equipHandFlag aimWeaponHand = equipHandFlag::right;//현재 적에게 겨누는 주무기
    equipHandFlag meleeAtkHand = equipHandFlag::none;//현재 근접공격에 사용하는 손
    unsigned __int8 aimStack = 0; //다음 공격에 가산될 조준 스택
    bool footChanged = false;
    bool leftFoot = true; //걷기 애니메이션에서의 왼발, 오른발 순서

    std::unique_ptr<ItemPocket> throwingItemPocket;
    Point3 throwCoord = { 0,0,0 };

public:
    std::unique_ptr<Entity> ridingEntity = nullptr; //탑승중인 엔티티
    ridingFlag ridingType = ridingFlag::none;
    std::vector<Point2> aStarData;
    SDL_Color flash = { 0,0,0,0 };

    // Nervedrive 잔상 데이터
    struct AfterImage {
        int worldX, worldY;
        int fakeX, fakeY;
        int sprIndex;
        bool flip;
        float angle;
        int jumpOffsetY;
        int offsetX, offsetY;
        Uint32 captureTime;
    };
    std::deque<AfterImage> afterImages;
    Uint32 lastAfterImageTime = 0;

    EntityData entityInfo;
    Entity(int newEntityIndex, int gridX, int gridY, int gridZ);
    virtual ~Entity();

    Vehicle* pulledCart = nullptr;


    void setSkillTarget(int gridX, int gridY, int gridZ);
    Point3 getSkillTarget();
    void addSkill(const std::wstring& skillId);
    void removeSkill(const std::wstring& skillId);

    unsigned __int8 getAimStack();
    void initAimStack();
    void addAimStack();

    void setNextAtkType(atkType inputAtkType);
    atkType getNextAtkType();
    void setAtkTarget(int inputX, int inputY, int inputZ, int inputPart);
    void setAtkTarget(int inputX, int inputY, int inputZ);
    ItemPocket* getEquipPtr();
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
    void takeDamage(int inputDmg, dmgFlag inputType, humanPartFlag inputPart = humanPartFlag::torso);
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
    void rayCasting(int x1, int y1, int x2, int y2);
    void rayCastingDark(int x1, int y1, int x2, int y2);
    void stepEvent();
    void drop(ItemPocket* txPtr);
    void throwing(std::unique_ptr<ItemPocket> txPtr, int gridX, int gridY);

    float getProficLevel(int index);
    void addProficExp(int expVal);
    virtual void endMove() = 0;
    bool runAnimation(bool shutdown);

    virtual void death() = 0;

    void aimWeaponRight();
    void aimWeaponLeft();
    equipHandFlag getAimHand();
    int getAimWeaponIndex();

    void pullEquipLights();

    virtual void drawSelf() override;

    // 플레이어 캐릭터 합성 텍스처 생성 (스킨/눈/머리/장비/돌연변이 전 레이어). 반환된 텍스처는 호출자가 SDL_DestroyTexture로 해제.
    SDL_Texture* composePlayerTexture();

    bool hitAnimation(bool shutdown, const std::function<void()>);
};
