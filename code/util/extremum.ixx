export module extremum;

export template <typename T>
T myMax(T value) 
{
    return value;
}

export template <typename T, typename... Args>
T myMax(T value, Args... args)
{
    T maxRest = myMax(args...);
    return (value > maxRest) ? value : maxRest;
}

export template <typename T>
T myMin(T value) 
{
    return value;
}

export template <typename T, typename... Args>
T myMin(T value, Args... args) 
{
    T minRest = myMin(args...);
    return (value < minRest) ? value : minRest;
}