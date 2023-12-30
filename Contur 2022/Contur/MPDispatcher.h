#pragma once
#ifndef __MPDispatcher_H__
#define __MPDispatcher_H__

#include <iostream>
#include "ArchitectureCPU.h" 
#include "VirtualMemory.h" 
#include "Scheduler.h"
#include "Process.h"
#include "Dispatcher.h"
#include <math.h> 

using namespace std;

class MPDispatcher : public Dispatcher
{
public:
	MPDispatcher(int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, Scheduler* scheduler, MMU* mmu, int MAX_PROCESSOR) :
		Dispatcher(SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, scheduler, mmu)
	{
		this->MAX_PROCESSOR = MAX_PROCESSOR;
		vaScheduler = valarray <Scheduler*>(MAX_PROCESSOR);
		vaStatus = valarray <bool>(MAX_PROCESSOR);
		for (int i = 0; i < MAX_PROCESSOR; i++)
		{
			this->vaScheduler[i] = new Scheduler(scheduler);
			this->vaStatus[i] = true; /* ��������� ��������, false - ����� */
		}
	}
	/* ������������ �������, ��������� scheduleprocess ������ Dispatcher */
	void scheduleProcess(MMU* mmu, bool priority)
	{
		cout << " *** scheduleProcess - overriding functions *** " << endl;
		/* 1. ������������ �������� �� �����������; ������� ��������� ����� ����������� �� ����� ���������� */
		int quotaProcess = (int) round((double)scheduler->getProcess() / MAX_PROCESSOR);
		scheduler->setQuotaProcess(quotaProcess);
		bool maxProcess = false;
		// ����������, ���� �� �������� � �������
		for (int i = 0; i < MAX_PROCESSOR; i++)
		{
			// 2. ������������ ������� �� ����� ������� 
			maxProcess = scheduler->scheduleJob(mmu, priority);
			// 3. �������� ������� ���������� ���������� 
			vaScheduler[i]->setJob(scheduler->getJob());
			// 4. ��������, ��� ��������� �����
			vaStatus[i] = false;
			if (!maxProcess) break;
			/* maxProcess == true ���������� MAX_PROCESS, ���� ��� �������� maxProcess == false, ��� �������� ����� � ������� */
		}
		MPDebug();
		mmu->DebugMemory();
	}
	// 3. ��������� ������� �� ����������� 

	Interrupt executeProcess(MMU * mmu)
	{
		cout << " *** executeProcess - overriding functions *** " << endl;
		Interrupt interrupt;
		for (int i = 0; i < MAX_PROCESSOR; i++)
		{
			if (!vaStatus[i])
			{// �����
				interrupt = vaScheduler[i]->execute(mmu);
				vaStatus[i] = true; // ���������� ���������
			}
		}
		return interrupt;
	}

	void MPDebug()
	{
		for (int i = 0; i < MAX_PROCESSOR; i++)
		{
			cout << "---Processor---" << i << " state = " << vaStatus[i] << endl;
		}
	}
private:
	int MAX_PROCESSOR;
	valarray <Scheduler *> vaScheduler;
	valarray <bool> vaStatus;
};
#endif