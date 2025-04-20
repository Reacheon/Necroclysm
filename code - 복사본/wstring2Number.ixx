export module wstring2Number;

import std;

export int wtoi(const std::wstring& input) {
    std::wstringstream wss(input);
    int result;
    wss >> result;
    if (wss.fail()) {
        // handle error
        throw std::invalid_argument("Invalid integer format");
    }
    return result;
}

export float wtof(const std::wstring& input) {
    std::wstringstream wss(input);
    float result;
    wss >> result;
    if (wss.fail()) {
        // handle error
        throw std::invalid_argument("Invalid float format");
    }
    return result;
}
