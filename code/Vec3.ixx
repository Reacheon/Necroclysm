export module Vec3;

import std;
import errorBox;

struct Vec3;
Vec3 getDefaultVec(int dir16);
Vec3 getZeroVec();

export struct Vec3
{
    float compX = 0;
    float compY = 0;
    float compZ = 0;

    Vec3()
    {
    };

    Vec3(float inputX, float inputY, float inputZ)
    {
        compX = inputX;
        compY = inputY;
        compZ = inputZ;
    }
    
    Vec3& operator=(const Vec3& ref)
    {
        this->compX = ref.compX;
        this->compY = ref.compY;
        this->compZ = ref.compZ;
        return *this;
    }

    void addVec(float addX, float addY, float addZ)
    {
        compX += addX;
        compY += addY;
        compZ += addZ;
    }

    void addVec(Vec3 addVec)
    {
        compX += addVec.compX;
        compY += addVec.compY;
        compZ += addVec.compZ;
    }

    float dotProduct(float addX, float addY, float addZ)
    {
        return addX * compX + addY * compY + addZ * compZ;
    }

    float dotProduct(Vec3 prodVec)
    {
        return prodVec.compX * compX + prodVec.compY * compY + prodVec.compZ * compZ;
    }

    float getLength()
    {
        return std::sqrt((float)std::pow(compX, 2)+ (float)std::pow(compY,2) + (float)std::pow(compZ, 2));
    }

    Vec3 getNormDirVec()
    {
        Vec3 dirVec;
        dirVec.compX = compX / getLength();
        dirVec.compY = compY / getLength();
        dirVec.compZ = compZ / getLength();
        return dirVec;
    }

    bool isZeroVec()
    {
        if (compX == 0)
        {
            if (compY == 0)
            {
                if (compZ == 0)
                {
                    return true;
                }
            }
        }

        return false;
    }
};

export int getNearDir16(Vec3 inputVec)
{
    Vec3 currentVec = inputVec.getNormDirVec();
    int maxDotProduct = -999;
    int maxDir16 = -1;
    for (int i = 0; i < 16; i++)
    {
        if (inputVec.dotProduct(getDefaultVec(i)) > maxDotProduct)
        {
            maxDotProduct = inputVec.dotProduct(getDefaultVec(i));
            maxDir16 = i;
        }
    }
    return maxDir16;
}

export Vec3 getDefaultVec(int dir16)
{
    if (dir16 == 0) return Vec3(1, 0, 0);
    else if (dir16 == 1) return Vec3(0.924, -0.383, 0.0);
    else if (dir16 == 2) return Vec3(0.707, -0.707, 0.0);
    else if (dir16 == 3) return Vec3(0.383, -0.924, 0.0);
    else if (dir16 == 4) return Vec3(0.0, -1.0, 0.0);
    else if (dir16 == 5) return Vec3(-0.383, -0.924, 0.0);
    else if (dir16 == 6) return Vec3(-0.707, -0.707, 0.0);
    else if (dir16 == 7) return Vec3(-0.924, -0.383, 0.0);
    else if (dir16 == 8) return Vec3(-1.0, -0.0, 0.0);
    else if (dir16 == 9) return Vec3(-0.924, 0.383, 0.0);
    else if (dir16 == 10) return Vec3(-0.707, 0.707, 0.0);
    else if (dir16 == 11) return Vec3(-0.383, 0.924, 0.0);
    else if (dir16 == 12) return Vec3(0.0, 1.0, 0.0);
    else if (dir16 == 13) return Vec3(0.383, 0.924, 0.0);
    else if (dir16 == 14) return Vec3(0.707, 0.707, 0.0);
    else if (dir16 == 15) return Vec3(0.924, 0.383, 0.0);
    else return Vec3(0, 0, 0);
}

export Vec3 getZeroVec()
{
    return Vec3(0, 0, 0);
}


export Vec3 scalarMultiple(Vec3 inputVec, float scalarVal)
{
    Vec3 rtnVec = inputVec;
    rtnVec.compX *= scalarVal;
    rtnVec.compY *= scalarVal;
    rtnVec.compZ *= scalarVal;
    return rtnVec;
}