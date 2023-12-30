#ifndef __ArchitectureCPU_H__
#define __ArchitectureCPU_H__

#include <iostream>
using namespace std;

enum Instruction {
	Mov, Add, Sub, Mul, Div, And, Or, Xor, Shlu,
	Shru, Shls, Shrs, Cmp, Je, Jne, Jge, Jgt, Jle,
	Jlt, Int, Pushw, Pushc, Popw, Popc, Rmem, Wmem
};
/*
OK - ���������� ���������� ���������
Error - ������ � ���������
Empty - ������ ������� ����� ������
��� ��������� �� ���������� ������ ����������� ����������� Exit
*/
enum Interrupt { OK = 0, Error = -1, Empty = 2, Exit = 16, Sys = 11, Div_0 = 3, Lan = 32, Dev = 254 };

// ������ ���������� ������������� ������� ����� ������

class Block
{
public:
	/* state ��������� ����� ����� - false,
	�������� - true */
	Block()
	{
		state = true; code = 0; nregister = 0; operand = 0;
	}
	void setState(bool state) { this->state = state; }
	bool getState() { return state; }

	void setCode(int code) { this->code = code; }
	int getCode() { return code; }

	void setNregister(int nregister) { this->nregister = nregister; }
	int getNregister() { return nregister; }

	void setOperand(int operand) { this->operand = operand; }
	int getOperand() { return operand; }

	void debugBlock()
	{
		cout << " state=" << getState()
			<< " code=" << getCode()
			<< " nregister=" << getNregister()
			<< " operand = " << getOperand() << endl;
	}

	/* ������ ���������� [�������] �[����� ��������]
	[$,�][����� �������� ��� ���������] � � � */
private:
	bool state;	     // ���� ����� ��� ��������
	_int8 code;	     // � - ����, ��� �������
	_int8 nregister; //� - ����, ����� ��������
	int operand;     // � - int, ������� [����� �������� ��� ���������]
};

#define NUMBER_OF_REGISTERS 16
#define SIZE_OF_REGISTER_NAMES 4 /* ���� ������ ��������� ��� ���������� ������ strcat_s */ 
enum Name { r1, r2, r3, r4, r5, ��, r7, r8, r9, rlO, rll, rl2, rl3, rl4, PC, SP };

/* ����������� ������������� ������������� operator ��� SysReg */
inline Name operator++ (Name &rs, int) { return rs = (Name)(rs + 1); };

/* PC ����������� �������, ����������� �� ���������� ����� ������, �� �������� ����� ������� ��������� ������� */

class SysReg {
public:
	SysReg()
	{
		int j = 0, i = 1; /* j ������� � ������� �� 0, i - ����� c 1*/
		for (Name curName = r1; curName <= SP; curName++)
		{
			register_[j++] = new Register(i++, -1, curName);
			status = true;/* �������� �� ������������ ���������, ��� ������������ */
		}
	}

	int getState(Name name)
	{
		for (int i = 0; i < NUMBER_OF_REGISTERS; i++)
		{
			if (register_[i]->getName() == name)
			{
				return (register_[i]->getState());
			}
		}
		return -1; //some system Error
	}

	void setState(_int8 nregister, int operand)
	{
		// ������ ��������� � ����� ����
		for (int i = 0; i < NUMBER_OF_REGISTERS; i++)
		{
			if (register_[i]->getName() == nregister)
			{
				register_[i]->setState(operand);
				return;
			}
		}
	}
	/* �������� ���������� ��������� ����� ��������� ��������� */
	void clearSysReg()
	{
		for (int i = 0; i < NUMBER_OF_REGISTERS; i++)
		{
			register_[i]->setState(-1);
		}
	}

	void Debug()
	{ // ���������� � ���������
		for (int i = 0; i < NUMBER_OF_REGISTERS; i++)
			cout << "register_[" << i << "] " << " numreg= " <<
			register_[i]->getNumReg() << " state = " <<
			register_[i]->getState() << " name = " <<
			register_[i]->getName() << " NameOf= " <<
			register_[i]->NameOf() << endl;
		cout << endl;
	}

	class Register
	{
	public:
		/* ���������� ���� �������������: �� ��������� � ������������,
		������� ������������� ��������� � �������� ����� �������� */
		Register() {};

		Register(int numreg, int state, Name name)
		{
			this->numreg = numreg;
			this->state = state;
			this->name = name;
		};

		/* get and set functions.
		numReg - ���������� ������������� ������ ��������,
		��������, ��� r1 numReg = 1, ����������� � ������ */

