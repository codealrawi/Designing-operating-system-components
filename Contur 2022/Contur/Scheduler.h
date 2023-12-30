#pragma warning (disable:4267)
#ifndef  __Scheduler_H__
#define  __Scheduler_H__

#include "ArchitectureCPU.h" 
#include "Process.h"
#include "Statistic.h"
#include "CPU.h"
#include "MMU.h"
#include <queue>
#include <unordered_map>
#include <valarray>
#include <iostream>

using namespace stdext; // for hash_map 
//using namespace System; // for Math:: 

/* ������� ��� ���������� ����������� CPU �������� � ������������� ������ */

class Job
{
private:
	int addr;// ��������� ������ ���������� ��������� 
	int id; // ������������� ��������
	ProcessImage * processImage; // ����� ��������
	bool done; // ��������� ���������� ������� 

public:
	Job()
	{ // ����������� �� ��������� 
		this->addr = -1;
		this->id = -1;
		this->done = false; // �������� 
		this->processImage = NULL; /* ��������� �� ����� �������� */
	}

	void set(int addr, int id, bool done)
	{
		this->addr = addr;
		this->id = id;
		this->done = done;
	}

	void set(int addr, int id, bool done, ProcessImage * processImage)
	{
		this->addr = addr;
		this->id = id;
		this->done = done;
		this->processImage = processImage;
	}

	void setAddr(int addr) { this->addr = addr; }
	int getAddr() { return addr; }

	void setId(int id) { this->id = id; }
	int getId() { return id; }

	void setDone(bool done) { this->done = done; }
	bool getDone() { return done; }

	void setProcessImage(ProcessImage * processImage)
	{
		this->processImage = processImage;
	}

	ProcessImage * getProcessImage()
	{
		return processImage;
	}

	void Debug()
	{
		cout << " job: addr = " << addr << " id = " << id << " done = " << done << endl;
	}
};

class Scheduler
{
private:
	CPU * cpu;
	int MAX_PROCESS;
	Job* job;
	SysReg *sysreg;
	int sliceCPU;

	/* ������� ��������� � ������������ � �� ����������� The queue class supports a first-in, first-service (FIFS, FIFO) method */
	queue <Process*> processQueue[NUMBER_OF_STATE];
	Statistic * statistic;

	// �������������� ������ 
	/* ������������� ��������� �� ����������� � ������ Multi Processing */
	int quotaProcess;
	int j; // ������������ � ���������� ������� 

		   // ���������� ��������: ����� RR � SRT, HRRN 
	void preemptive(ProcessImage * processImage, MMU * mmu)
	{
		ProcessImage * processImage_w = NULL;
		int size_ = size(Running);
		for (int i = 0; i < size_; i++)
		{
			processImage_w = (ProcessImage *)(front(Running));
			if (processImage_w == processImage)
			{
				pop(Running);
				// 1. ��������� ������� � ������� Swapped 
				push(Swapped, processImage);
				// 2. ���������� ������ swap, ������� ���������� 
				mmu->swapOut(processImage);
				// 3. ��������� ����� ������������ 
				processImage->setTservice(processImage->getTservice() + 1);
			}
			else {
				pop(Running);
				push(Running, processImage_w);
			}
		}
	}

public:
	// ������ ���������� ��� ������������ 
	VectorParam vParam;
	Scheduler(CPU *cpu, int MAX_PROCESS)
	{
		this->cpu = cpu; 	/* ������ ������� ���������� ������� ������ ���������� */
		this->MAX_PROCESS = MAX_PROCESS;
		/* � ������������� ������, ����� ���������� ��� ������� �������� ����������� ��������� ���������, � .� . ������ ������� ���������� ������� ������ ���������� */
		sysreg = new SysReg[MAX_PROCESS];
		sliceCPU = 20; /* slice (�����), ��� ������� ��������� ����������� ��������� �� ����������� 20 ������/����� � ������������� ������ */
		job = NULL; // ������� ������� ����� 
		this->statistic = new Statistic;
		// ����� Multi Processing 
		this->quotaProcess = -1;
		this->j = 0;
	}

