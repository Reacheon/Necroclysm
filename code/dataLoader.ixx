#include <sol/sol.hpp>

export module dataLoader;

import std;
import util;
import globalVar;
import readItemDex;
import readEntityDex;
import readSkillDex;

export void dataLoader()
{
	//루아스크립트 로드
	lua.open_libraries(sol::lib::base, sol::lib::math);

	//아이템 데이터 로드
	std::wstring itemPath = L"language/" + option::language + L"/itemDex.tsv";
	readItemDex(itemPath.c_str());

	//엔티티 데이터 로드
	std::wstring entityPath = L"language/" + option::language + L"/entityDex.tsv";
	readEntityDex(entityPath.c_str());

	//스킬 데이터 로드
	std::wstring skillPath = L"language/" + option::language + L"/skillDex.tsv";
	readSkillDex(skillPath.c_str());

	//시스템(UI) 문자열 로드
	std::wstring systemPath = L"language/" + option::language + L"/sysStr.tsv";
	std::vector<std::array<std::wstring, 4>> tempSysStr(1, { L" ", L" ", L" " });
	readTSV(systemPath.c_str(), tempSysStr);
	systemPath.clear();
	for (int i = 0; i < tempSysStr.size(); i++) { sysStr.push_back(tempSysStr[i][1]); }
}