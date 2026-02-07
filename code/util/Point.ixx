export module Point;

import std;

export struct Point2
{
    int x = 0;
    int y = 0;

    Point2() : x(0), y(0) {}
    Point2(int initX, int initY) : x(initX), y(initY) {}
    Point2(const std::array<int, 2>& arr) : x(arr[0]), y(arr[1]) {}

    void set(int newX, int newY)
    {
        x = newX;
        y = newY;
    }

    Point2 operator+(const Point2& rhs) const
    {
        return Point2(x + rhs.x, y + rhs.y);
    }

    bool operator==(const Point2& rhs) const { return x == rhs.x && y == rhs.y; }

    struct Hash
    {
        std::size_t operator()(const Point2& p) const
        {
            std::size_t seed = 0;
            seed ^= std::hash<int>()(p.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<int>()(p.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
};

export Point2 calcMidpoint(std::vector<Point2> vec)
{
    if (vec.empty()) return Point2();

    long long totalX = 0;
    long long totalY = 0;

    for (const auto& p : vec)
    {
        totalX += p.x;
        totalY += p.y;
    }

    return Point2((int)(totalX / vec.size()), (int)(totalY / vec.size()));
}

export struct Point3
{
    int x = 0;
    int y = 0;
    int z = 0;

    Point3() : x(0), y(0), z(0) {}
    Point3(int initX, int initY, int initZ) : x(initX), y(initY), z(initZ) {}
    Point3(const std::array<int, 3>& arr) : x(arr[0]), y(arr[1]), z(arr[2]) {}

    void set(int newX, int newY, int newZ)
    {
        x = newX;
        y = newY;
        z = newZ;
    }

    Point3 operator+(const Point3& rhs) const
    {
        return Point3(x + rhs.x, y + rhs.y, z + rhs.z);
    }

    bool operator==(const Point3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }

    bool operator<(const Point3& rhs) const
    {
        if (x != rhs.x) return x < rhs.x;
        if (y != rhs.y) return y < rhs.y;
        return z < rhs.z;
    }

    struct Hash
    {
        std::size_t operator()(const Point3& p) const
        {
            std::size_t seed = 0;
            seed ^= std::hash<int>()(p.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<int>()(p.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<int>()(p.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
};

export Point3 calcMidpoint(std::vector<Point3> vec)
{
    if (vec.empty()) return Point3();

    long long totalX = 0;
    long long totalY = 0;
    long long totalZ = 0;

    for (const auto& p : vec)
    {
        totalX += p.x;
        totalY += p.y;
        totalZ += p.z;
    }

    return Point3((int)(totalX / vec.size()), (int)(totalY / vec.size()), (int)(totalZ / vec.size()));
}