	// ����������� ���������������� 
	Scheduler(Scheduler *scheduler)
	{
		this->cpu = scheduler->cpu;
		this->MAX_PROCESS = scheduler->MAX_PROCESS;
		this->sysreg = scheduler->sysreg;
		this->sliceCPU = scheduler->sliceCPU;
		this->job = scheduler->job;
		this->statistic = scheduler->statistic;
	}

	Interrupt execute(Job *job, MMU * mmu)
	{
		Interrupt interrupt; /* OK, Error, Exit = 16, 		Sys, Div_0, Timer, Lan = 32, Dev = 254 */
		ProcessImage * processImage = NULL;

		//���������� PC ��� ��������� 
		for (int i = 0; i < MAX_PROCESS; i++)
		{
			/* -1 ������� �����������, �.�. �������� ��������� ������, ��� MAX_PROCESS */
			if (job[i].getAddr() != -1)
			{
				processImage = job[i].getProcessImage();

				if (processImage == NULL)
				{ /* ������� ��� ProcessImage, lab 1, 2 */
					sysreg[job[i].getId()].setState(PC, job[i].getAddr());
				}
				// ������ ��������� � ������
			}
		}

		while (true)
		{
			// ��������� ������� ������� �� ���� CPU
			Timer::Tick();
			for (int i = 0; i < MAX_PROCESS; i++)
			{
				// -1 ������� ����������� 
				if (job[i].getAddr() == -1) continue;

				// ������� �������� 
				if (job[i].getDone())
				{
					processImage = job[i].getProcessImage();
					// ������� �� c ProcessImage 
					if (processImage == NULL) {
						cout << "Process number " << job[i].getId();
						interrupt = cpu->exeInstr(job[i].getAddr(), &sysreg[job[i].getId()]);
					}
					else if (processImage != NULL)
					{/* ������� c ProcessImage */
					 // -1 �� ��������� 
						if (processImage->getTbegin() < 0)
							processImage->setTbegin(Timer::getTime());

						int AddrPC = processImage->getSysReg()->getState(PC);

						interrupt = cpu->exeInstr(AddrPC, &sysreg[job[i].getId()]);

						// ��������� Texe 
						processImage->setTexec(processImage->getTexec() + 1); // ����� ���������� 

																			  /* ����� RR ������ ���� ����������� �����. �� ��������� -1. �� ��, ���� ���������� ������ ����������� � ������ DP */
						if (processImage->getTimeSlice() > 0)
						{
							// ����� ���������� �������� ������������� ����� 
							if (processImage->getTexec() % processImage->getTimeSlice() == 0)
							{
								// 1. ������ ������� �� ������� 
								job[i].setDone(false);
								// 2. ��������� ������� 
								preemptive(processImage, mmu);
							}
						}

						/* ���������� ���������� �������� �� ������������� ����������, ������� ��������� ������������� �����, ���� ��������� ����������, �� ������ � ������� SRT,HRRN */
						paramPreemptive(processImage, mmu);
					}

					if (interrupt == Exit)
					{ // ������� ��������
						job[i].setDone(false); //������� ��������� 
						processImage = job[i].getProcessImage();

						// ������� c ProcessImage 
						if (processImage != NULL)
						{
							//����� ���������� 
							processImage->setTterminate(Timer::getTime() + 1);

							// �������� ������ 
							mmu->swapOut(processImage);
							/* ������� ������� �� ������� Running � ������ � ������� ExitProcess � ��������� ������ dispatch() �������� ������� ��� �����������, ���������� � �������� � ��� */
							processImage->setState(ExitProcess);
						}
						// +1, ��� ����� ���������� � ������� 
					}

					if ((interrupt == Error) || (interrupt == Empty))
					{
						cout << "interrupt == " << interrupt << endl; 	// ��������� �� �����������; 
						job[i].Debug();
						return interrupt;
					}
				}
			}

			// ��� �������� ��������� 
			if (!isJob(job)) return OK;
		}
		return OK;
	}

	// �������� ���� �� � ������� ������� ��� ���������� 
	bool isJob(Job *job)
	{
		bool isJob = false;
		for (int i = 0; i < MAX_PROCESS; i++)
		{
			// -1 ������� ����������� 
			if (job[i].getAddr() == -1) continue;
			if (job[i].getDone()) isJob = true;
		}
		return isJob;
	}

