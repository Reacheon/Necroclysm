#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

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
import util;
import Vehicle;
import Light;
import SkillData;


export struct PartData
{
    std::wstring partName = L"머리";
    float accRate = 0.2f;
    int maxHP = 100;
    int currentHP = 100;
    int resCut = 0.0f; //절단 저항
    int resBash = 0.0f; //타격 저항
    int resPierce = 0.0f; //관통 저항
};

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

    std::unique_ptr<ItemPocket> throwingItemPocket;
    Point3 throwCoord = { 0,0,0 };



public:
    std::wstring name = L"DEFAULT ENTITY";
    unsigned __int16 entityCode = 1;
    std::unique_ptr<Entity> ridingEntity = nullptr; //탑승중인 엔티티
    ridingFlag ridingType = ridingFlag::none;
    std::vector<Point2> aStarData;
    bool entityFlip = false; //현재 이 객체의 좌우반전 여부
    int stamina = 100; //기력
    int maxStamina = 100; //최대 기력   
    std::array<int, TALENT_SIZE> proficExp = { 0, }; //경험치
    std::array<float, TALENT_SIZE> proficApt = { 2.0,2.0,2.0,2.0,2.0,2.0,2.0,2.0,2.0, }; //적성
    std::array<int, TALENT_SIZE> proficFocus = { 0, }; //집중도 0:미분배, 1:소분배, 2:일반분배
    int eyeSight = 8; //기본 시야 범위
    std::vector<std::pair<statEfctFlag, int>> statusEffects;

    __int16 sh = 0;
    __int16 ev = 0;
    __int16 rFire = 0;
    __int16 rCold = 0;
    __int16 rElec = 0;
    __int16 rCorr = 0;
    __int16 rRad = 0;

    __int16 shRef = 0;
    __int16 evRef = 0;
    __int16 rFireRef = 0;
    __int16 rColdRef = 0;
    __int16 rElecRef = 0;
    __int16 rCorrRef = 0;
    __int16 rRadRef = 0;

    std::vector<SkillData> skillList;
    walkFlag walkMode = walkFlag::walk;
    relationFlag relation = relationFlag::hostile;
    __int16 HP = 100;
    __int16 MP = 100;
    __int16 maxHP = 100;
    __int16 maxMP = 100;
    __int16 fakeHP = 100;
    unsigned __int8 fakeHPAlpha = 255;
    __int16 fakeMP = 100;
    unsigned __int8 fakeMPAlpha = 255;

    bool isPlayer = false;
    __int8 direction = 0;
    std::unique_ptr<ItemPocket> equipment;
    __int16 sprIndex = 0;
    __int16 sprIndexInfimum = 0;
    humanCustom::skin skin = humanCustom::skin::null;
    humanCustom::eyes eyes = humanCustom::eyes::null;
    humanCustom::scar scar = humanCustom::scar::null;
    humanCustom::beard beard = humanCustom::beard::null;
    humanCustom::hair hair = humanCustom::hair::null;
    humanCustom::horn horn = humanCustom::horn::null;

    Sprite* entitySpr = nullptr;
    double gridMoveSpd = 3.0;//그리드와 그리드 사이를 넘어갈 때의 속도
    __int8 hpBarHeight = -12;

    //////////////////////////////////////////////////
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
};
