export module hasher;

import std;
import Point;

export auto arrayHasher2 = [](const std::array<int, 2>& arr)
    {
        std::size_t seed = 0;
        seed ^= std::hash<int>()(arr[0]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(arr[1]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    };

export auto arrayHasher3 = [](const std::array<int, 3>& arr)
    {
        std::size_t seed = 0;
        seed ^= std::hash<int>()(arr[0]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(arr[1]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(arr[2]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    };

export auto pointHasher2 = [](const Point2& point)
    {
        std::size_t seed = 0;
        seed ^= std::hash<int>()(point.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(point.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    };


export auto pointHasher3 = [](const Point3& point)
    {
        std::size_t seed = 0;
        seed ^= std::hash<int>()(point.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(point.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(point.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    };
