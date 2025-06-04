export module HSVConverter;

import std;
import extremum;

export void HSV2RGB(int& colorH, int& colorS, int& colorV, int& colorR, int& colorG, int& colorB)
{
    float primeH = (float)colorH / 60.0;
    float primeS = colorS / 100.0;
    float primeV = colorV / 100.0;

    float c = primeV * primeS;
    float x = c * (1 - std::fabs(std::fmod(primeH, 2) - 1));
    float m = primeV - c;

    float primeR;
    float primeG;
    float primeB;

    if (primeH >= 0 && primeH < 1)
    {
        primeR = c;
        primeG = x;
        primeB = 0;
    }
    else if (primeH >= 1 && primeH < 2)
    {
        primeR = x;
        primeG = c;
        primeB = 0;
    }
    else if (primeH >= 2.0 && primeH < 3)
    {
        primeR = 0;
        primeG = c;
        primeB = x;
    }
    else if (primeH >= 3 && primeH < 4)
    {
        primeR = 0;
        primeG = x;
        primeB = c;
    }
    else if (primeH >= 4 && primeH < 5)
    {
        primeR = x;
        primeG = 0;
        primeB = c;
    }
    else
    {
        primeR = c;
        primeG = 0;
        primeB = x;
    }

    colorR = std::round((primeR + m) * 255.0);
    colorG = std::round((primeG + m) * 255.0);
    colorB = std::round((primeB + m) * 255.0);
}

export void RGB2HSV(int& colorR, int& colorG, int& colorB, int& colorH, int& colorS, int& colorV)
{
    float primeR = colorR / 255.0;
    float primeG = colorG / 255.0;
    float primeB = colorB / 255.0;

    
    float maxColor = myMax(primeR, primeG, primeB);
    float minColor = myMin(primeR, primeG, primeB);
    float deltaColor = maxColor - minColor;

    //calculate h
    if (deltaColor == 0)
    {
        colorH = 0;
    }
    else if (maxColor == primeR)
    {
        colorH = std::round(60 * std::fmod((primeG - primeB) / deltaColor, 6));
    }
    else if (maxColor == primeG)
    {
        colorH = std::round(60 * (((primeB - primeR) / deltaColor) + 2));
    }
    else
    {
        colorH = std::round(60 * (((primeR - primeG) / deltaColor) + 4));
    }

    //calculate s
    if (maxColor == 0)
    {
        colorS = 0;
    }
    else
    {
        colorS = std::round(100 * deltaColor / maxColor);
    }

    //calculate v
    colorV = std::round(100 * maxColor);
}