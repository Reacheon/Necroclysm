export module Mapmaker;

import std;
import util;
import constVar;

// Mapmaker
// ������ ������ ûũ���� ����ϴ� ��ü (ū ������ �������� ������� ����)
// �÷��̾� �ֺ��� ���� ��ô� ���� ���Ϸ� ������
// ���� ����� ����

export class Mapmaker
{
private:
	std::unordered_map<std::array<int, 3>, chunkFlag, decltype(arrayHasher3)> prophecy;
public:
	static Mapmaker* ins()//�̱��� �Լ�
	{
		static Mapmaker* ptr = new Mapmaker();
		return ptr;
	}
	void addProphecy(int chunkX, int chunkY, int chunkZ, chunkFlag input)
	{
		//errorBox(prophecy.find({ chunkX,chunkY,chunkZ }) == prophecy.end(), L"�̹� ��õ� ûũ�� Mapmaker�� �Էµ�");
		prophecy[{ chunkX, chunkY, chunkZ }] = input;
	}
	chunkFlag getProphecy(int chunkX, int chunkY, int chunkZ)
	{
		errorBox(prophecy.find({ chunkX,chunkY,chunkZ }) == prophecy.end(), L"�� ûũ�� Mapmaker�� �������� ����");
		return prophecy[{ chunkX, chunkY, chunkZ }];
	}
	bool isEmptyProphecy(int chunkX, int chunkY, int chunkZ)
	{
		if (prophecy.find({ chunkX,chunkY,chunkZ }) == prophecy.end()) return true;
		else return false;
	}
};