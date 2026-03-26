export module constVar:names;


export namespace partType
{
    constexpr unsigned __int8 null = 0;
    constexpr unsigned __int8 head = 1;
    constexpr unsigned __int8 torso = 2;
    constexpr unsigned __int8 larm = 3;
    constexpr unsigned __int8 rarm = 4;
    constexpr unsigned __int8 lleg = 5;
    constexpr unsigned __int8 rleg = 6;
    constexpr unsigned __int8 tail = 7;

    constexpr unsigned __int8 turret = 8;
    constexpr unsigned __int8 camera = 9;
    constexpr unsigned __int8 body = 10;
    constexpr unsigned __int8 lCaterpillar = 11;
    constexpr unsigned __int8 rCaterpillar = 12;
};

export namespace wound
{
    constexpr int pierce = 0; //관통상
    constexpr int cut = 1; //절단상
    constexpr int bash = 2; //타박상
    constexpr int fracture = 3; //골절 : 피격시 타박상 포인트가 높을 경우 발생, 부목으로만 치료 가능 20% 고정 손실
    constexpr int burn = 4; //화상 : 치료 불가능, 의사에게 가야함
    constexpr int frostbite = 5; //동상 : 따뜻한 곳에 가면 아주 조금씩 재생됨
    constexpr int shock = 6; //쇼크 : 정신적인 충격, 머리에만 발생함
};

export namespace proficFlag
{
    constexpr int fighting = 0;
    constexpr int dodging = 1;
    constexpr int stealth = 2;
    constexpr int throwing = 3;
    constexpr int unarmedCombat = 4;
    constexpr int piercingWeapon = 5;
    constexpr int cuttingWeapon = 6;
    constexpr int bashingWeapon = 7;
    constexpr int archery = 8;
    constexpr int gun = 9;
    constexpr int electronics = 10;
    constexpr int chemistry = 11;
    constexpr int mechanics = 12;
    constexpr int computer = 13;
    constexpr int medicine = 14;
    constexpr int cooking = 15;
    constexpr int fabrication = 16;
    constexpr int social = 17;
    constexpr int architecture = 18;
    constexpr int invocations = 19;
};

export namespace toolQuality
{
    constexpr int none = 0;
    constexpr int screwDriving = 1;
    constexpr int drilling = 2;
    constexpr int welding = 3;
    constexpr int soldering = 4;
    constexpr int cutting = 5;
    constexpr int sawing = 6;
    constexpr int hammering = 7;
    constexpr int digging = 8;
    constexpr int sewing = 9;
    constexpr int distillation = 10;
    constexpr int boiling = 11;
    constexpr int frying = 12;
    constexpr int roasting = 13;
    constexpr int boltTurning = 14;
    constexpr int woodCutting = 15;
    constexpr int metalCutting = 16;
    constexpr int metalDrilling = 17;
    constexpr int refrigeration = 18;
    constexpr int heating = 19;
};

export namespace keyboardIndex
{
    constexpr int tab = 1;
    constexpr int tabPressed = 2;
    constexpr int m = 3;
    constexpr int mPressed = 4;
    constexpr int enter = 5;
    constexpr int enterPressed = 6;
    constexpr int shift = 7;
    constexpr int shiftPressed = 8;
    constexpr int mouseLeft = 10;
    constexpr int mouseRight = 11;
    constexpr int mouseWheel = 12;
}

export namespace keyIcon
{
    constexpr int blankRect = 0;

    constexpr int keyboard_1 = 1;
    constexpr int keyboard_2 = 2;
    constexpr int keyboard_3 = 3;
    constexpr int keyboard_4 = 4;
    constexpr int keyboard_5 = 5;
    constexpr int keyboard_6 = 6;
    constexpr int keyboard_7 = 7;
    constexpr int keyboard_8 = 8;
    constexpr int keyboard_9 = 9;
    constexpr int keyboard_0 = 10;

    constexpr int keyboard_Num1 = 11;
    constexpr int keyboard_Num2 = 12;
    constexpr int keyboard_Num3 = 13;
    constexpr int keyboard_Num4 = 14;
    constexpr int keyboard_Num5 = 15;
    constexpr int keyboard_Num6 = 16;
    constexpr int keyboard_Num7 = 17;
    constexpr int keyboard_Num8 = 18;
    constexpr int keyboard_Num9 = 19;
    constexpr int keyboard_Num0 = 20;

    constexpr int keyboard_F1 = 21;
    constexpr int keyboard_F2 = 22;
    constexpr int keyboard_F3 = 23;
    constexpr int keyboard_F4 = 24;
    constexpr int keyboard_F5 = 25;
    constexpr int keyboard_F6 = 26;
    constexpr int keyboard_F7 = 27;
    constexpr int keyboard_F8 = 28;
    constexpr int keyboard_F9 = 29;
    constexpr int keyboard_F10 = 30;
    constexpr int keyboard_F11 = 31;
    constexpr int keyboard_F12 = 32;

