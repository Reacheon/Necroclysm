#include <SDL3/SDL.h>

export module Skill;

import std;
import GUI;
import SkillData;

enum class skillCategory
{
	all,
	general,
	mutation,
	bionic,
	magic,
};

export class Skill : public GUI
{
private:
	inline static Skill* ptr = nullptr;
	SDL_Rect skillBase;
	SDL_Rect generalBox;
	SDL_Rect mutationBox;
	SDL_Rect bionicBox;
	SDL_Rect magicBox;
	int skillCursor = -1;
	int skillScroll = 0;
	int initSkillScroll = 0;

	skillCategory categoryCursor = skillCategory::general;

	std::array<SDL_Rect, 7> skillBtn;

	std::vector<SkillData> filteredSkills;

	int dragSkillTarget = -1;
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
