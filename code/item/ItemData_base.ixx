export module ItemDataBase;

import std;
import util;
import constVar;

export struct ItemDataBase
{
    std::wstring name; //기본 이름을 지정해두면 인텔리센스 오류가 발생함
    std::wstring dir; //프롭의 방향을 표시하는 특수문자
    int itemCode = 1;
    unsigned __int16 itemSprIndex = 1;
    unsigned __int16 tooltipIndex = 0;
    itemCategory category = itemCategory::materials;
    itemSubcategory subcategory = itemSubcategory::material_etc;
    unsigned __int32 weight = 1000;
    unsigned __int32 originalVolume = 1000;
    std::vector<unsigned __int16> material;
    std::vector<itemFlag> flag;

    unsigned __int32 calorie = 0;
    unsigned __int32 hydration = 0;
    float hydrationPerML = 1.0f;

    std::vector<unsigned __int8> toolQuality;
    unsigned __int8 craftMinUnit = 1;
    unsigned __int16 craftTime = 20;
    std::vector<std::pair<unsigned __int16, unsigned __int32>> recipe;
    std::vector<unsigned __int8> recipeQualityNeed;
    std::vector<std::pair<unsigned __int8, unsigned __int8>> recipeProficNeed;
    std::vector<std::pair<unsigned __int16, unsigned __int32>> disassy;
    std::vector<unsigned __int8> disassyQualityNeed;
    std::vector<std::pair<unsigned __int8, unsigned __int8>> disassyProficNeed;
    std::vector<std::pair<unsigned __int16, unsigned __int32>> repair;
    std::vector<unsigned __int8> repairQualityNeed;
    std::vector<std::pair<unsigned __int8, unsigned __int8>> repairProficNeed;
    unsigned __int32 pocketMaxVolume = 0;
    unsigned __int32 pocketMaxNumber = 0;
    std::vector<unsigned __int16> pocketOnlyItem;
    // Torso (몸통)
    __int16 rPierceTorso = 0;
    __int16 rCutTorso = 0;
    __int16 rBashTorso = 0;
    __int8 encTorso = 0;

    // Head (머리)
    __int16 rPierceHead = 0;
    __int16 rCutHead = 0;
    __int16 rBashHead = 0;
    __int8 encHead = 0;

    // Left Arm (왼팔)
    __int16 rPierceLArm = 0;
    __int16 rCutLArm = 0;
    __int16 rBashLArm = 0;
    __int8 encLArm = 0;

    // Right Arm (오른팔)
    __int16 rPierceRArm = 0;
    __int16 rCutRArm = 0;
    __int16 rBashRArm = 0;
    __int8 encRArm = 0;

    // Left Leg (왼다리)
    __int16 rPierceLLeg = 0;
    __int16 rCutLLeg = 0;
    __int16 rBashLLeg = 0;
    __int8 encLLeg = 0;

