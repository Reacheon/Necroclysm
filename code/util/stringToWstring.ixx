#include <string>
#include <codecvt>
#include <locale>

export module stringToWstring;

export std::wstring stringToWstring(const std::string& str) 
{
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}