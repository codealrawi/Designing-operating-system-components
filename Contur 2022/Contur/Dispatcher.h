#pragma once
#ifndef __Dispatcher_H__
#define __Dispatcher_H__

#include <iostream>

#include "ArchitectureCPU.h"
#include "MMU.h"
#include "Process.h"
#include "Scheduler.h"
#include "VirtualMemory.h"

using namespace std;

/* ��������� ���������, �������� ��������� ��������� */
class Dispatcher
{
public:
	Dispatcher(int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, Scheduler *scheduler, MMU *mmu)
	{
		virtualMemory = new VirtualMemory(SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES);
		this->scheduler = scheduler;
		this->mmu = mmu; // Model memory unit
		this->cs = new CS; // critical section
		this->timeSlice = -1; /* ����� ������� ��� ������������ �� ������ RR */
	}

	ProcessImage * getVirtualMemory()
	{
		/* ��������� �������� �� ���������, ��������� �� ������� */

		int addr = virtualMemory->getAddrFreeImage();

		if (addr == -1)
		{
			return NULL;
		}

		// 1. �������� VirtualMemory
		virtualMemory->setAddrImage(addr);

		// 2. �������� ����� �� ������������ ������
		ProcessImage * processImage = virtualMemory->getImage();

		// 3. ��������� ����������� ����� � ������
		processImage->setVirtualAddr(addr);

		// 4. ��������� ����� ������������ �� �������������� ������ 
		processImage->setTservice(processImage->getTservice() + 1);

		return processImage;
	}

	void freeVirtualMemory(HANDLE *handle)
	{
		ProcessImage * processImage = (ProcessImage *)handle;
		/* 0. ������ ���� ������� � ������� ExitProcess (����������) */

		State state = processImage->getState();
		int ID = processImage->getID();

		// 1. �������� ��������
		scheduler->getSysReg(ID)->setStatus(true);
		scheduler->getSysReg(ID)->clearSysReg();
		processImage->setSysReg(NULL);

		// 2. ������� �� �������
		scheduler->pop(state, processImage);

		// 3. ��������� ���������� ��� ������ SPN;
		if (scheduler->vParam.getStateTime() != noParam)
		{
			scheduler->setObservation(processImage);

			if (scheduler->vParam.getState(TimeExecNotPreem))
				scheduler->setTthreshold(TimeExec); // ���������� SPN

													/* 4. ���������� ����� ����������� ���������� ������� */
			if (scheduler->vParam.getState(TimeExec))
				scheduler->setTthreshold(TimeExec); // ���������� SRT

			if (scheduler->vParam.getState(TimeServ))
				scheduler->setTthreshold(TimeServ); // ������������ HRRN
		}

		/* 4. ���������� VirtualMemory, ���������������� PCB */
		virtualMemory->clearImage(((PCB*)handle)->getVirtualAddr());
	}

	void initProcessID(ProcessImage *processImage, Memory *code)
	{
		// 1. ���������� ID
		int ID = scheduler->getID();

		if (ID >= 0)
		{
			//ID �� -1, �.�. ������ ��������� 
			processImage->setID(ID);
		}
		else
		{
			// � ������� ��� ����� 
			cout << "init Process ID > MAX_PROCESS in system" << endl;
		}

		// 2. �������� �������� no ID 
		scheduler->getSysReg(ID)->setStatus(false);
		processImage->setSysReg(scheduler->getSysReg(ID));
		// 3. ��������� ��������
		processImage->setState(NotRunning);
		// 4. ���������� ��������� 
		processImage->setCode(code);
		// � ��������� ��� ������� �� ���
		code = NULL;
		// 5. ���������� ����� ����� 
		processImage->setTenter(Timer::getTime());
		// 6. ���������������� ����� ������������
		processImage->setTservice(0);
		// 7. � ������� ��� NotRunning
		scheduler->push(NotRunning, processImage);
	}

