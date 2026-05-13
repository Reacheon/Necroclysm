export module dsu;

import std;

//============================================================
// Disjoint Set Union (Union-Find) with path compression + union by rank.
//   find: amortized α(n) ≈ O(1)
//   unite: 새로 합쳐졌으면 true, 이미 같은 집합이면 false.
//
//   원소는 0..n-1 정수 인덱스로 식별. 도메인 무관 순수 자료구조.
//============================================================

//@brief 대량의 원소들을 동적으로 묶고, 이게 같은 그룹인지 물어보는 자료구조 amortized O(1)
export struct DSU
{
    std::vector<int> p, r;

    DSU(int n) : p(n), r(n, 0)
    {
        std::iota(p.begin(), p.end(), 0);
    }

    int find(int x)
    {
        while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; }
        return x;
    }

    bool unite(int a, int b)
    {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (r[a] < r[b]) std::swap(a, b);
        p[b] = a;
        if (r[a] == r[b]) ++r[a];
        return true;
    }
};
