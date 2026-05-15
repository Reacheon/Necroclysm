export module stringToWstring;

import std;
import utf8Decoder; //자체 UTF-8 디코더 사용, <codecvt>(C++26 제거 예정) 대체

export std::wstring stringToWstring(const std::string& str)
{
    return utf8Decoder(str);
}