#pragma once
// sol::state를 모듈 경계 밖으로 분리하기 위한 헤더
// (MSVC의 C++20 모듈 + sol2 템플릿 특수화 호환성 문제 회피)
#include <sol/sol.hpp>

inline sol::state lua;
