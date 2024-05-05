export module calc;

import extremum;


export namespace calcMelee
{
    float maxDmg(int weaponAtk, int str, float fightingTalent, float weaponTalent)
    {
        float fightTotal = (1/3)*fightingTalent + (2/3)*weaponTalent;
        return weaponAtk + 0.5*str + 10*(fightTotal/27);
    } 
    float minDmg(int weaponAtk , float balance, int dex, float fightingTalent, float weaponTalent)
    {
        float fightTotal = (1/3)*fightingTalent + (2/3)*weaponTalent;
        return weaponAtk*balance + dex + 15*(fightTotal/27);
    }
    float acc(float partAcc, float weaponAcc, int dex, float fightingTalent, float weaponTalent, int aimStack)
    {
        float fightTotal = (1 / 3) * fightingTalent + (2 / 3) * weaponTalent;
        //(부위명중률 + (0.4)(1-부위명중률)(dex/10) + (0.4)(1-부위명중률)(fightTotal/27))*(무기명중률)
        float accVal = (partAcc + (1 - partAcc) * (0.4) * (dex / 10) + (1 - partAcc) * (0.4) * (fightTotal / 27)) * weaponAcc;
        if (aimStack > 0)//조준 스택 보정
        {
            float residue = 1 - accVal;
            accVal += 1 / (-0.1 * aimStack * (20 / residue)) + residue;
        }
        if (accVal > 0.99) { accVal = 0.99; };
        return accVal;
    }
    float atkSpd(float weaponSpd, int str, float fightingTalent, float weaponTalent)
    {
        float fightTotal = (1 / 3) * fightingTalent + (2 / 3) * weaponTalent;
        return weaponSpd + (0.02 * str) + 0.1 * (fightTotal / 27);
    }
};

export namespace calcUnarmed
{
    float maxDmg(int str, float fightingTalent, float unarmedCombat)
    {
        float fightTotal = (1 / 3) * fightingTalent + (2 / 3) * unarmedCombat;
        return 2 * str + 10 * (fightTotal / 27);
    }
    float minDmg(int dex, float fightingTalent, float unarmedCombat)
    {
        float fightTotal = (1 / 3) * fightingTalent + (2 / 3) * unarmedCombat;
        return dex + 9 * (fightTotal / 27);
    }
    float acc(float partAcc, float weaponAcc, int dex, float fightingTalent, float unarmedCombat, int aimStack)
    {        
        float fightTotal = (1 / 3) * fightingTalent + (2 / 3) * unarmedCombat;
        //(부위명중률 + (0.4)(1-부위명중률)(dex/10) + (0.4)(1-부위명중률)(fightTotal/27))*(무기명중률)
        float accVal = (partAcc + (1-partAcc)*(0.4)*(dex/10) + (1-partAcc)*(0.4)*(fightTotal/27))*weaponAcc;   
        if (aimStack > 0)//조준 스택 보정
        {
            float residue = 1 - accVal;
            accVal += 1 / (-0.1 * aimStack * (20 / residue)) + residue;
        }
        if (accVal > 0.99) { accVal = 0.99; };
        return accVal;
    }
    float atkSpd(int str, float fightingTalent, float unarmedCombat)
    {
        float fightTotal = (1 / 3) * fightingTalent + (2 / 3) * unarmedCombat;
        return 1.00 + (0.02 * str) + 0.1 * (fightTotal / 27);
    }
};

export namespace calcShot
{
    float maxDmg(int bulletAtk, float gunPower)
    {
        return bulletAtk*gunPower;
    }
    float minDmg(int bulletAtk, float gunBalance)
    {
        return bulletAtk*gunBalance;
    }
    float acc(float partAcc, float gunAcc, int dex, float shooting, int aimStack, int distance)
    {
        float accVal = (partAcc + (1 - partAcc) * (0.4) * (dex / 10) + (1 - partAcc) * (0.4) * (shooting / 27)) * gunAcc - 0.05 * distance;
        if (accVal < 0) { accVal = 0; }

        if (aimStack > 0)//조준 스택 보정
        {
            float residue = 1 - accVal;
            accVal += 1 / (-0.1 * aimStack * (20 / residue)) + residue;
        }
        if (accVal > 0.99) { accVal = 0.99; };
        return accVal;
    }
    float atkSpd(float gunShotSpd)
    {
        return gunShotSpd;
    }
};

export namespace calcThrow
{
    float maxDmg(int throwAtk, int str, float throwing)
    {
        return throwAtk + str + 10*(throwing/27);
    }
    float minDmg(int throwAtk, float throwBalance, int dex, float throwing )
    {
        return (throwAtk*throwBalance) + dex + 15*(throwing/27);
    }
    float acc(float partAcc, float throwAcc, int dex, float throwing, int aimStack, int distance)
    {
        float accVal = (partAcc + (1 - partAcc) * (0.4) * (dex / 10) + (1 - partAcc) * (0.4) * (throwing / 27)) * throwAcc - 0.05*distance;
        if (accVal < 0) { accVal = 0; }
        
        if (aimStack > 0)//조준 스택 보정
        {
            float residue = 1 - accVal;
            accVal += 1 / (-0.1 * aimStack * (20 / residue)) + residue;
        }
        if (accVal > 0.99) { accVal = 0.99; };
        return accVal;
    }
    float atkSpd()
    {
        return 1.0;
    }
};