#include <SDL.h>

export module Drawable;

//�ڱ� �ڽ��� �׸� �� �ִ� �������̽� : �����۽���, ��ƼƼ, ����, ��ġ��
export class Drawable
{
public:
    virtual void drawSelf() = 0;
};