	Interrupt execute(int addr, int id, CPU *cpu)
	{
		// id ������������� �������� 
		// ���� ���������� ��������� CPU 
		Interrupt interrupt;

		// ������ ��������� � ������ 
		sysreg[id].setState(PC, addr);
		SysReg *sysreg_ = &sysreg[id];

		for (int i = 0; i < sliceCPU; i++)
		{
			interrupt = cpu->exeInstr(addr, sysreg_);

			if ((interrupt == Exit) || (interrupt == Error) || (interrupt == Empty))
				return interrupt;
		} // end while 

		return interrupt;
	}// end execute

	bool scheduleJob(MMU* mmu, bool priority)
	{
		/* ����������� ������� �� ����������, ���� �� ������ ������� */
		DebugSPNQueue();
		// ������������ ������� job ��� ������� Running 
		// ������������� �������������� ������ 
		job = new Job[MAX_PROCESS];
		ProcessImage * processImage = NULL;
		/* ��������� ������� ��� ��������� < MAX_PROCESS

		1. �� ������� Swapped ������ � ������� Running */
		int size_ = size(Swapped);

		for (int i = 0; i < size_; i++)
		{
			processImage = (ProcessImage *)(front(Swapped));
			pop(Swapped);

			// 1.2. ��������� ����� ������������ 
			processImage->setTservice(processImage->getTservice() + 1);
			push(Running, processImage);
		}

		// 2. ��������� 
		if (vParam.getStateTime() != noParam)
		{
			if (vParam.getStateTime() == TimeExecNotPreem)
				// �� ������� ���������� 
				ProcessNext(TimeExec);
			else
				ProcessNext(vParam.getStateTime());
		}

		// ����������� ������� �� ���������� 
		if (priority)
		{// ����� DP 
		 // ��� ���������� �������������� ������� 
			ProcessNext(noParam);
		}

		/* ����������� ������� � ��������� ��������� � ������ maxProcess == true ���������� MAX_PROCESS, ���� ��� �������� maxProcess == false, ��� �������� ����� � ������� */
		size_ = size(Running);

		if (quotaProcess == -1)
		{
			j = 0;

			for (int i = j; i < size_; i++)
			{
				if (j > MAX_PROCESS - 1)
					return true;

				processImage = (ProcessImage *)(front(Running));

				mmu->swapIn(processImage);
				job[j].set(processImage->getAddr(),
					processImage->getID(), true, processImage);
				pop(Running);
				push(Running, processImage);
				j++;
			}
			return false;
		}
		else
		{ // ��� ������������������ ������ 
			int k = j; // ������ ������ ����� 

			for (int i = j; i < size_; i++)
			{
				if (j > MAX_PROCESS - 1)
					return false; // ��� ����� 
				if (j > quotaProcess - 1)
					return true; //����� ���� 

				processImage = (ProcessImage *)(front(Running));
				mmu->swapIn(processImage);
				job[j].set(processImage->getAddr(), processImage->getID(), true, processImage);
				pop(Running);
				push(Running, processImage);
				j++;
			}

			j = 0; // ������ ������ 
			return false; // ����� ������, ��� ������ �����
		}
	} // end schedulejob 

	  // ��������� ������� 
	Interrupt execute(MMU * mmu)
	{
		return execute(job, mmu);
	}

	// ����������� ��� ����������������� ������������ 
	// Multiprocessor scheduling (MP) 
	void setJob(Job *job) { this->job = job; }
	Job * getJob() { return this->job; }

	/*���������� ����� ��� CPU, �.e.������� ������ �������� CPU.����������� ��� ������������� �������������� ������ � �������� ���������� */
	void setSliceCPU(int sliceCPU) { this->sliceCPU = sliceCPU; }

	/* �������� ������������� ��������, ������������� �������� ������������� ���������� ������� � ���� ��������� */
	int getID()
	{
		for (int i = 0; i < MAX_PROCESS; i++)
		{
			if (sysreg[i].getStatus())
				return i;
		}

		return -1; // ��� �������� ������
	} //