    constexpr int keyboard_H = 33;
    constexpr int keyboard_J = 34;
    constexpr int keyboard_K = 35;
    constexpr int keyboard_L = 36;
    constexpr int keyboard_Y = 37;
    constexpr int keyboard_U = 38;
    constexpr int keyboard_B = 39;
    constexpr int keyboard_N = 40;
    constexpr int keyboard_A = 41;
    constexpr int keyboard_S = 42;
    constexpr int keyboard_Z = 43;
    constexpr int keyboard_X = 44;

    constexpr int keyboard_Tab = 45;
    constexpr int keyboard_Enter = 46;
    constexpr int keyboard_PgUp = 47;
    constexpr int keyboard_PgDn = 48;

    constexpr int pad_X = 49;
    constexpr int pad_Y = 50;
    constexpr int pad_A = 51;
    constexpr int pad_B = 52;
    constexpr int pad_L = 53;
    constexpr int pad_R = 54;
    constexpr int pad_LStick = 55;
    constexpr int pad_ZL = 56;
    constexpr int pad_ZR = 57;

    constexpr int pad_Right = 58;
    constexpr int pad_Up = 59;
    constexpr int pad_Left = 60;
    constexpr int pad_Down = 61;

    constexpr int keyboard_C = 62;
    constexpr int keyboard_V = 63;

    constexpr int keyboard_LShift = 64;
    constexpr int keyboard_RShift = 65;
    constexpr int keyboard_M = 66;

    constexpr int duelSense_L1 = 80;
    constexpr int duelSense_L2 = 81;
    constexpr int duelSense_R1 = 82;
    constexpr int duelSense_R2 = 83;

    constexpr int duelSense_X = 84;
    constexpr int duelSense_CIR = 85;
    constexpr int duelSense_RECT = 86;
    constexpr int duelSense_TRI = 87;
    constexpr int duelSense_LStick = 88;
    constexpr int duelSense_RStick = 89;

    constexpr int duelSense_RIGHT = 90;
    constexpr int duelSense_UP = 91;
    constexpr int duelSense_LEFT = 92;
    constexpr int duelSense_DOWN = 93;
    constexpr int duelSense_OPTIONS = 94;
    constexpr int duelSense_SHARE = 95;

    ////////

    constexpr int joyCon_L = 96;
    constexpr int joyCon_ZL = 97;
    constexpr int joyCon_R = 98;
    constexpr int joyCon_ZR = 99;

    constexpr int joyCon_A = 100;
    constexpr int joyCon_B = 101;
    constexpr int joyCon_X = 102;
    constexpr int joyCon_Y = 103;
    constexpr int joyCon_LStick = 104;
    constexpr int joyCon_RStick = 105;

    constexpr int joyCon_RIGHT = 106;
    constexpr int joyCon_UP = 107;
    constexpr int joyCon_LEFT = 108;
    constexpr int joyCon_DOWN = 109;
    constexpr int joyCon_PLUS = 110;
    constexpr int joycon_MINUS = 111;
};

export namespace sprInf
{
    constexpr int walk = 0;
    constexpr int run = 6;
    constexpr int sit = 12;
    constexpr int crawl = 18;
};

export namespace entityCategory
{
    constexpr int none = 0;
    constexpr int human = 1;
    constexpr int zombie = 2;
    constexpr int robot = 3;
    constexpr int animal = 4;
};

export namespace bulletFlag
{
    constexpr int normal = 0;
    constexpr int tracer = 1;
    constexpr int ap = 2;
};

export namespace weaponMode
{
    constexpr int none = 0;
    constexpr int safeMode = 1;
    constexpr int semiAutoMode = 2;
    constexpr int burstMode = 3;
    constexpr int autoMode = 4;
};

