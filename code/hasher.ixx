export module hasher;

import std;
import Point;

export auto arrayHasher2 = [](const std::array<int, 2>& arr) { return std::hash<int>()(arr[0]) ^ std::hash<int>()(arr[1]); };

export auto arrayHasher3 = [](const std::array<int, 3>& arr) { return std::hash<int>()(arr[0]) ^ std::hash<int>()(arr[1]) ^ std::hash<int>()(arr[2]); };

export auto pointHasher2 = [](const Point2& arr) { return std::hash<int>()(arr.x) ^ std::hash<int>()(arr.y); };

export auto pointHasher3 = [](const Point3& arr) { return std::hash<int>()(arr.x) ^ std::hash<int>()(arr.y) ^ std::hash<int>()(arr.z); };