export module hasher;

import std;
import Point;

export struct arrayHasher2 {
    std::size_t operator()(const std::array<int, 2>& arr) const noexcept {
        std::size_t seed = 0;
        seed ^= std::hash<int>()(arr[0]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(arr[1]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};

export struct arrayHasher3 {
    std::size_t operator()(const std::array<int, 3>& arr) const noexcept {
        std::size_t seed = 0;
        seed ^= std::hash<int>()(arr[0]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(arr[1]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        seed ^= std::hash<int>()(arr[2]) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        return seed;
    }
};
