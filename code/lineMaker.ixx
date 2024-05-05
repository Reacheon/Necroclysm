export module lineMaker;

import std;

//@brief ��ǥ���� �����ϴ�inputPath ���Ϳ� srcXY���� delXY������ ���� �߰� �ش��ϴ� ��ǥ���� ������� ���Ϳ� �״� �Լ� 
export void makeLine(std::vector<std::array<int, 2>>& inputPath, int delX, int delY)
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