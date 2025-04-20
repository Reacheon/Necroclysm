export module extremum;

//최대값과 최소값을 가변인자로 구해주는 함수
//chatGPT가 만듬
//주의할 점은 맨 앞에 있는 인수의 자료형을 따르므로 다른 자료형을 넣을 때 꼭 같은 자료형으로 변환해줄 것

export template <typename T>
T myMax(T value) {
    return value;
}

export template <typename T, typename... Args>
T myMax(T value, Args... args)
{
    T maxRest = myMax(args...);
    return (value > maxRest) ? value : maxRest;
}

export template <typename T>
T myMin(T value) {
    return value;
}

// Recursive template function to find the minimum value
export template <typename T, typename... Args>
T myMin(T value, Args... args) {
    T minRest = myMin(args...);
    return (value < minRest) ? value : minRest;
}