#pragma once
#include "Memory.h"
#include "Process.h"

class MMU
{
private:
	Memory * memory;

public:
	MMU(Memory &memory) { this->memory = &memory; } /* �������� ��������� �� ������ �������� � �������� ������ */

	virtual void swapIn(ProcessImage * processImage)
	{ // �������� ��������� �� ������ �������� 
		Memory * code = processImage->getCode(); /* �������� ����� ���������� ��������� � ����������� ������, ���� �� ��� */
		int addrPCB = processImage->getAddr();
		int addrReal = getRealAddr(); /* ����� ���������� ��������� */

		if (addrPCB == -1)
		{ //�� ��������
		  /* ���� PC = -1, �� PC �� ������������������ ��������� ������ ��������� */
			processImage->getSysReg()->setState(PC, addrReal);
		}
		else
		{
			/* ���������� �������� � ������ 1. ���������� �������� PC - addrPCB, �������� ����� PC */
			int addrOffPC = processImage->getSysReg()->getState(PC) - addrPCB;
			processImage->getSysReg()->setState(PC, addrOffPC + addrReal);
		}

		// ��������� ����� � ����� PCB 
		processImage->setAddr(addrReal);

		/* ���������� ����� ���������� ��������� � ����������� ������ */
		setAlloc(addrReal);

		// ��������� ���� ����, ������ ��������� � ������ 
		Block * block = NULL;
		for (int i = 0; i < code->getSizeMemory(); i++)
		{
			block = code->read(i);
			/* ��� ��������� �������� � 0 ������ */
			/* �������� ���� � ������ ������ ���������������, �� ��������� ����� */
			if (!block->getState())
			{
				memory->setCmd(block->getCode(), block->getNregister(), block->getOperand());
			}
		}
	}

	/* �������� ��������� �� �������� ������, ����������� � ���������� ���������� */
	virtual void swapOut(ProcessImage * processImage)
	{
		/* �������� ����� ���������� ��������� � ����������� ������ �� ��� */
		int AddrFrom = processImage->getAddr();
		int AddrTo = AddrFrom + processImage->getCode()->getSizeMemory() - 1;
		memory->clearMemory(AddrFrom, AddrTo);
	}

	/* ���������� ��������� ����� ��� �������� ��������� �����������, ����������� ������ ���� ����, ��������� ��������� ���������� - ��� �� ������ ��� */
	virtual int getRealAddr() { return this->memory->getAddrFreeBlock(); }

	// ���������� ����� ���������� ��������� � ������ 
	virtual void setAlloc(int addr) { memory->setAddrBlock(addr); }

	// �������� ���������� swapIn
	virtual void Debug(HANDLE * handle) {
		// �������� ��������� �� ������ �������� 
		ProcessImage * processImage = (ProcessImage *)handle;

		// ��. void swapin(Processimage * processimage) 
		Memory * code = processImage->getCode();

		/* �������� ����� ���������� ��������� � ����������� ������, ���� �� ��� */
		cout << endl;
		cout << "-get address of loading programm in memory & set PC" << endl;

		int addrPCB = processImage->getAddr();
		cout << "addrPCB = " << addrPCB << " address of loading programm from PCB" << endl;

		int addrReal = getRealAddr();
		/* ����� ���������� ��������� */
		cout << "addrReal= " << addrReal << " address of loading programm Real" << endl;

		if (addrPCB == -1) {/* �� ��������, ���� PC = -1,�� PC �� ������������������, ��������� ������ ��������� */
			cout << " if address in PCB == -1 set addrReal PC = " << addrReal << endl;
		}
		else {	/* ���������� �������� � ������ 1. ���������� �������� PC - addrPCB � �������� ����� PC */
			cout << "PC = " << processImage->getSysReg()->getState(PC) << " address in PC " << endl;
			int addrOffPC = processImage->getSysReg()->getState(PC) - addrPCB;
			cout << "addrOffPC = PC - addrPCB = " << addrOffPC << " address off set" << endl;
			cout << "PC = addrOffPC + addrReal = " << addrOffPC + addrReal << endl;
		}

		// ��������� ���� ����, ������ ��������� � ������ 
		Block * block = NULL;
		cout << endl;
		cout << "read block from process image code to memory" << endl;
		cout << "size code = " << code->getSizeMemory() << endl;
		for (int i = 0; i < code->getSizeMemory(); i++) {
			block = code->read(i); /* ��� ��������� �������� � 0 ������, �������� ���� � ������ ������ ���������������, �� ��������� ����� */
			if (!block->getState()) {
				block->debugBlock();
			}
		}
	}

	virtual void DebugMemory() { memory->debugHeap(); }
};
