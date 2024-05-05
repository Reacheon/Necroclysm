//16¹æÇâ

export module dir16;

import std;
import Vec3;

export enum class dir16
{
    dir0,
    dir0_5,
    dir1,
    dir1_5,
    dir2,
    dir2_5,
    dir3,
    dir3_5,
    dir4,
    dir4_5,
    dir5,
    dir5_5,
    dir6,
    dir6_5,
    dir7,
    dir7_5,
};

export dir16 ACW(dir16 input)
{
    if (input == dir16::dir0) return dir16::dir0_5;
    else if (input == dir16::dir0_5) return dir16::dir1;
    else if (input == dir16::dir1) return dir16::dir1_5;
    else if (input == dir16::dir1_5) return dir16::dir2;
    else if (input == dir16::dir2) return dir16::dir2_5;
    else if (input == dir16::dir2_5) return dir16::dir3;
    else if (input == dir16::dir3) return dir16::dir3_5;
    else if (input == dir16::dir3_5) return dir16::dir4;
    else if (input == dir16::dir4) return dir16::dir4_5;
    else if (input == dir16::dir4_5) return dir16::dir5;
    else if (input == dir16::dir5) return dir16::dir5_5;
    else if (input == dir16::dir5_5) return dir16::dir6;
    else if (input == dir16::dir6) return dir16::dir6_5;
    else if (input == dir16::dir6_5) return dir16::dir7;
    else if (input == dir16::dir7) return dir16::dir7_5;
    else if (input == dir16::dir7_5) return dir16::dir0;
}

export dir16 ACW2(dir16 input)
{
    return ACW(ACW(input));
}

export dir16 CW(dir16 input)
{
    if (input == dir16::dir0) return dir16::dir7_5;
    else if (input == dir16::dir0_5) return dir16::dir0;
    else if (input == dir16::dir1) return dir16::dir0_5;
    else if (input == dir16::dir1_5) return dir16::dir1;
    else if (input == dir16::dir2) return dir16::dir1_5;
    else if (input == dir16::dir2_5) return dir16::dir2;
    else if (input == dir16::dir3) return dir16::dir2_5;
    else if (input == dir16::dir3_5) return dir16::dir3;
    else if (input == dir16::dir4) return dir16::dir3_5;
    else if (input == dir16::dir4_5) return dir16::dir4;
    else if (input == dir16::dir5) return dir16::dir4_5;
    else if (input == dir16::dir5_5) return dir16::dir5;
    else if (input == dir16::dir6) return dir16::dir5_5;
    else if (input == dir16::dir6_5) return dir16::dir6;
    else if (input == dir16::dir7) return dir16::dir6_5;
    else if (input == dir16::dir7_5) return dir16::dir7;
}

export dir16 CW2(dir16 input)
{
    return CW(CW(input));
}

export dir16 reverse(dir16 input)
{
    if (input == dir16::dir0) return dir16::dir4;
    else if (input == dir16::dir0_5) return dir16::dir4_5;
    else if (input == dir16::dir1) return dir16::dir5;
    else if (input == dir16::dir1_5) return dir16::dir5_5;
    else if (input == dir16::dir2) return dir16::dir6;
    else if (input == dir16::dir2_5) return dir16::dir6_5;
    else if (input == dir16::dir3) return dir16::dir7;
    else if (input == dir16::dir3_5) return dir16::dir7_5;
    else if (input == dir16::dir4) return dir16::dir0;
    else if (input == dir16::dir4_5) return dir16::dir0_5;
    else if (input == dir16::dir5) return dir16::dir1;
    else if (input == dir16::dir5_5) return dir16::dir1_5;
    else if (input == dir16::dir6) return dir16::dir2;
    else if (input == dir16::dir6_5) return dir16::dir2_5;
    else if (input == dir16::dir7) return dir16::dir3;
    else if (input == dir16::dir7_5) return dir16::dir3_5;
}

export std::wstring dir16ToString(dir16 input)
{
    if (input == dir16::dir0) return L"0¡Æ";
    else if (input == dir16::dir0_5) return L"22.5¡Æ";
    else if (input == dir16::dir1) return L"45¡Æ";
    else if (input == dir16::dir1_5) return L"67.5¡Æ";
    else if (input == dir16::dir2) return L"90¡Æ";
    else if (input == dir16::dir2_5) return L"112.5¡Æ";
    else if (input == dir16::dir3) return L"135¡Æ";
    else if (input == dir16::dir3_5) return L"157.5¡Æ";
    else if (input == dir16::dir4) return L"180¡Æ";
    else if (input == dir16::dir4_5) return L"202.5¡Æ";
    else if (input == dir16::dir5) return L"225¡Æ";
    else if (input == dir16::dir5_5) return L"247.5¡Æ";
    else if (input == dir16::dir6) return L"270¡Æ";
    else if (input == dir16::dir6_5) return L"292.5¡Æ";
    else if (input == dir16::dir7) return L"315¡Æ";
    else if (input == dir16::dir7_5) return L"337.5¡Æ";
}

export Vec3 dir16ToVec(dir16 input)
{
    if (input == dir16::dir0) return getDefaultVec(0);
    else if (input == dir16::dir0_5) return getDefaultVec(1);
    else if (input == dir16::dir1) return getDefaultVec(2);
    else if (input == dir16::dir1_5) return getDefaultVec(3);
    else if (input == dir16::dir2) return getDefaultVec(4);
    else if (input == dir16::dir2_5) return getDefaultVec(5);
    else if (input == dir16::dir3) return getDefaultVec(6);
    else if (input == dir16::dir3_5) return getDefaultVec(7);
    else if (input == dir16::dir4) return getDefaultVec(8);
    else if (input == dir16::dir4_5) return getDefaultVec(9);
    else if (input == dir16::dir5) return getDefaultVec(10);
    else if (input == dir16::dir5_5) return getDefaultVec(11);
    else if (input == dir16::dir6) return getDefaultVec(12);
    else if (input == dir16::dir6_5) return getDefaultVec(13);
    else if (input == dir16::dir7) return getDefaultVec(14);
    else if (input == dir16::dir7_5) return getDefaultVec(15);
}

export int dir16toInt16(dir16 input)
{
    if (input == dir16::dir0) return 0;
    else if (input == dir16::dir0_5) return 1;
    else if (input == dir16::dir1) return 2;
    else if (input == dir16::dir1_5) return 3;
    else if (input == dir16::dir2) return 4;
    else if (input == dir16::dir2_5) return 5;
    else if (input == dir16::dir3) return 6;
    else if (input == dir16::dir3_5) return 7;
    else if (input == dir16::dir4) return 8;
    else if (input == dir16::dir4_5) return 9;
    else if (input == dir16::dir5) return 10;
    else if (input == dir16::dir5_5) return 11;
    else if (input == dir16::dir6) return 12;
    else if (input == dir16::dir6_5) return 13;
    else if (input == dir16::dir7) return 14;
    else if (input == dir16::dir7_5) return 15;
}