	/* ������ ������� ��������� � ������������ � �������� �������� */
	virtual void dispatch()
	{
		ProcessImage * processImage = NULL;
		Timer::Tick();

		/* 1. ����������� ��� ������� Running, ����
		���� ������������� ��������, ������ ������ ��
		������� � ����������� ������; ������ ������
		��������� ������ ������� ���� */

		int size_ = scheduler->size(Running);

		for (int i = 0; i < size_; i++)
		{
			processImage = (ProcessImage *)(scheduler->front(Running));
			//��������� ����� ������������.
			processImage->setTservice(processImage->getTservice() + 1);

			if (processImage->getState() == ExitProcess) //������� ��������
			{
				scheduler->pop(Running); //������� �� �������
				scheduler->push(ExitProcess, processImage); //������ � ������� �����������
			}
		}

		//3. ����������� ������� Blocked (� ����� ������ ����� ���������)
		size_ = scheduler->size(Blocked);

		for (int i = 0; i < size_; i++)
		{
			processImage = (ProcessImage *)(scheduler->front(Blocked));

			//��������� ����� ������������.
			processImage->setTservice(processImage->getTservice() + 1);

			if (!cs->getCS())// ����������� ������ ��������
			{
				scheduler->pop(Blocked);// ������� �� �������
				processImage->setState(Running); /*�������� ��������� ��������*/
				processImage->setFlag(1);
				// �� �������� ��������� ��������
				scheduler->push(ExitProcess, processImage); //������ � ������� �����������
				cs->setCS(true);//������� ������
				break;
			}
		}

		/*2. ����������� ������� NotRunning �, ���� ��� ������ ���������� ��� ��������
		(�������� �. �. �.) ��������� �� ����������*/
		size_ = scheduler->size(NotRunning);

		for (int i = 0; i < size_; i++)
		{
			processImage = (ProcessImage *)(scheduler->front(NotRunning));

			//��������� ����� ������������.
			processImage->setTservice(processImage->getTservice() + 1);

			/*�������� �� ��� ����� �������� ������� � ���������*/
			if (processImage->getSysReg() == NULL)
			{
				/*�������� ��������, ���� ��� ��������� �������� � �������*/
				SysReg *sysReg = scheduler->getSysReg(scheduler->getID());

				if (sysReg == NULL)
				{
					scheduler->pop(NotRunning);// ������� �� �������
					scheduler->push(NotRunning, processImage); //������ � ��� �� �������
					continue;
				}

				processImage->setSysReg(sysReg);
			}

			scheduler->pop(NotRunning);// ������� �� �������

									   /*2.1. Code ��������� � ������, ��������� ������� ���������� � ����� ���������� �������,
									   ���� ���� ������*/
			if (processImage->getFlag() == 1)
			{
				processImage->setState(Running);
				/* �������� ��������� ��������; ���������� ����� ������� � ���������,
				� ����������� �� ������ */
				processImage->setTimeSlice(this->timeSlice);

				// ����� RR
				if (prSlice.getPrioritySlice())
				{
					// ����� DP
					int pr = processImage->getPriority();
					processImage->setTimeSlice(prSlice.getTimeSlice(pr));
					processImage->setPriority(prSlice.getPriority(pr));
				}

				scheduler->push(Running, processImage);
				// ������ � ������� �����������
			}
			else
			{
				processImage->setState(Blocked);
				/* �������� ��������� �������� */
				scheduler->push(Blocked, processImage);
				// ������ � ������� �������������
			}
		}

		/* 3. ����������� ������� Swapped � ��������� ����� DP
		����������� ������ ��� ������ DP */
		bool priority = prSlice.getPrioritySlice();

		if (priority)
		{ // ����� DP
			size_ = scheduler->size(Swapped);

			for (int i = 0; i < size_; i++)
			{
				processImage = (ProcessImage *)(scheduler->front(Swapped));
				// 3.1. �������� ��������� �������� � TimeSlice 
				int pr = processImage->getPriority();
				processImage->setTimeSlice(prSlice.getTimeSlice(pr));
				processImage->setPriority(prSlice.getPriority(pr));
				scheduler->pop(Swapped); /* ������� �� ������� */
				scheduler->push(Swapped, processImage); // ������ � ��� �� �������
			}
		}

		// ����������� ������
		if (scheduler->empty(Running) && scheduler->empty(Swapped))
		{
			cout << "dispatch: empty Running && Swapped" << endl;
			return;//��� ������� �� ����������
		}

		scheduleProcess(mmu, priority);

		/* ��������� �������� � ������������ �� �������������� �������� */
		Interrupt interrupt = executeProcess(mmu);
	}

