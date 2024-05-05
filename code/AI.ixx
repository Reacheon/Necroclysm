export module AI;

export class AI
{
private:
    bool isActivate = true;
    float timeResource = 0;

public:
    //��� AI ������ ������ true ��ȯ
    virtual bool runAI() = 0;
    bool getActivate() { return isActivate; }
    void activateAI() { isActivate = true; }
    void deactivateAI(){ isActivate = false; }
    float getTimeResource() { return timeResource; }
    void addTimeResource(float inputVal) { timeResource += inputVal; }
    void useTimeResource(float inputVal) { timeResource -= inputVal; }
    void clearTimeResource() { timeResource = 0; }
};