    // Right Leg (오른다리)
    __int16 rPierceRLeg = 0;
    __int16 rCutRLeg = 0;
    __int16 rBashRLeg = 0;
    __int8 encRLeg = 0;
    std::vector<std::pair<unsigned __int8, __int16>> enc;
    __int16 sh = 0;
    __int16 ev = 0;
    __int16 rFire = 0;
    __int16 rCold = 0;
    __int16 rElec = 0;
    __int16 rCorr = 0;
    __int16 rRad = 0;
    std::wstring bionicId = L"";  //바이오닉 CBM 아이템이 설치하는 스킬 ID. 비어있으면 CBM 아니다.
    __int16 durablility = 100;
    __int16 pierce = 0;
    __int16 cut = 0;
    __int16 bash = 0;
    __int16 bulletPierce = 1;
    __int16 bulletCut = 1;
    __int16 bulletBash = 1;
    float bulletStoppingPower = 0.0f;
    unsigned __int16 bulletJam = 1;
    unsigned __int8 bulletType = 0;
    unsigned __int8 bulletRange = 1;
    float gunDmg = 0.00f;
    float gunAccInit = 0.5f;
    unsigned __int8 gunFullAimTurns = 5; //최대 명중률 도달까지 걸리는 평균 턴 수
    float gunAccMax = 0.99f;
    unsigned __int8 gunOptRange = 5; //최적 사거리 (이 거리에서 명중률 패널티 0)
    unsigned __int8 gunRebound = 5;
    float gunShotSpd = 1.0f;
    float gunReloadSpd = 1.0f;
    float gunJam = 0.001f;
    std::vector<unsigned __int16> gunMod;
    float gunBalance = 0.2f;
    unsigned __int8 gunRange = 1;
    float meleeAtkSpd = 0.5f;
    float meleeAtkAcc = 0.5f;
    float meleeBalance = 0.5f;
    unsigned __int8 meleeRange = 1;
    unsigned __int8 modeTemplate = 0;
    __int16 throwAtk = 1;
    unsigned __int8 throwType = 3; //0:무속성, 1:관통, 2:절단, 3:타격
    float throwAtkAcc = 0.3f;
    float throwBalance = 0.3f;
    unsigned __int8 throwRange = 5;
    float throwStoppingPower = 0.005f;
    unsigned __int16 bookIndex = 0; //책 파일의 이름명, 0이면 작동하지 않음(책이 아닌걸로 취급), 1이면 1.txt를 읽어냄
    unsigned __int16 propSprIndex = 0; //아이템아이콘 말고 실제 Vehicle로 설치되었을 때의 이미지 인덱스
    void* equipSpr = nullptr;
    void* equipSprToggleOn = nullptr;
    void* flipEquipSpr = nullptr;
    void* flipEquipSprToggleOn = nullptr;
    // EQUIP_SPR_GENDERED 시에만 사용. 로드 시점엔 포인터 해석을 못하므로(착용자 성별이 미정),
    // TSV의 베이스명을 그대로 보관해두고 그릴 때 "<base>_<gender>"로 spriteMapper에서 조회한다.
    // 일반 장비는 빈 문자열.
    std::wstring equipSprName;
    std::wstring flipEquipSprName;
    unsigned __int32 equipPriority = 0;
    unsigned __int32 flipEquipPriority = 0;
    void* leftWieldSpr = nullptr;
    unsigned __int32 leftWieldPriority = 0;
    void* rightWieldSpr = nullptr;
    unsigned __int32 rightWieldPriority = 0;
    unsigned __int16 tileSprIndex = 0;
    __int8 tileConnectGroup = -1; //타일이 그려질 때 주변과 연결 여부 (-1:미할당, 0:only자신, 1:자연벽, 2:인공벽, 3:차벽, 4:전선, 5:배관, 6:컨베이어, 7:울타리, 8:물)
    unsigned __int8 lightRange = 0;
    unsigned __int8 lightIntensity = 0;
    unsigned __int8 lightR = 0;
    unsigned __int8 lightG = 0;
    unsigned __int8 lightB = 0;
    __int8 lightDelX = 0;
    __int8 lightDelY = 0;
    __int8 animeSize = 1;
    __int16 animeFPS = 0;
    __int8 randomPropSprSize = 1;
    unsigned __int16 molecularWeight = 1;
    unsigned __int8 liqColorR = 255;
    unsigned __int8 liqColorG = 255;
    unsigned __int8 liqColorB = 255;
    unsigned __int8 gasColorR = 255;
    unsigned __int8 gasColorG = 255;
    unsigned __int8 gasColorB = 255;
    unsigned __int8 gasAlphaMax = 0; //가스연기의 짙은 정도, 만약 무색의 기체의 경우 값은 0
    __int16 propMaxHP = 100; //나무같은 프롭의 HP나 방어상성 설정
    __int8 propRPierce = 0;
    __int8 propRCut = 0;
    __int8 propRBash = 0;
    __int16 propDrawPriority = 0;

    __int32 codeID = 0; //아이템 생성 시 genItemID()로 부여되는 고유 ID값 (0인 경우 미할당)

    __int32 propWIPSprIndex = 80; //제작 중인 아이템의 스프라이트

    int propInstallCode = itemID::none;   //이게 none이 아닐 경우 주변 타일에 설치 가능
    int propUninstallCode = itemID::none; //이게 none이 아닐 경우 주변 타일에 해체 가능

    double electricResistance = 0; //전자기기 저항 (Ω)

    int electricMaxPower = 0; //전력원 순간 출력 (kJ/turn)
    int gndUsePower = 0; //전자기기 소비 전력 (kJ/turn), Prop에서도 작동하지만 개별 아이템(광부헬멧 등)에도 이 값이 참조됨

    //지향성 전자기기 소비 전력 (kJ/turn)
    int gndUsePowerRight = 0;
    int gndUsePowerUp = 0;
    int gndUsePowerLeft = 0;
    int gndUsePowerDown = 0;

    int powerStorageMax = 0; //저장 가능한 최대 전하량(파워뱅크용)

    int fluidDemand = 0; //스프링클러 등이 사용하는 분당 유체 소모량
    int maxFluid = 0; //노드에 들어갈 수 있는 최대 유량
};