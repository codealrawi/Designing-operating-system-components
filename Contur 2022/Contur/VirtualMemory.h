#pragma once
#ifndef __VirtualMemory_H__ 
#define __VirtualMemory_H__

#include "ArchitectureCPU.h"
#include "Memory.h"
#include "Process.h"
#include <iostream> 

using namespace std;

class VirtualMemory
{
private:
	int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES;
	int currImage;
	ProcessImage* image;

public:
	VirtualMemory(int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES)
	{
		this->SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES = SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES;
		image = new ProcessImage[SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES];
		currImage = 0;
	}
	/* ����������� ��� ���������� ���������� ������� ������������� ��������� ����� ��� ���������������� ������ */
	virtual void setAddrImage(int addr)
	{
		currImage = addr;
	}

	// �������� �� �������������� ������ ����� �������� 
	ProcessImage * getImage()
	{
		return &image[currImage];
	}

	/* ����������� ��� ������������� ������
	�� �������������� ������, �������� � ����������� ������ ��������� */
	virtual void setMemory(Memory* memory)
	{
		image[currImage].setStatus(false); // ����� ����� 
		image[currImage].setCode(memory);
	}

	/* read ����������� ��� ����������� ��������: �� ����������� ������ � ����������� */
	virtual Memory* read(int addr)
	{
		return image[addr].getCode();
	}

	/* �������� ������ ���������� ������, ���� ��� ������, �� Error, -1 ������������ ������� ��� ���������� ������� */
	virtual int getAddrFreeImage()
	{
		/* ���������� ������ ��������� ����� ��� ��������� */
		for (int i = 0; i < SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES; i++)
			if (image[i].getStatus())
			{
				// true VirtualMemory ����� �������� 
				image[i].setStatus(false);
				return i; // false �����
			}
		return -1; // ��� ��������� �������
	}
	/* ��� �������� ��������� �� �������: ���������� virtualMemory, ���������������� ��� */
	virtual void clearImage(int addr)
	{
		image[addr].setStatus(true);
		image[addr].setCode(NULL);
		image[addr].setAddr(-1);
		image[addr].setID(-1);
		image[addr].setPriority(-1);
		image[addr].setTimeSlice(-1);
		image[addr].setState(NotRunning);
		image[addr].setSysReg(NULL);
		image[addr].setUser("");
		image[addr].setVirtualAddr(-1);
		image[addr].clearTime();
		image[addr].setTimeSlice(-1);
	}

	virtual void clearMemory()
	{
		for (int i = 0; i < SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES; i++)
			clearImage(i);
		currImage = 0;
	}
	/* ����� ���������� ���������� ������ �������� � ����������� ������ */
	virtual void Debug()
	{
		for (int i = 0; i < SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES; i++)
		{
			cout << " VertualAddress = " << i << " status = " << image[i].getStatus() << " ID = " << image[i].getID() << endl;
		}
	}
};
#endif