export module connectGroupExtraIndex;

//@brief �����¿� Ÿ�Ͽ� ���� ���� Ÿ���� �߰��ε���(0~15)�� ��ȯ�ϴ� �Լ�
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