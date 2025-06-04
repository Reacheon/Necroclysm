export module replaceStr;

import std;

//입력한 문자열의 해당 문자를 바꿔치기한다.
export std::wstring replaceStr(std::wstring inputStr, std::wstring targetFrag, std::wstring newFrag)
{
	return inputStr.replace(inputStr.find(targetFrag), targetFrag.length(), newFrag);
}

export std::wstring replaceStr(std::wstring inputStr, std::wstring targetFrag, const std::vector<std::wstring>& newFrag)
{
	std::wstring rtnStr = inputStr;
	for (int i = 0; i < newFrag.size(); i++) rtnStr = replaceStr(rtnStr, targetFrag,newFrag[i]);
	return rtnStr;
}