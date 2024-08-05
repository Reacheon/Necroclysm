#include <SDL.h>
#include <SDL_image.h>

export module Entity;

import std;
import globalVar;
import constVar;
import textureVar;
import log;
import Ani;
import Coord;
import World;
import Sprite;
import Drawable;
import ItemStack;
import ItemPocket;
import ItemData;
import SkillData;
import EntityData;
import util;
import Vehicle;

export class Entity : public Ani, public Coord, public Drawable {
private:
    Sprite* sprite = nullptr; //메인 스프라이트
    Sprite* spriteFlash = nullptr; //플래시용 흰색 마스킹 스프라이트
    SDL_Color flash = { 0,0,0,0 }; //플래시 컬러
    int spriteIndex = 0;
    int spriteInfimum = 0;
    int flashType = 0; // 0 : NULL, 1 : white, 2 : white->red
    SDL_RendererFlip flip = SDL_FLIP_NONE;
    std::vector<std::array<int, DMG_FLAG_SIZE>> dmgVec;//현재 이 개체가 받은 데미지의 총량
    bool hasAStarDst = false;
    int aStarDstX = 0;
    int aStarDstY = 0;
    int atkTargetGridX = 0;
    int atkTargetGridY = 0;
    int atkTargetGridZ = 0;
    int atkTargetPart = -1;
    atkType nextAtkType = atkType::bash; //다음 공격에 사용할 공격의 타입
    unsigned __int8 aimStack = 0; //다음 공격에 가산될 조준 스택
    int aimWeaponHand = equip::right;//현재 적에게 겨누는 주무기

    humanCustom::skin skin = humanCustom::skin::null;
    humanCustom::eyes eyes = humanCustom::eyes::null;
    humanCustom::scar scar = humanCustom::scar::null;
    humanCustom::beard beard = humanCustom::beard::null;
    humanCustom::hair hair = humanCustom::hair::null;
    
    std::vector<SkillData> bionicList;
    std::vector<SkillData> mutationList;
    std::vector<SkillData> martialArtList;
    std::vector<SkillData> divinePowerList;
    std::vector<SkillData> magicList;
    Point3 skillTarget;
    //애니메이션 실행에서만 사용되는 변수
    bool footChanged = false;
    bool leftFoot = true; //걷기 애니메이션에서의 왼발, 오른발 순서

public:
    EntityData entityInfo;
    Entity(int newEntityIndex, int gridX, int gridY, int gridZ);
    ~Entity();

    Vehicle* pulledCart = nullptr;
    std::vector<SkillData>& getBionicList();
    std::vector<SkillData>& getMutationList();
    std::vector<SkillData>& getMartialArtList();
    std::vector<SkillData>& getDivinePowerList();
    std::vector<SkillData>& getMagicList();

    void setSkillTarget(int gridX, int gridY, int gridZ);
    Point3 getSkillTarget();
    void addSkill(int index);
    int searchBionicCode(int inputCode);
    bool eraseBionicCode(int inputCode);
    bool eraseBionicIndex(int inputIndex);
    int searchMutationCode(int inputCode);
    bool eraseMutationCode(int inputCode);
    bool eraseMutationIndex(int inputIndex);
    int searchMartialArtCode(int inputCode);
    bool eraseMartialArtCode(int inputCode);
    bool eraseMartialArtIndex(int inputIndex);
    int searchDivinePowerCode(int inputCode);
    bool eraseDivinePowerCode(int inputCode);
    bool eraseDivinePowerIndex(int inputIndex);
    int searchMagicCode(int inputCode);
    bool eraseMagicCode(int inputCode);
    bool eraseMagicIndex(int inputIndex);

    humanCustom::skin getSkin();
    void setSkin(humanCustom::skin input);
    humanCustom::eyes getEyes();
    void setEyes(humanCustom::eyes input);
    humanCustom::scar getScar();
    void setScar(humanCustom::scar input);
    humanCustom::beard getBeard();
    void setBeard(humanCustom::beard input);
    humanCustom::hair getHair();
    void setHair(humanCustom::hair input);

    unsigned __int8 getAimStack();
    void initAimStack();
    void addAimStack();

    void setNextAtkType(atkType inputAtkType);
    atkType getNextAtkType();
    void setAtkTarget(int inputX, int inputY, int inputZ, int inputPart);
    void setAtkTarget(int inputX, int inputY, int inputZ);
    ItemPocket* getEquipPtr();
    Sprite* getSpriteFlash();
    void setFlashType(int inputType);
    void setSpriteFlash(Sprite* inputSprite);
    int getFlashType();
    bool getLeftFoot();
    void setLeftFoot(bool input);
    void setSpriteInfimum(int inputVal);
    int getSpriteInfimum();
    SDL_RendererFlip getEntityFlip();
    void setEntityFlip(SDL_RendererFlip inputFlip);
    void setSpriteIndex(int index);
    int getSpriteIndex();
    Sprite* getSprite();
    void setSprite(Sprite* inputSprite);
    void setDirection(int dir);

    virtual void startAtk(int inputGridX, int inputGridY, int inputGridZ, int inputTarget, aniFlag inputAniType);
    float endAtk();
    void loadDataFromDex(int index);
    void addDmg(int inputPartIndex, int inputDmg);
    bool existPart(int inputPartIndex);
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
    void attack(int gridX, int gridY, int inputPartType);
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


    float getTalentLevel(int index);
    void addTalentExp(int expVal);

    virtual void endMove() = 0;
    bool runAnimation(bool shutdown);

    virtual void death() = 0;

    std::vector<int> getAllParts();

    void aimWeaponRight();
    void aimWeaponLeft();
    int getAimHand();
    float getAimAcc(Entity* victimEntity, int inputPartType, bool aim);
    int getAimWeaponIndex();

    //void setPulledVehicle(Vehicle* inputVeh);
    //void releasePulledVehicle();
    //bool hasPulledVehicle();
    //Vehicle* getPulledVehicle();

    virtual void drawSelf() override;
};