	/* ��������� �������� � ������������ �� �������������� �������� */
	virtual void scheduleProcess(MMU * mmu, bool priority)
	{
		bool maxProcess = scheduler->scheduleJob(mmu, priority);
		/*������� ������������ maxProcess == true ���������� MAX_PROCES,
		���� ��� ��������
		maxProcess == false, ��� �������� ����� � ������� */
	}

	/* ��������� �������� � ������������ �� �������������� �������� */
	virtual Interrupt executeProcess(MMU * mmu)
	{
		return scheduler->execute(mmu);
	}

	// ��� ���������� ����������� �������
	HANDLE * getHandleCS()
	{
		return (HANDLE *)cs;
	}

	void EnterCriticalSection(HANDLE* handle)
	{
		if (!cs->getCS())
		{ // ������ �� �����, ��� ����� �������� ����������� ������ �������
			((ProcessImage *)handle)->setFlag(1);
			cs->setCS(true); /* ������� ����������� ������ ��� ������ */
		}
		else {
			// ������� ����������� ������ ��� ����� �������� 
			((ProcessImage *)handle)->setFlag(0);
		}
	}

	void LeaveCriticalSection(HANDLE* handle)
	{
		cs->setCS(false); /* ������� ����������� ������, �� �����, false */
	}

	// ��� ������������ �� ������ RR 
	void setTimeSlice(int timeSlice)
	{
		this->timeSlice = timeSlice;
	}

	void resetTimeSlice()
	{
		this->timeSlice = -1;
		this->prSlice.setPrioritySlice(-1, NULL);
	}

	/* ��� ������������ �� ������ DP - ������������ ����������� */
	void setTimeSlice(int size, int * prSlice)
	{
		this->prSlice.setPrioritySlice(size, prSlice);
	}

	// ��� ������ SRT � HRRN
	void setTpredict(StatTime time)
	{
		scheduler->vParam.setState(time);
	}

	void resetTpredict()
	{
		scheduler->vParam.clearVectorParam();
	}

	double getTpredict(string user, StatTime time)
	{
		return scheduler->getTpredict(user, time);
	}

	void DebugQueue(State state)
	{
		scheduler->DebugQueue(state);
	}

	void DebugVirtualMemory()
	{
		this->virtualMemory->Debug();
	}

	void DebugPCB(HANDLE* handle)
	{
		handle->ProcessTime();
		((ProcessImage *)handle)->Debug();
	}

	class Priorityslice
	{
	public:
		Priorityslice()
		{
			this->size = -1;
			this->prSlice = NULL;
		}

		void setPrioritySlice(int size, int * prSlice)
		{
			this->size = size;
			this->prSlice = prSlice;
		}

		bool getPrioritySlice()
		{
			if (prSlice == NULL)
				return false;
			return true;
		}

		int getTimeSlice(int priority)
		{
			if (size == -1)
				return -1; /* timeSlice �� ���������� */

			if (priority == -1)
				return prSlice[0]; // ��������� ��������� ��������� 

			if (priority == size - 1)
				return prSlice[size - 1];

			return prSlice[priority + 1];
		}

		int getPriority(int priority)
		{
			if (priority == -1)
				return 0;

			if (priority == size - 1)
				return size - 1;

			return priority + 1;
		}

	private:
		int size;
		int *prSlice; /* ������  ����������� � ������� ������� ��� ������������ �� ������ DP */
	};

protected:
	VirtualMemory * virtualMemory;
	Scheduler* scheduler;
	MMU* mmu;
	CS* cs;	// critical section
	int timeSlice; /* ����� ������� ��� ������������ �� ������ RR */
	Priorityslice prSlice; /* ��� ������������ �� ������ DP */
};
#endif