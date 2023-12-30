#pragma once
#pragma once
#ifndef __Kernel_�__
#define __Kernel_�__

#include "Dispatcher.h" 
#include "Scheduler.h"

class Kernel
{
private:
	Dispatcher * dispatcher;

protected:

public:
	Kernel(Dispatcher *dispatcher)
	{
		this->dispatcher = dispatcher;
	}

	// �������� ��������
	HANDLE * CreateProcess(string user, Memory * code)
	{
		/* ���������� � ����� ���������� VirtualMemory ��� ���������� ������ ��� ����� �������� */
		ProcessImage *processImage = dispatcher->getVirtualMemory();

		if (processImage == NULL)
			return NULL; // ��� ������ 
		
		processImage->setUser(user); 
		dispatcher->initProcessID(processImage,code);

						 // ���������������� ������� 
		return (HANDLE *)processImage;
	} // end CreateProcess

	// �������� ��������
	void TerminateProcess(HANDLE* handle)
	{
		// 1. �������� VirtualMemory � sysreg �������� 
		dispatcher->freeVirtualMemory(handle);
	}

	// ������ ����������� ������
	HANDLE* CreateCriticalSection()
	{
		return dispatcher->getHandleCS();
	}

	void EnterCriticalSection(HANDLE* handle)
	{
		dispatcher->EnterCriticalSection(handle);
	}

	void LeaveCriticalSection(HANDLE* handle)
	{
		dispatcher->LeaveCriticalSection(handle);
	}

	void DebugProcessImage(HANDLE* handle)
	{
		((ProcessImage*)handle)->Debug();
	}
};
#endif