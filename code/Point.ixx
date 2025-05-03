export module Point;

import std;

export struct Point2
{
    int x = 0;
    int y = 0;

    Point2() : x(0), y(0) {}
    Point2(int initX, int initY) : x(initX), y(initY) { }
    Point2(const std::array<int, 2>& arr) : x(arr[0]), y(arr[1]) {}

    void set(int newX, int newY)
    {
        x = newX;
        y = newY;
    }

    bool operator==(const Point2& rhs) const { return x == rhs.x && y == rhs.y; }
};

export Point2 calcMidpoint(std::vector<Point2> vec)
{
    int totalX = 0;
    int totalY = 0;

    for (int i = 0; i < vec.size(); i++)
    {
        totalX += vec[i].x;
        totalY += vec[i].y;
    }

    return { (int)(totalX / vec.size()), (int)(totalY / vec.size()) };
}

export struct Point3
{
    int x = 0;
    int y = 0;
    int z = 0;

    Point3() : x(0), y(0), z(0) {}
    Point3(int initX, int initY, int initZ) : x(initX), y(initY), z(initZ) { }
    Point3(const std::array<int, 3>& arr) : x(arr[0]), y(arr[1]), z(arr[2]) {}

    void set(int newX, int newY, int newZ)
    {
        x = newX;
        y = newY;
        z = newZ;
    }

    bool operator==(const Point3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }

    bool operator<(const Point3& rhs) const 
    {
        if (x != rhs.x) return x < rhs.x;
        if (y != rhs.y) return y < rhs.y;
        return z < rhs.z;
    }
};


export Point3 calcMidpoint(std::vector<Point3> vec)
{
    int totalX = 0;
    int totalY = 0;
    int totalZ = 0;

    for (int i = 0; i < vec.size(); i++)
    {
        totalX += vec[i].x;
        totalY += vec[i].y;
        totalZ += vec[i].z;
    }

    return { (int)(totalX / vec.size()), (int)(totalY / vec.size()), (int)(totalZ / vec.size()) };
}