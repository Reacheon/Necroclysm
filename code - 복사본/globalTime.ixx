export module globalTime;

import constVar;

static int year = 2099;
static int month = 4;
static int day  = 15;
static int hour = 9;
static int min = 9;
static int sec = 0;

export int getYear() { return year; }
export int getMonth() { return month; }
export int getDay() { return day; }
export int getHour() { return hour; }
export int getMin() { return min; }
export int getSec() { return sec; }
export seasonFlag getSeason()
{
    if (getMonth() == 12 || getMonth() == 1 || getMonth() == 2) return seasonFlag::winter;
    else if (getMonth() == 3 || getMonth() == 4 || getMonth() == 5) return seasonFlag::spring;
    else if (getMonth() == 6 || getMonth() == 7 || getMonth() == 8) return seasonFlag::summer;
    else return seasonFlag::autumn;
}

void addTimeMonth(int inputMonth)
{
    month += inputMonth;
    while (month > 12)
    {
       month -= 12;
       year++;
    }
}

void addTimeDay(int inputDay)
{
    day += inputDay;

    int endOfMonth;
    switch (month)
    {
        case 1:
        case 3:
        case 5:
        case 7:
        case 9:
        case 11:
            endOfMonth = 31;
            break;
        case 2:
            if (year % 4 == 0)
            {
                if (year % 100 == 0 && year % 400 != 0)
                {
                    endOfMonth = 28;
                }
                else
                {
                    endOfMonth = 29;
                }
            }
            else
            {
                endOfMonth = 28;
            }
            break;
        case 4:
        case 6:
        case 8:
        case 10:
        case 12:
            endOfMonth = 30;
            break;
    }

    while (day > endOfMonth)
    {
        day -= endOfMonth;
        addTimeMonth(1);
    }
}

void addTimeHour(int inputHour)
{
    hour += inputHour;
    while (hour > 24)
    {
        hour -= 24;
        addTimeDay(1);
    }
}

void addTimeMinInt(int inputMin)
{
    min += inputMin;
    while (min > 60)
    {
        min -= 60;
        addTimeHour(1);
    }
}

void addTimeSec(int inputSec)
{
    sec += inputSec;
    while (sec > 60)
    {
        sec -= 60;
        addTimeMinInt(1);
    }
}

export void setDebugTime(int inputHour, int inputMin)
{
    hour = inputHour;
    min = inputMin;
}

export void setDebugDay(int inputMonth, int inputDay)
{
    month = inputMonth;
    day = inputDay;
}

export void addTimeTurn(float inputTurn)
{
    addTimeSec((int)(60.0 * (inputTurn - (int)inputTurn)));

    min += (int)(inputTurn);
    while (min > 60)
    {
        min -= 60;
        addTimeHour(1);
    }
}