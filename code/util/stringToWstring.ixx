export module stringToWstring;

import std;
import utf8Decoder;

export std::wstring stringToWstring(const std::string& str)
{
    return utf8Decoder(str);
}