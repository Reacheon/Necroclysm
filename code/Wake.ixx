#include <SDL3/SDL.h>
export module Wake;
import Coord;
import std;
import globalVar;
import textureVar;
import constVar;
import drawText;
import Sprite;
import util;
export class Wake : public Coord
{
private:
public:
    int sprIndex = 0;
    int lifetime = 30;
    int alpha = 255;
    int dir = 0;
    static std::vector<Wake*> list;
    static std::unordered_map<Point3, std::vector<Wake*>, Point3::Hash> map;
    Wake(int inputGridX, int inputGridY, int inputGridZ, int inputDir, int inputLifetime = 18)
    {
        setGrid(inputGridX, inputGridY, inputGridZ);
        dir = inputDir;
        lifetime = inputLifetime;
        list.push_back(this);
        map[{ inputGridX, inputGridY, inputGridZ }].push_back(this);
        if (list.size() > 500) prt(L"[메모리 누수 경고] Wake의 객체 수가 500개를 넘어갔습니다.\n");
        if (map[{ inputGridX, inputGridY, inputGridZ }].size() > 500) prt(L"[메모리 누수 경고] Wake의 객체 수가 한 타일에서 500개를 넘어갔습니다.\n");
    }
    ~Wake()
    {
        Point3 key = { getGridX(), getGridY(), getGridZ() };
        auto it = map.find(key);
        if (it != map.end())
        {
            auto& vec = it->second;
            vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
            if (vec.empty()) map.erase(it);
        }
        list.erase(std::find(list.begin(), list.end(), this));
    }
    void step()
    {
        lifetime--;
        if (lifetime <= 0)
        {
            alpha -= 20;
            if (alpha <= 0) delete this;
        }
    }
};
std::vector<Wake*> Wake::list;
std::unordered_map<Point3, std::vector<Wake*>, Point3::Hash> Wake::map;