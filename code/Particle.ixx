#include <SDL.h>

export module Particle;

import Coord;
import std;
import globalVar;
import textureVar;
import constVar;
import drawText;
import Sprite;
import util;

export class Particle : public Coord
{
private:
    particleFlag type;
    float velX = 0.0;
    float velY = 0.0;
    float gradY = 0;
    
    int timer = 0;
    int lifetime;
public:
    Sprite* sprite = spr::particle;
    int sprIndex = 2;
    Uint8 alpha = 255;
    static std::vector<Particle*> list;
    Particle(int inputRealX, int inputRealY, int inputSprIndex, float inputVelX, float inputVelY, float inputGradY, int inputLifetime)
    {
        sprIndex = inputSprIndex;

        velX = inputVelX;
        velY = inputVelY;
        gradY = inputGradY;

        lifetime = inputLifetime;

        type = particleFlag::parabolic;
        list.push_back(this);
        setXY(inputRealX, inputRealY);
    }
    ~Particle()
    {
        list.erase(std::find(list.begin(), list.end(), this));
    }
    void step()
    {
        timer++;
        if (type == particleFlag::parabolic)
        {
            addFakeX(velX);
            addFakeY(velY);
            velY += gradY;
            if (timer >= lifetime)
            {
                if (alpha >= 15) alpha -= 15;
                else alpha = 0;
            }

            if (alpha == 0) delete this;
        }
        else if (type == particleFlag::leaf)
        {
        }
        else errorBox("알 수 없는 particleFlag가 step 이벤트에서 발생하였음");
    }
};

std::vector<Particle*> Particle::list;