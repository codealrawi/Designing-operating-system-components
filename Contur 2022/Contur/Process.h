#ifndef  __Process_H__
#define  __Process_H__

#include "HANDLE.h"
#include "ArchitectureCPU.h"

/* ��������� �������� ��.
State {New, Ready, Running, Blocked, ExitProcess, NotRunning);
New - �����, ��� ���������� ��������� ��������� ����� ������� Ready - �������
Running - �������������
Blocked - �������������
ExitProcess - �������������
NotRunning - ��������������� */

// ���������� ������ �� ���� ��������� 
class Process : public HANDLE
{
public:
	Process() {
		this->user = "";
		state = NotRunning;
	}

	virtual ~Process() { cout << "object Process deleted" << endl; }

	string getUser() { return user; }

	void setUser(string user) { this->user = user; }

	int getID() { return HANDLE::getID(); }

	void setID(int ID) { HANDLE::setID(ID); }

	// ��������� ��������
	void setState(State state) { this->state = state; }

	State getState() { return state; }

private:
	State state; // ������ ��������� ��������

protected:
	string user;

};

class PCB : public Process
{ // Process Control Block 

private:
	int addr;
	int virtualAddr;
	SysReg *sysReg;
	int timeSlice; /* ����� �������, ���������� �������� ��� ������ RR */
	int priority; // ��������� �������� protected:

public:
	PCB()
	{
		sysReg = NULL;
		addr = -1;			/* ����� ���������� � ����������� ������ ������ ��������� */
		virtualAddr = -1;	/* ����� ���������� � ����������� ������ */

							// Scheduling
		timeSlice = -1; 	/* ����� �������, ���������� �������� ��� ������ RR */
		priority = -1; 		/* ��������� �������� ����� ���� ������� �������� DP */
	}

	virtual ~PCB() { cout << " object ��� deleted" << endl; }

	void setSysReg(SysReg *sysReg) { this->sysReg = sysReg; }

	SysReg * getSysReg() { return sysReg; }

	// ����� ���������� � ����������� ������ 
	int getAddr() { return addr; }
	void setAddr(int addr) { this->addr = addr; }

	// ����� ���������� � ����������� ������ 
	int getVirtualAddr() { return virtualAddr; }
	void setVirtualAddr(int virtualAddr) { this->virtualAddr = virtualAddr; }

	// ��������� ��������
	int getPriority() { return priority; }
	void setPriority(int priority) { this->priority = priority; }

	// ����� ������������ ������� ��� �������� 
	int getTimeSlice() { return timeSlice; }
	void setTimeSlice(int timeSlice) { this->timeSlice = timeSlice; }

	// �������� string ������������� ��������� 
	char *NameOf(State state) {
		const int SIZE_OF_STATE_NAME = 15;
		static char szName[SIZE_OF_STATE_NAME];
		switch (state) {
		case NotRunning:
			strcpy_s(szName, SIZE_OF_STATE_NAME, "NotRunning"); break;
		case Running:
			strcpy_s(szName, SIZE_OF_STATE_NAME, "Running"); break;
		case Swapped:
			strcpy_s(szName, SIZE_OF_STATE_NAME, "Swapped"); break;
		case New:
			strcpy_s(szName, SIZE_OF_STATE_NAME, "New"); break;
		case Ready:
			strcpy_s(szName, SIZE_OF_STATE_NAME, "Ready"); break;
		case Blocked:
			strcpy_s(szName, SIZE_OF_STATE_NAME, "Blocked"); break;
		case ExitProcess:
			strcpy_s(szName, SIZE_OF_STATE_NAME, "ExitProcess"); break;
		}

		return szName;
	}
};


class ProcessImage : public PCB
{
private:
	Memory * memory;
	// ��� ��������� ������ VirtualMemory 
	bool status;
	int flag; /* ������ � �������, �������� 1 - ������ ��������, 0 - ����� */

protected:
	friend class Dispatcher;
	friend class Scheduler;

public:
	ProcessImage()
	{
		status = true; /* Processimage � VirtualMemory �������� */ this->memory = NULL;
		flag = 1; // ������ ��������
	}

	virtual ~ProcessImage() { cout << " object Processimage deleted" << endl; }

	void Debug()
	{
		cout << "-- begin Debug ProcessImage deleted" << endl;
		//cout << "user:\t" << user << endl;
		cout << "ID:\t" << getID() << endl;
		cout << "addr:\t" << getAddr() << endl;
		cout << "VirtualAddr:\t" << getVirtualAddr() << endl;
		cout << "State:\t" << NameOf(getState()) << endl;
		cout << "priority:\t" << getPriority() << endl;
		cout << "flag:\t" << flag << endl;
		cout << "SysReg:\t" << endl;

		SysReg * sysReg = getSysReg();
		if (sysReg == NULL)
			cout << "SysReg: NULL" << endl;
		else {
			sysReg->Debug();
		}

		cout << "Code:" << endl;

		if (memory == NULL)
			cout << "Code (memory): NULL" << endl;
		else memory->debugHeap();

		cout << "--- end Debug Processimage --" << endl;
	}

	void DebugTime()
	{
		cout << "--Processimage: DebugTime" << endl;
		cout << "ID\t= " << getID() << endl;
		cout << "Timer tick\t= " << Timer::getTime() << endl;
		cout << "TimeSlice\t= " << getTimeSlice() << endl;
		cout << "Texe\t= " << getTexec() << endl;
		cout << "Priority\t= " << getPriority() << endl;
	}

	Memory* getCode() { return memory; }

	void setCode(Memory* memory) { this->memory = memory; }

	/*������ ������ ��������, ������������ ������ � ������ VirtualMemory ��� ��������� ������: true Processimage �������� */
	void setStatus(bool status) { this->status = status; }
	bool getStatus() { return status; }

	/*����, ��� ������������� ������� � �������� (������������) �������, ���������� �������� ��� ���������� */
	void setFlag(int flag) { this->flag = flag; }
	int getFlag() { return flag; }
	void clearTime() { HANDLE::clearTime(); }
};
#endif
