module GodRegistry;

import std;
import GodBehavior;
import GodRehylion;

void GodRegistry::init()
{
	registerGod(std::make_unique<GodRehylion>());
}
