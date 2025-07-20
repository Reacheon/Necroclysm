#include <SDL3/SDL.h>
export module Footprint;
import Coord;
import std;
import globalVar;
import textureVar;
import constVar;
import drawText;
import Sprite;
import util;
export class Footprint : public Coord
{
private:
public:
    Sprite* sprite = spr::footprint;
    int sprIndex = 0;
    int lifetime = 30;
    int alpha = 80;
    int gridX = 0;
    int gridY = 0;
    int gridZ = 0;
    static std::vector<Footprint*> list;
    static std::unordered_map<Point3, std::vector<Footprint*>, Point3::Hash> map;
    Footprint(int inputGridX, int inputGridY, int inputGridZ, int inputDir, int inputLifetime = 80)
    {
        gridX = inputGridX;
        gridY = inputGridY;
        gridZ = inputGridZ;
        sprIndex += inputDir;
        lifetime = inputLifetime;
        list.push_back(this);
        map[{ inputGridX, inputGridY, inputGridZ }].push_back(this);
        setXY(16 * inputGridX, 16 * inputGridY);
        if (list.size() > 500) prt(L"[메모리 누수 경고] Footprint의 객체 수가 500개를 넘어갔습니다.\n");
        if (map[{ inputGridX, inputGridY, inputGridZ }].size() > 500) prt(L"[메모리 누수 경고] Footprint의 객체 수가 한 타일에서 500개를 넘어갔습니다.\n");
    }
    ~Footprint()
    {
        Point3 key = { gridX, gridY, gridZ };
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
            alpha -= 5;
            if (alpha <= 0) delete this;
        }
    }
};
std::vector<Footprint*> Footprint::list;
std::unordered_map<Point3, std::vector<Footprint*>, Point3::Hash> Footprint::map;