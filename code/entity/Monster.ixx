#include <sol/sol.hpp>

export module Monster;

import std;
import Entity;
import util;
import AI;

export class Monster : public Entity, public AI
{
private:
	std::string scriptAI;
public:
	Monster(int index, int gridX, int gridY, int gridZ);
	~Monster();
	virtual void startAtk(int inputGridX, int inputGridY, int inputGridZ, aniFlag inputAniType) override;
	void setGrid(int inputGridX, int inputGridY, int inputGridZ) override;
	void startAtk(int inputGridX, int inputGridY, int inputGridZ);
	void endMove();
	bool runAI();
	void death();
};
