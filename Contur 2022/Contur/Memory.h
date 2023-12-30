#ifndef	__Memory_H__
#define	__Memory_H__

#include <iostream>
#include "ArchitectureCPU.h"

using namespace std;

class Memory
{
private:
	int SIZE_OF_MEMORY_IN_BLOCKS;
	int currBlock;
	Block *heap;

	void clearBlock(int addr)
	{
		heap[addr].setState(true);
		heap[addr].setCode(0);
		heap[addr].setNregister(0);
		heap[addr].setOperand(0);
	}
public:
	Memory(int SIZE_OF_MEMORY_IN_BLOCKS)
	{
		currBlock = 0;
		this->SIZE_OF_MEMORY_IN_BLOCKS = SIZE_OF_MEMORY_IN_BLOCKS;
		heap = new Block[SIZE_OF_MEMORY_IN_BLOCKS];
	}

	/* ����������� ��� ���������� ���������� �� ����� ������� ������������� ��������� ����� ��� ���������������� ������ */
	void setAddrBlock(int addr)
	{
		currBlock = addr;
	}

	// ����������� ��� ���������� ���������� �� ����� 
	void setCmd(int cmd, int op1, int op2)
	{
		int addr = currBlock++;
		if (addr == -1) {
			cout << "No free blocks" << endl;
			return;
		}
		heap[addr].setState(false);
		heap[addr].setCode(cmd);
		heap[addr].setNregister(op1);
		heap[addr].setOperand(op2);
	}

	/* read ����������� ��� ���������� ���������� CPU � ��� ����������� �������� � ����������� ������� */
	Block* read(int addr)
	{
		return &heap[addr];
	}

	/* �������� ������ ���������� �����, ���� ��� �����, �� Error */
	// ������������ ������� ��� ���������� ������� 
	int getAddrFreeBlock()
	{ /* ������� ������ ��������� ���� ��� ��������� */
		for (int i = 0; i < SIZE_OF_MEMORY_IN_BLOCKS; i++)
			if (heap[i].getState()) return i;
		return -1;	// ��� ��������� ������
	}

	/* ����� ����������� ��� ��������� �� �������;
	��� ���������� � ���������� ����������� ����� ������ ���� �������. ������������ � CPU, ��� ����������� �������� PC */
	int getNextAddr(int currAddr)
	{
		return currAddr + 1;
	}

	void clearMemory()
	{	// ������� ���� ������
		for (int i = 0; i < SIZE_OF_MEMORY_IN_BLOCKS; i++)
			clearBlock(i);
		currBlock = 0;
	}

	void clearMemory(int AddrFrom, int AddrTo)
	{
		// ������� ����� ������, ������� ���� �� �������� 
		for (int i = AddrFrom; i < SIZE_OF_MEMORY_IN_BLOCKS; i++)
		{
			if (i > AddrTo) return;
			clearBlock(i);
		}
	}

	int getSizeMemory()
	{
		return SIZE_OF_MEMORY_IN_BLOCKS;
	}
	// ����� ���������� ���������� ������ 

	void debugHeap()
	{
		for (int i = 0; i < SIZE_OF_MEMORY_IN_BLOCKS; i++)
		{
			cout << "heap[" << i << "] = " << " state=" <<
				heap[i].getState() << " code=" <<
				heap[i].getCode() << " nregister=" <<
				heap[i].getNregister() << " operand=" <<
				heap[i].getOperand() << endl;
		}
	}

};

class Code : public Memory
{
public:
	Code(int SIZE_OF_PROGRAMM) : Memory(SIZE_OF_PROGRAMM) {}

	virtual  ~Code()
	{
		cout << " object Code deleted" << endl;
	}
};
#endif
