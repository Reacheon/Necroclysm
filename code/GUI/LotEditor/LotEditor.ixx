module;
#include <SDL3/SDL.h>

export module LotEditor;

import std;
import Point;
import GUI;
import constVar;
import ItemPocket;
import Vehicle;

//게임 내장 Lot 에디터. 디버그 콘솔(39번)에서 활성화. 마우스로 floor/wall/prop/item/monster를
//  실제 World에 직접 칠하고, 청크 정렬 영역을 Lot 작성용 텍스트로 Export 한다. 별도 그리기 레이어
//  없이 게임 렌더 루프가 그대로 그려준다(World 데이터만 수정). 시간은 흐르지 않는다.
//  편집 중 lotEditorActive로 시야 전체공개 + 플레이어 숨김. 카메라는 플레이어와 분리(자유 팬).
export class LotEditor : public GUI
{
public:
    enum class EditMode { Floor, Wall, Prop, Item, Monster, Vehicle };
    enum class EditTool { Brush, RectFill, RectOutline, Eyedropper, Eraser, Clear };
    static constexpr int MODE_COUNT = 6;
    static constexpr int TOOL_COUNT = 6;
    static constexpr int TOPBAR_H = 56;
    static constexpr int HINT_H = 20;
    static constexpr int PAL_W = 176;

private:
    inline static LotEditor* ptr = nullptr;

    EditMode mode_ = EditMode::Floor;
    EditTool tool_ = EditTool::Brush;

    //카테고리별로 마지막 선택을 기억. selMonster_는 entityCode(itemDex 인덱싱 금지).
    int selFloor_ = itemID::dirt;
    int selWall_ = itemID::none;
    int selProp_ = itemID::none;
    int selItem_ = itemID::none;
    int selMonster_ = 0;
    int selVeh_ = itemID::metalFrame; //차량 프레임/부품 선택(VFRAME/VPART)

    //생성자 1회 스캔으로 채움(itemDex flag별 / entityDex).
    std::vector<int> bucketFloor_;
    std::vector<int> bucketWall_;
    std::vector<int> bucketProp_;
    std::vector<int> bucketMonster_;
    std::vector<int> bucketItem_;
    std::vector<int> bucketVeh_;
    int paletteScroll_ = 0;
    int hoverPaletteIdx_ = -1; //팔레트 호버 셀(툴팁용). -1=없음

    //사각형 도구 드래그 상태.
    bool rectActive_ = false;
    Point2 rectAnchor_{ 0, 0 };

    //Alt 홀드 동안 임시 스포이드.
    bool altEyedrop_ = false;

    //브러시 드래그 중복 페인트 방지(직전 칠한 타일).
    Point2 lastPainted_{ -2000000000, -2000000000 };

    //Export 박스(청크 좌표, 양끝 포함).
    bool boxSet_ = false;
    Point3 boxChunkA_{ 0, 0, 0 };
    Point3 boxChunkB_{ 0, 0, 0 };

    //우클릭 컨텍스트 메뉴.
    bool menuOpen_ = false;
    Point2 menuPos_{ 0, 0 };
    Point3 menuTile_{ 0, 0, 0 };
    std::vector<std::pair<std::wstring, int>> menuItems_;

    //차량 부품 에디터.
    Vehicle* activeVeh_ = nullptr; //현재 편집 중인 차량(편집 범위 = 이 차량의 footprint)
    bool newVehPending_ = false;   //다음 프레임 클릭이 새 차량 생성
    bool vehNameEdit_ = false;     //차량 이름 텍스트 입력 중(exInput 사용)
    SDL_Rect vehPanelRect_{ 0, 0, 0, 0 };
    SDL_Rect vehNameBtn_{ 0, 0, 0, 0 };
    SDL_Rect vehTypeBtn_{ 0, 0, 0, 0 };
    SDL_Rect vehDirBtn_{ 0, 0, 0, 0 };
    SDL_Rect vehDeselectBtn_{ 0, 0, 0, 0 };

    //그리드 토글(에디터 자체 렌더).
    bool gridOn_ = true;
    bool tileGridOn_ = false; //1타일 무채색 격자(문 진입 시 적 거리 판단용)

    //레이아웃 rect(매 프레임 computeLayout로 채움; 입력 히트테스트와 공유).
    SDL_Rect modeTabRect_[MODE_COUNT]{};
    SDL_Rect toolBtnRect_[TOOL_COUNT]{};
    SDL_Rect palettePanelRect_{ 0, 0, 0, 0 };
    SDL_Rect paletteGridRect_{ 0, 0, 0, 0 };

    //호스트 상태 저장(종료 시 복원).
    bool savedCameraFix_ = true;
    int savedCameraX_ = 0;
    int savedCameraY_ = 0;
    float savedZoom_ = 3.0f;
    Point3 savedPlayerPos_{ 0, 0, 0 };
    bool gridWasOn_ = false;
    bool savedDrawHUD_ = true;

public:
    LotEditor();
    ~LotEditor();
    static LotEditor* ins() { return ptr; }

    void changeXY(int inputX, int inputY, bool center) override;
    void drawGUI() override;
    void clickDownGUI() override;
    void clickMotionGUI(int dx, int dy) override;
    void clickUpGUI() override;
    void clickRightGUI() override;
    void keyDownGUI() override;
    void keyUpGUI() override;
    void mouseWheel() override;
    void step() override;

private:
    //lifecycle
    void saveHostState();
    void restoreHostState();

    //palette
    void buildPalettes();
    std::vector<int>* currentBucket();
    int paletteCols();
    int paletteMaxScroll();
    bool mouseOverPalette();
    void drawPalette();
    void paletteClick();

    //navigation / apply
    void changeZ(int dz);
    void panCameraTiles(int dx, int dy);
    void ensureChunkAt(int x, int y, int z);
    void ensureVisibleChunks();
    Point3 cursorTile();
    Point2 cursorChunkXY();
    Point2 currentChunkXY();
    void applyToolAt(Point3 t);
    void applyByMode(Point3 t);
    void applyFloor(Point3 t, int code);
    void applyWall(Point3 t, int code);
    void applyProp(Point3 t, int code);
    void applyMonster(Point3 t);
    void removeMonsterAt(Point3 t);
    void applyVehicle(Point3 t);
    void startNewVehicle();
    void cycleVehType();
    void rotateActiveVeh();
    void startVehNameEdit();
    void confirmVehName();
    void stopVehNameEdit();
    void applyItem(Point3 t);
    ItemPocket* pocketAt(Point3 t, int* outKind = nullptr); //kind: 0=바닥스택, 1=컨테이너 프롭, 2=차량 cargo 부품
    void eraseAt(Point3 t);
    void rasterRect(Point2 a, Point2 b, bool outline);
    void clearRect(Point2 a, Point2 b);
    void eyedropAt(Point3 t);
    void rotateSelectedProp();
    int currentSelCode();
    void setSelCode(int code);
    void setMode(EditMode m);

    //draw
    void computeLayout();
    void drawTopBar();
    void drawChunkGrid();
    void drawTileGrid();
    void drawPreview();
    void drawExportBox();
    void drawInspector();
    void drawActiveVehHighlight();
    void drawVehPanel();
    void drawTooltip();
    void drawHintBar();
    void tileToScreenCenter(int tx, int ty, int& sx, int& sy);
    int modeTabAt(int mx, int my);
    int toolBtnAt(int mx, int my);

    //export
    void doExport();

    //menu
    void openEditorContextMenu(Point3 t);
    void drawContextMenu();
    void contextMenuClick();
    void executeMenuAction(int id);
};
