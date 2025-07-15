export module lineMaker;

import std;
import Point;

//@brief 좌표들을 저장하는inputPath 벡터에 srcXY부터 delXY까지의 선을 긋고 해당하는 좌표들을 순서대로 벡터에 쌓는 함수 
export void makeLine(std::vector<Point2>& inputPath, int delX, int delY)
{
    int cursorX = 0;
    int cursorY = 0;
    int xo = 0;
    int yo = 0;
    int delx = std::abs(delX);
    int dely = std::abs(delY);
    int i = 0;
    inputPath.push_back({ cursorX,cursorY });
    if (std::fabs(1.0 * dely / delx) < 1)
    {
        int p = 2 * dely - delx;
        while (i < delx)
        {
            if (p < 0)
            {
                if (delX > xo && delY >= yo) { cursorX++; }
                else if (delX > xo && yo > delY) { cursorX++; }
                else if (xo > delX && delY > yo) { cursorX--; }
                else { cursorX--; }
                inputPath.push_back({ cursorX,cursorY });
                p = p + (2 * dely);
            }
            else
            {
                if (delX > xo && delY >= yo) { cursorX++; cursorY++; }
                else if (delX > xo && yo > delY) { cursorX++; cursorY--; }
                else if (xo > delX && delY > yo) { cursorX--; cursorY++; }
                else { cursorX--; cursorY--; }
                inputPath.push_back({ cursorX,cursorY });
                p = p + (2 * dely) - (2 * delx);
            }
            i++;
        }
        return;
    }
    else if (std::fabs(1.0 * dely / delx) > 1)
    {
        int p = (2 * delx) - dely;
        while (i < dely)
        {
            if (p < 0)
            {
                if (delX >= xo && delY > yo) { cursorY++; }
                else if (delX > xo && yo > delY) { cursorY--; }
                else if (xo > delX && delY > yo) { cursorY++; }
                else { cursorY--; }
                inputPath.push_back({ cursorX,cursorY });
                p = p + (2 * delx);
            }
            else
            {
                if (delX >= xo && delY > yo) { cursorX++; cursorY++; }
                else if (delX > xo && yo > delY) { cursorX++; cursorY--; }
                else if (xo > delX && delY > yo) { cursorX--; cursorY++; }
                else { cursorX--; cursorY--; }
                inputPath.push_back({ cursorX,cursorY });
                p = p + (2 * delx) - (2 * dely);
            }
            i++;
        }
    }
    else
    {
        while (i < delx)
        {
            if (delX > cursorX && delY > cursorY) { cursorX++; cursorY++; }
            else if (delX > cursorX && cursorY > delY) { cursorX++; cursorY--; }
            else if (cursorX > delX && delY > cursorY) { cursorX--; cursorY++; }
            else { cursorX--; cursorY--; }
            inputPath.push_back({ cursorX,cursorY });
            i++;
        }
    }
};


export void makeLine(std::unordered_set<Point2, Point2::Hash>& inputPath,int delX, int delY)
{
    int cursorX = 0;
    int cursorY = 0;
    int xo = 0;
    int yo = 0;
    int delx = std::abs(delX);
    int dely = std::abs(delY);
    int i = 0;

    inputPath.insert(Point2{ cursorX, cursorY });

    if (std::fabs(1.0 * dely / delx) < 1)          // 기울기 |m| < 1
    {
        int p = 2 * dely - delx;
        while (i < delx)
        {
            if (p < 0)                              // E 픽셀 선택
            {
                if (delX > xo && delY >= yo) { ++cursorX; }
                else if (delX > xo && yo > delY) { ++cursorX; }
                else if (xo > delX && delY > yo) { --cursorX; }
                else { --cursorX; }

                inputPath.insert(Point2{ cursorX, cursorY });
                p += 2 * dely;
            }
            else                                    // NE 픽셀 선택
            {
                if (delX > xo && delY >= yo) { ++cursorX; ++cursorY; }
                else if (delX > xo && yo > delY) { ++cursorX; --cursorY; }
                else if (xo > delX && delY > yo) { --cursorX; ++cursorY; }
                else { --cursorX; --cursorY; }

                inputPath.insert(Point2{ cursorX, cursorY });
                p += 2 * dely - 2 * delx;
            }
            ++i;
        }
        return;
    }
    else if (std::fabs(1.0 * dely / delx) > 1)     // 기울기 |m| > 1
    {
        int p = 2 * delx - dely;
        while (i < dely)
        {
            if (p < 0)                              // N 픽셀 선택
            {
                if (delX >= xo && delY > yo) { ++cursorY; }
                else if (delX > xo && yo > delY) { --cursorY; }
                else if (xo > delX && delY > yo) { ++cursorY; }
                else { --cursorY; }

                inputPath.insert(Point2{ cursorX, cursorY });
                p += 2 * delx;
            }
            else                                    // NE 픽셀 선택
            {
                if (delX >= xo && delY > yo) { ++cursorX; ++cursorY; }
                else if (delX > xo && yo > delY) { ++cursorX; --cursorY; }
                else if (xo > delX && delY > yo) { --cursorX; ++cursorY; }
                else { --cursorX; --cursorY; }

                inputPath.insert(Point2{ cursorX, cursorY });
                p += 2 * delx - 2 * dely;
            }
            ++i;
        }
    }
    else                                            // 기울기 |m| == 1
    {
        while (i < delx)
        {
            if (delX > cursorX && delY > cursorY) { ++cursorX; ++cursorY; }
            else if (delX > cursorX && cursorY > delY) { ++cursorX; --cursorY; }
            else if (cursorX > delX && delY > cursorY) { --cursorX; ++cursorY; }
            else { --cursorX; --cursorY; }

            inputPath.insert(Point2{ cursorX, cursorY });
            ++i;
        }
    }
}