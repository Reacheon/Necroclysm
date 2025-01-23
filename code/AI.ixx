export module AI;

export class AI
{
private:
    bool isActivate = true;
    float turnResource = 0;

public:
    //��� AI ������ ������ true ��ȯ
    virtual bool runAI() = 0;
    bool getActivate() { return isActivate; }
    void activateAI() { isActivate = true; }
    void deactivateAI() { isActivate = false; }
    float getTurnResource() { return turnResource; }
    void addTurnResource(float inputVal) { turnResource += inputVal; }
    void useTurnResource(float inputVal) { turnResource -= inputVal; }
    void clearTurnResource() { turnResource = 0; }
};