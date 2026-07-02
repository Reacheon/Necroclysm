module;
#include <SDL3/SDL.h>

export module Skill;

import std;
import GUI;
import SkillData;
import constVar;

export class Skill : public GUI
{
private:
	inline static Skill* ptr = nullptr;
	SDL_Rect skillBase;
	std::vector<skillCategory> visibleCats;  //표시 중인 탭들. 전체는 항상, 나머지는 해당 카테고리 스킬 보유 시에만 (refilterSkills에서 재구성)
	int skillCursor = -1;
	int skillScroll = 0;
	int initSkillScroll = 0;

	skillCategory categoryCursor = skillCategory::all;

	std::array<SDL_Rect, SKILL_GUI_MAX> skillBtn;

	std::vector<SkillData> filteredSkills;

	std::wstring dragSkillTarget;  //드래그 중인 스킬 ID. 비어있으면 드래그 중 아님.

	void refilterSkills();  //categoryCursor 기준으로 filteredSkills 재구성 (생성자/클릭 공용)
public:
	Skill();
	~Skill();
	static Skill* ins() { return ptr; }
	void changeXY(int inputX, int inputY, bool center);
	void drawGUI();
	void clickUpGUI();
	void clickDownGUI();
	void clickRightGUI();
	void mouseWheel();
	void step();
};