	SysReg * getSysReg(int ID)
	{
		return &sysreg[ID];
	}

	// ����������� ��������� ��������� ��� ������ ����� void DebugBlock(int id, CPU *cpu){ cpu->Debug(&sysreg[id]);
	void DebugBlock(int id, CPU *cpu)
	{
		cpu->Debug(&sysreg[id]);
	}

	// ����������� ��������� ��������� 
	void DebugSysReg(int id)
	{
		sysreg[id].Debug();
	}

	void push(State state, ProcessImage * processImage)
	{
		processQueue[state].push(processImage);
	}

	Process* pop(State state)
	{
		if (processQueue[state].empty())
			return NULL;
		Process* process = processQueue[state].front();
		// ������� ������� �� ������� ������ 
		processQueue[state].pop();
		return process;
	}

	void pop(State state, ProcessImage * processImage)
	{
		ProcessImage * processImage_ = NULL;
		for (int i = 0; i < (int)processQueue[state].size(); i++)
		{
			processImage_ = (ProcessImage *)processQueue[state].front();
			// ������� ������� �� ������� ������ 
			processQueue[state].pop();
			if (processImage_ != processImage)
				processQueue[state].push(processImage);
		} /* ���������� �� ����� �������, ����� ��������� �� ������� */
	}

	Process * front(State state)
	{
		return processQueue[state].front();
	}

	bool empty(State state)
	{
		return processQueue[state].empty();
	}

	int size(State state)
	{
		return processQueue[state].size();
	}

	void setObservation(ProcessImage * processImage)
	{
		statistic->setObservation(processImage->getUser(), processImage->getTexec(), processImage->getTservice());
	}

	void clearTpredict()
	{
		statistic->clearTpredict();
	}

	double getTpredict(string user, StatTime time)
	{
		return statistic->getTpredict(user, time);
	}

	/* ����������� ������� �� �������������� �������, � ������������ � ���������� */
	void ProcessNext(StatTime time)
	{
		//1. Running 
		sortQueue(Running, time);
		//2. Blocked 
		sortQueue(Blocked, time);
	}

	void sortQueue(State state, StatTime time)
	{
		int size_ = size(state);
		double wk = 0;
		// ������������ ������� ��� ������ 
		valarray<ProcessImage*> vaProcess(size_);
		/* ��� ���������� ������� ��������� �� ������� */
		valarray<double> va(size_);
		ProcessImage * processImage = NULL;

		for (int i = 0; i < size_; i++)
		{
			processImage = (ProcessImage *)(front(state));
			vaProcess[i] = processImage
				;
			/* ������������� �� ����������� ����������� �������, �� ����, �.�. ����� FCFS */
			if (time == noParam) // ����� DP 
				va[i] = processImage->getPriority();
			else
				va[i] = statistic->getTpredict(vaProcess[i]->getUser(), time);

			pop(state);
			push(state, processImage);
		}

		// ���������� ������� ������� 
		for (int i = 0; i < size_ - 1; i++)
		{
			// �������� �������, ������� �� ������� �������� 
			for (int j = i + 1; j < size_; j++)
			{
				/* 1. ���������� ���������� ������� �� ��������� � ������ */
				if (va[j] < va[i])
				{
					// 2. �� �������� ������� � ������ ��������� swap 
					wk = va[j];
					processImage = vaProcess[j];
					va[j] = va[i];
					vaProcess[j] = vaProcess[i];
					va[i] = wk;
					vaProcess[i] = processImage;
				}
			} // end for 				  // 3. ���������� ������ �������, ���� ������ 
		} // end for 

		for (int i = 0; i < size_; i++)
		{
			pop(state);
			// ������� �� ������� state 
			// ������ ������������� ������� ����� ���������� 
			push(state, vaProcess[i]);
		}
	} // end sortQueue 

	void setTthreshold(StatTime time)
	{
		vParam.setTthreshold(time, statistic->getTimeThreshold(time));
	}

