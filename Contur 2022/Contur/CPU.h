#ifndef __CPU_H__
#define __CPU_H__

#include "ArchitectureCPU.h"
#include "Memory.h"
#include "Device.h"
#include "LAN.h"

class CPU 
{
	public:
		CPU(Memory &memory, int MAX_PROCESS, LAN *lan, Device *dev) 
		{
			this->memory = &memory;
			this->lan = lan;
			this->dev = dev;
		}
		~CPU() {}

	// ��������� ���������� - ����
	Interrupt exeInstr(int addr, SysReg *sysreg) 
	{
		Interrupt interrupt;
		Block* block = fetch(sysreg);
		
		// ������ ���� �� ������������
		if (block->getState()) 
			return Error;
		
		// ����������
		interrupt = decode(block, sysreg);
		
		switch (interrupt) 
		{
			case OK:  // �������� PC 
				sysreg->setState(PC, memory->getNextAddr(sysreg->getState(PC)));
				break;
			case Exit: cout << "Exit" << endl;
				return Exit;
			case Dev: // ��������� � ���������� ������ 
				dev->printData(sysreg->getState((Name)block->getNregister()));
				// �������� PC 
				sysreg->setState(PC, memory->getNextAddr(sysreg->getState(PC)));
				break;
			case Lan: // ��������� � ���������� ���� 
				lan->sendData(sysreg->getState((Name)block->getNregister())); // �������� PC 
				sysreg->setState(PC, memory->getNextAddr(sysreg->getState(PC)));
				break;
			default: 
				return interrupt;
		}

		return OK;
	} // end exelnstr

	void Debug(SysReg *sysreg) 
	{ 
		/* ����������� ��������� ��������� ��� ������ ����� ��. Scheduler Debug */
		sysreg->Debug();
		// ������ ���� ����
		Block* block = fetch(sysreg);
		block->debugBlock();
		decode(block, sysreg);
		sysreg->Debug();
	}

	private:
		Memory * memory;
		int MAX_PROCESS;
		LAN *lan;
		Device *dev;

		// ������� ��������� CPU
		// ������� ���������� �� ��������� PC
		Block* fetch(SysReg *sysreg) 
		{
			return memory->read(sysreg->getState(PC));
		}

	protected:
	virtual Interrupt decode(Block* block, SysReg *sysreg) {
		/* ������ � ������� �������� access: sysreg[id]->getState(....)	*/
		Block* mblock = NULL; /* ������������ ��� ������ � ������ */
		cout << " opearand " << block->getOperand() << endl;
		switch (block->getCode()) {
		case Mov: /* ����������� ����������� ������� �������� � ������ {�������} */
			sysreg->setState(block->getNregister(), block->getOperand());
			break;
		case Add:
			/* 1. ����� ���������� �������� � �������� (���� �������) 2. ��������� �������� � ������ ������� */
			sysreg->setState((Name)block->getNregister(), sysreg->getState((Name)block->getNregister()) + block->getOperand());
			break;
		case Sub:
			/* 1. ����� ���������� �������� � �������� (���� �������) 2. ��������� �������� � ������ ������� */
			sysreg->setState((Name)block->getNregister(), sysreg->getState((Name)block->getNregister()) - block->getOperand());
			break;
		case Mul:
			/* 1. ����� ���������� �������� � �������� (���� �������) 2. ��������� �������� � ������ ������� */
			sysreg->setState((Name)block->getNregister(), sysreg->getState((Name)block->getNregister())* sysreg->getState((Name)block->getOperand()));
			break;
		case Div:
			if (sysreg->getState((Name)block->getOperand()) == 0) return Div_0;
			/* 1. ����� ���������� �������� � �������� (���� �������) 2. ��������� �������� � ������ ������� */
			sysreg->setState((Name)block->getNregister(), sysreg->getState((Name)block->getNregister()) / sysreg->getState((Name)block->getOperand()));
			break;
		case Int: // ����������
			return (Interrupt)block->getOperand();
			break;
		case Wmem:
			// ��������� ���������� �������� � ������ 
			mblock = memory->read(block->getOperand());
			if (!mblock->getState()) return Error;
			// ������ �������� ����� 
			mblock->setOperand(sysreg->getState((Name)block->getNregister()));
			mblock->setState(false); // ����� 
			break;
		case Rmem:
			// ������ �������� �� ����� ������ � ������� 
			mblock = memory->read(block->getOperand());
			if (mblock->getState()) return Empty;
			// ������ ������ �������� ����� 
			sysreg->setState((Name)block->getNregister(), mblock->getOperand());
			break;
		default: return Error;
		}
		return OK;
	} // end decode
};
#endif