		int	getNumReg() { return numreg; }; /* �������� r����������� ����� */
		void setNumReg(int numreg) { this->numreg = numreg; }; // ���������� ������������ ����� 
		int getState() { return state; }; /* �������� ��������� �������� */
		void setState(int state) { this->state = state; }; // ���������� ��������� ��������

		Name getName() { return name; }; /* �������� code �������� � enum */
		void setName(Name name) { this->name = name; };	// ���������� code �������� � enum */

		char* NameOf() {
			/* �������� string ������������� �������� */
			static char szName[SIZE_OF_REGISTER_NAMES];
			const char* Numbers[] = { "1" , "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14" }; /////////// char ������� �� const

			if (getName() <= 13)
			{
				strcpy_s(szName, SIZE_OF_REGISTER_NAMES, "r");
				strcat_s(szName, SIZE_OF_REGISTER_NAMES, Numbers[getName()]);
				return szName;
			}

			switch (getName())
			{
			case PC:
				strcpy_s(szName, SIZE_OF_REGISTER_NAMES, "PC");
				break;

			case SP:
				strcpy_s(szName, SIZE_OF_REGISTER_NAMES, "SP");
				break;
			}
			return szName;
		}
	private:
		int numreg;
		Name name;
		int state;
	}; // end Register


	bool getStatus() { return status; }
	void setStatus(bool status) { this->status = status; }

private:
	Register * register_[NUMBER_OF_REGISTERS];
	bool status;
	/* true - �������� �� ������������ ���������
	false - �������� ���������� ������� */
}; // end SysReg

   // ��������� ��������
#define NUMBER_OF_STATE 7
enum State { NotRunning = 0, Running = 1, Blocked = 2, Swapped = 3, ExitProcess = 4, New = 5, Ready = 6 };

/* New - �����, ��� ���������� ��������� ��������� ����� ������� (�� ������������)
Ready - ������� (�� ������������)
Running - �������������
Blocked - �������������
ExitProcess - �������������
Swapped - �����������, ������� � ����������
NotRunning - ��������������� */

class Timer
{
public:
	Timer() { time = 0; }
	static void setTime() { time = 0; }
	static int getTime() { return time; }
	static void Tick() { time++; }
private:
	/* �� �������� ��������������� ������������� �����, �� CPU �� ������������ - �������� � ������������ */
	static int time;
};

int Timer::time;
/* ��� ���������� ������� ������ Statistic, �������� ���� �� ���������� �������
TimeExec - �������������� ����� ����������
TimeServ - �������������� ����� ������������
�������� StatTime ������������� � ������������ �
��������� �������, ��� ����������� ��� ��������� ������� */
enum StatTime { TimeExecNotPreem = 0, TimeExec = 1, TimeServ = 2, noParam = -1 };

#define NUMBER_OF_PARAM 3

/* ����������������� ����� ���������� �����������
������� getShortestRemainTime, ������� ����������
�������� ������ Dispatcher ����� ������� ���������� */

class VectorParam
{
	/* ������ ���������� �������, ����������� � Scheduler */
public:
	VectorParam()
	{
		for (int i = 0; i < NUMBER_OF_PARAM; i++)
		{
			param[i] = new Param();
		}
	} //

	void setState(StatTime time) { param[time]->state = true; }
	bool getState(StatTime time) { return param[time]->state; }

	StatTime getStateTime()
	{
		for (int i = 0; i < NUMBER_OF_PARAM; i++)
		{
			if (param[i]->state)
			{
				if (i == 0)	return	TimeExecNotPreem;
				if (i == 1)	return	TimeExec;
				if (i == 2)	return	TimeServ;
			}
		}
		return noParam;
	} //

	void resetState(StatTime time)
	{
		param[time]->state = false;
	}

	void setTthreshold(StatTime time, double Tthreshold)
	{
		param[time]->Tthreshold = Tthreshold;
	}

	double getTthreshold(StatTime time)
	{
		return param[time]->Tthreshold;
	}

	void clearVectorParam()
	{
		for (int i = 0; i < NUMBER_OF_PARAM; i++)
		{
			param[i]->state = false;
			param[i]->Tthreshold = -1;
		}
	}

private:
	class Param
	{
	public:
		Param() { Tthreshold = -1; state = false; }
		bool state; /* ��������� ��������� ���������� ������� */
		double Tthreshold; /* ����� ������������������ ������� */
	};
	Param* param[NUMBER_OF_PARAM];
};
#endif