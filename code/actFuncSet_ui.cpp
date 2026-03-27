module actFuncSet;

import util;
import constVar;
import globalVar;
import log;
import Msg;

namespace actFunc
{
	//아이템 수량 선택 (Loot, Inventory 공통) — 우클릭 시 수량 직접 입력
	Corouter selectItemEx(ItemPocket* pocket, int index)
	{
		std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
		exInputText.clear();
		new Msg(msgFlag::input, sysStr[40], sysStr[39], choiceVec);//아이템 선택, 얼마나?
		co_await std::suspend_always();

		if (exInputText.empty() == false)
		{
			int inputSelectNumber = 0;
			try { inputSelectNumber = wtoi(exInputText.c_str()); }
			catch (...) {}

			if (inputSelectNumber < 0) inputSelectNumber = 0;
			else if (inputSelectNumber > pocket->itemInfo[index].number)
				inputSelectNumber = pocket->itemInfo[index].number;

			pocket->itemInfo[index].lootSelect = inputSelectNumber;
		}
	}

	//아이템 검색 (Loot, Inventory 공통) — 키워드로 아이템을 필터링
	Corouter searchItems(ItemPocket* pocket, int& scroll)
	{
		//이미 검색 중인지 체크
		for (int i = 0; i < pocket->itemInfo.size(); i++)
		{
			if (pocket->itemInfo[i].checkFlag(itemFlag::GRAYFILTER))//이미 검색 중일 경우 검색 상태를 해제함
			{
				for (int j = 0; j < pocket->itemInfo.size(); j++)
				{
					pocket->itemInfo[j].eraseFlag(itemFlag::GRAYFILTER);
				}
				pocket->sortByUnicode();
				updateLog(sysStr[86]);//검색 상태를 해제했다.
				co_return;
			}

			if (i == pocket->itemInfo.size() - 1)//검색 중이 아닐 경우
			{
				std::vector<std::wstring> choiceVec = { sysStr[38], sysStr[35] };//확인, 취소
				new Msg(msgFlag::input, sysStr[27], sysStr[97], choiceVec);//검색, 검색할 키워드를 입력해주세요
				scroll = 0;
				co_await std::suspend_always();
				if (coAnswer == sysStr[38])
				{
					int matchCount = pocket->searchTxt(exInputText);
					for (int i = 0; i < pocket->itemInfo.size(); i++) pocket->itemInfo[i].addFlag(itemFlag::GRAYFILTER);
					for (int i = 0; i < matchCount; i++) pocket->itemInfo[i].eraseFlag(itemFlag::GRAYFILTER);
				}
				else {}
			}
		}
	}
}
