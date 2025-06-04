export module connectGroupExtraIndex;

//@brief 상하좌우 타일에 따라 현재 타일의 추가인덱스(0~15)를 반환하는 함수
export int connectGroupExtraIndex(bool topCheck, bool botCheck, bool leftCheck, bool rightCheck)
{
	if (!topCheck && !botCheck && !leftCheck && !rightCheck) return 1;

	else if (topCheck && !botCheck && !leftCheck && !rightCheck) return 13;
	else if (!topCheck && botCheck && !leftCheck && !rightCheck) return 11;
	else if (!topCheck && !botCheck && leftCheck && !rightCheck) return 10;
	else if (!topCheck && !botCheck && !leftCheck && rightCheck) return 12;

	else if (topCheck && botCheck && !leftCheck && !rightCheck) return 15;
	else if (topCheck && !botCheck && leftCheck && !rightCheck) return 9;
	else if (topCheck && !botCheck && !leftCheck && rightCheck) return 7;

	else if (!topCheck && botCheck && leftCheck && !rightCheck) return 3;
	else if (!topCheck && botCheck && !leftCheck && rightCheck) return 5;

	else if (!topCheck && !botCheck && leftCheck && rightCheck) return 14;

	else if (topCheck && botCheck && leftCheck && !rightCheck) return 2;
	else if (topCheck && botCheck && !leftCheck && rightCheck) return 6;
	else if (topCheck && !botCheck && leftCheck && rightCheck) return 8;
	else if (!topCheck && botCheck && leftCheck && rightCheck) return 4;

	else return 0;
}