export namespace UNI
{
    constexpr int NUL = 0;   // Null char
    constexpr int SOH = 1;   // Start of Heading
    constexpr int STX = 2;   // Start of Text
    constexpr int ETX = 3;   // End of Text
    constexpr int EOT = 4;   // End of Transmission
    constexpr int ENQ = 5;   // Enquiry
    constexpr int ACK = 6;   // Acknowledgment
    constexpr int BEL = 7;   // Bell
    constexpr int BACKSPACE = 8;    // Back Space
    constexpr int TAB = 9;   // Horizontal Tab
    constexpr int LF = 10;   // Line Feed
    constexpr int VT = 11;   // Vertical Tab
    constexpr int FF = 12;   // Form Feed
    constexpr int CR = 13;   // Carriage Return
    constexpr int SO = 14;   // Shift Out / X-On
    constexpr int SI = 15;   // Shift In / X-Off
    constexpr int DLE = 16;  // Data Line Escape
    constexpr int DC1 = 17;  // Device Control 1 (oft. XON)
    constexpr int DC2 = 18;  // Device Control 2
    constexpr int DC3 = 19;  // Device Control 3 (oft. XOFF)
    constexpr int DC4 = 20;  // Device Control 4
    constexpr int NAK = 21;  // Negative Acknowledgement
    constexpr int SYN = 22;  // Synchronous Idle
    constexpr int ETB = 23;  // End of Transmit Block
    constexpr int CAN = 24;  // Cancel
    constexpr int EM = 25;   // End of Medium
    constexpr int SUB = 26;  // Substitute
    constexpr int ESC = 27;  // Escape
    constexpr int FS = 28;   // File Separator
    constexpr int GS = 29;   // Group Separator
    constexpr int RS = 30;   // Record Separator
    constexpr int US = 31;   // Unit Separator
    constexpr int SPACE = 32;
    constexpr int EXCLAMATION_MARK = 33;  // '!'
    constexpr int DOUBLE_QUOTES = 34;  // '"'
    constexpr int HASH = 35;  // '#'
    constexpr int DOLLAR_SIGN = 36;  // '$'
    constexpr int PERCENT_SIGN = 37;  // '%'
    constexpr int AMPERSAND = 38;  // '&'
    constexpr int APOSTROPHE = 39;  // '''
    constexpr int LEFT_PARENTHESIS = 40;  // '('
    constexpr int RIGHT_PARENTHESIS = 41;  // ')'
    constexpr int ASTERISK = 42;  // '*'
    constexpr int PLUS_SIGN = 43;  // '+'
    constexpr int COMMA = 44;  // ','
    constexpr int MINUS_SIGN = 45;  // '-'
    constexpr int PERIOD = 46;  // '.'
    constexpr int SLASH = 47;  // '/'
    constexpr int ZERO = 48;
    constexpr int ONE = 49;
    constexpr int TWO = 50;
    constexpr int THREE = 51;
    constexpr int FOUR = 52;
    constexpr int FIVE = 53;
    constexpr int SIX = 54;
    constexpr int SEVEN = 55;
    constexpr int EIGHT = 56;
    constexpr int NINE = 57;
    constexpr int COLON = 58;  // ':'
    constexpr int SEMICOLON = 59;  // ';'
    constexpr int LESS_THAN_SIGN = 60;  // '<'
    constexpr int EQUAL_SIGN = 61;  // '='
    constexpr int GREATER_THAN_SIGN = 62;  // '>'
    constexpr int QUESTION_MARK = 63;  // '?'
    constexpr int AT_SIGN = 64;  // '@'
    constexpr int A = 65;
    constexpr int B = 66;
    constexpr int C = 67;
    constexpr int D = 68;
    constexpr int E = 69;
    constexpr int F = 70;
    constexpr int G = 71;
    constexpr int H = 72;
    constexpr int I = 73;
    constexpr int J = 74;
    constexpr int K = 75;
    constexpr int L = 76;
    constexpr int M = 77;
    constexpr int N = 78;
    constexpr int O = 79;
    constexpr int P = 80;
    constexpr int Q = 81;
    constexpr int R = 82;
    constexpr int S = 83;
    constexpr int T = 84;
    constexpr int U = 85;
    constexpr int V = 86;
    constexpr int W = 87;
    constexpr int X = 88;
    constexpr int Y = 89;
    constexpr int Z = 90;
    constexpr int LEFT_SQUARE_BRACKET = 91;  // '['
    constexpr int BACKSLASH = 92;  // '\'
    constexpr int RIGHT_SQUARE_BRACKET = 93;  // ']'
    constexpr int CARET = 94;  // '^'
    constexpr int UNDERSCORE = 95;  // '_'
    constexpr int GRAVE_ACCENT = 96;  // '`'
    constexpr int a = 97;
    constexpr int b = 98;
    constexpr int c = 99;
    constexpr int d = 100;
    constexpr int e = 101;
    constexpr int f = 102;
    constexpr int g = 103;
    constexpr int h = 104;
    constexpr int i = 105;
    constexpr int j = 106;
    constexpr int k = 107;
    constexpr int l = 108;
    constexpr int m = 109;
    constexpr int n = 110;
    constexpr int o = 111;
    constexpr int p = 112;
    constexpr int q = 113;
    constexpr int r = 114;
    constexpr int s = 115;
    constexpr int t = 116;
    constexpr int u = 117;
    constexpr int v = 118;
    constexpr int w = 119;
    constexpr int x = 120;
    constexpr int y = 121;
    constexpr int z = 122;
    constexpr int LEFT_CURLY_BRACKET = 123;  // '{'
    constexpr int VERTICAL_BAR = 124;  // '|'
    constexpr int RIGHT_CURLY_BRACKET = 125;  // '}'
    constexpr int TILDE = 126;  // '~'
    constexpr int DEL = 127;   // Delete
    constexpr int MIDDLE_DOT = 183;
};

export namespace entityRefCode
{
    constexpr int none = 0;
    constexpr int player = 1;
    constexpr int zombieA = 2;
    constexpr int horse = 3;
};

export namespace skillRefCode
{
    constexpr int roll = 32;
    constexpr int leap = 33;
}

export namespace connectFlag
{
    constexpr int cross = 0;
    constexpr int none = 1;
    constexpr int ULD = 2;
    constexpr int LD = 3;
    constexpr int RLD = 4;
    constexpr int RD = 5;
    constexpr int RUD = 6;
    constexpr int RU = 7;
    constexpr int RUL = 8;
    constexpr int UL = 9;
    constexpr int L = 10;
    constexpr int D = 11;
    constexpr int R = 12;
    constexpr int U = 13;
    constexpr int RL = 14;
    constexpr int UD = 15;
}