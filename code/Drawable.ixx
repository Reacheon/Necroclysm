#include <SDL.h>

export module Drawable;

//자기 자신을 그릴 수 있는 인터페이스 : 아이템스택, 엔티티, 차량, 설치물
export class Drawable
{
public:
    virtual void drawSelf() = 0;
};