	// ���������� �������� �� ������ SRT � HRRN 
	void methodPreemptive(ProcessImage * ProcessImage, MMU * mmu, StatTime time) 
	{
		double Tpredict = statistic->
			getTpredict(ProcessImage->getUser(), time);
		if (Tpredict == -1)
			return; // ��� ���������� 
		if ((Tpredict - ProcessImage->getTexec()) > vParam.getTthreshold(time))
			return;
		/* 1. ����� ������ ��� �������� �� �������, ����� ������� */
		for (int i = 0; i < MAX_PROCESS; i++) {
			/* -1 ������� �����������, �.�. �������� ��������� ������, ��� MAX_PROCESS */
			if (job[i].getAddr() != -1) {
				if (job[i].getProcessImage()->getID() != ProcessImage->getID()) {
					job[i].setDone(false);
					/* 1.1. ������ ������� �� �������
					1.2. ��������� ��� ��������, �������� ������� ProcessImage */
					preemptive(job[i].getProcessImage(), mmu);
				}
			}
		}
	}

	// ���������� � ������������ � ��������� ����������� 
	void paramPreemptive(ProcessImage *processImage, MMU * mmu) 
	{
		// ��� ���������� SPN 
		if (vParam.getState(TimeExecNotPreem))
			return;
		// ���������� �������� �������� ��������� 
		if (vParam.getStateTime() != noParam)
			methodPreemptive(processImage, mmu, vParam.getStateTime());
	}

	int getProcess() 
	{
		return MAX_PROCESS;
	}

	void setQuotaProcess(int quotaProcess) 
	{
		this->quotaProcess = quotaProcess;
	}

	void DebugQueue(State state) 
	{
		bool empty_ = empty(state);
		if (empty_) {
			cout << " queue is empty " << state << endl;
			return;
		}

		int size = processQueue[state].size();
		if (size == 0)
			return;
		ProcessImage * processImage = (ProcessImage *)processQueue[state].front();
		cout << endl;
		cout << "Queue " << processImage->NameOf(state) << " size: " << size << endl;
		cout << "ID = " << processImage->getID()<<endl;
		cout << "PC = " << processImage->getSysReg()->getState(PC) << endl;
		cout << "ProcessImage" << endl;
		processImage->Debug();
		cout << endl;
	}

	void DebugSPNQueue(State state) 
	{
		if (empty(state)) {
			cout << " is empty " << endl;
			return;
		}
		int size_ = size(state);
		ProcessImage * processImage = NULL;

		for (int i = 0; i < size_; i++) {
			processImage = (ProcessImage *)(front(state));
			cout << endl;
			cout << "user = "<< processImage->getUser() << endl;

			if (vParam.getState(TimeExecNotPreem) || vParam.getState(TimeExec)) {
				cout << "TpredictExec = " << statistic->getTpredict(processImage->getUser(), TimeExec) << endl;
			}

			cout << "Texec = " << processImage->getTexec() << endl;
			if (vParam.getState(TimeServ)) {
				cout << "TpredictServ = " << statistic->getTpredict(processImage->getUser(), TimeServ) << endl;
				cout << "Tserv = " << processImage->getTservice() << endl;
			}

			cout << "Queue state = " << processImage->getState() << endl;
			cout << "Priority = " << processImage->getPriority() << endl;
			cout << "TimeSlice = " << processImage->getTimeSlice() << endl;

			pop(state); push(state, processImage);
		}
	} // end DebugsPNQueue 

	void DebugSPNQueue() 
	{
		cout << "Method : queue order " << endl;
		if (vParam.getState(TimeExec)) // ������ SRT 
			cout << "Tthreshold SRT = " << vParam.getTthreshold(TimeExec) << endl;

		if (vParam.getState(TimeServ)) // ������ SRT 
			cout << "Tthreshold HRRN = " << vParam.getTthreshold(TimeServ) << endl;

		cout << " queue Running " << endl;
		DebugSPNQueue(Running);

		cout << " queue Swapped " << endl;
		DebugSPNQueue(Swapped);

		cout << " queue Blocked " << endl;
		DebugSPNQueue(Blocked);

		cout << " queue ExitProcess " << endl;
		DebugSPNQueue(ExitProcess);
	}
};
#endif