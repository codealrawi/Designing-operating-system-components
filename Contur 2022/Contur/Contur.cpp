#include "stdafx.h"
#include <string>
#include <iostream>

#include "Dispatcher.h"
#include "CPU.h"
#include "Kernel.h"
#include "Memory.h"
#include "MMU.h"
#include "MPDispatcher.h"
#include "Scheduler.h"
#include "Process.h"
#include "HANDLE.h"

using namespace std;

int main()
{
	int SIZE_OF_MEMORY_IN_BLOCKS = 100; // размер оперативной памяти в блоках
	int MAX_PROCESS = 3; // максимальное количество процессов

	Memory memory(SIZE_OF_MEMORY_IN_BLOCKS);
	MMU mmu(memory);

	CPU* cpu = new CPU(memory, MAX_PROCESS, new LAN, new Device);
	Interrupt interrupt = OK;

	// планирование расписания, выполнение программ для однопроцессорных систем
	Scheduler scheduler(cpu, MAX_PROCESS);
	Job* job = new Job[MAX_PROCESS]; // моделирование многозадачного режима 3
	char ch = 'Y'; /* демонстрация создания образа процесса	при создании объектов
				   класса Code указывается точный размер кода программы в блоках, т. е. количество строк */
	Code* code, *code_1, *code_2 = NULL;
	HANDLE* handle_1, *handle_2; //для двух процессов
	int SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES = MAX_PROCESS; // размер виртуальной памяти в образах процесса
	Dispatcher dispatcher(SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES, &scheduler, &mmu);
	Kernel kernel(&dispatcher);

	while (ch == 'Y' || ch == 'N')
	{
		cout << endl;
		cout << "dialog mode:\n"
			"1. Computer architecture. execute program 1,2.................enter 1\n"
			"2. OS architecture. MultiTasking execute programm 1 and 2.....enter 2\n"
			"3. Model process. Create Process for programm 1 and 2.........enter 3\n"
			"4. Synchronize process. CriticalSection for programm 1 and 2..enter 4\n"
			"5. Main memory and Management Memory Unit.....................enter 5\n"
			"6. Virtual memory and process control block - PCB.............enter 6\n"
			"7. Scheduling for one processor system. Methods...............enter 7\n"
			"8. MultiProcessor scheduling. Methods.........................enter 8" << endl;
		cin >> ch;

		switch (ch) {
		case '1':
			cout << "execute programm 1: Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				/* Programm 1: вычисление арифметического выражения
				(10*2 + 3) / 2 - 3 и вывод результата на устройство (Dev) печати
				Деление и умножение в модели CPU определены только между регистрами.
				Программа в начале работы размещается по нулевому адресу */
				memory.setCmd(Mov, r1, 8); // Перемещение содержимого второго операнда в первый (регистр)
				cout << endl;
				cout << "dump SysReg & first block execution: Y/N-any" << endl;
				cin >> ch;

				if (ch == 'Y') {
					/* присвоить адрес в PC выполняемого блока;
					для программы по умолчанию выделен пул регистров с индесом 0 */
					scheduler.getSysReg(0)->setState(PC, 0);
					scheduler.DebugBlock(0, cpu); /* трассировка состояния регистров и блока, его выполнение */
				}

				memory.setCmd(Mov, r2, 10);
				memory.setCmd(Mul, r1, r2); // регистр * регистр
				memory.setCmd(Add, r1, 3);
				memory.setCmd(Mov, r2, 2);
				memory.setCmd(Div, r1, r2);
				memory.setCmd(Sub, r1, 3);
				memory.setCmd(Wmem, r1, 69); // запись содержимого r1 в память по адресу 69
				memory.setCmd(Rmem, r1, 69); // чтение содержимого r1 из памяти по адресу 69
				memory.setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
				memory.setCmd(Int, r1, Lan); // посыл содержимого r1 по сети 
				memory.setCmd(Int, 0, Exit); // все программы должны заканчиваться прерыванием Exit
				cout << endl;
				cout << " programm 1 in memory: dump memory Y/N-any" << endl;
				cin >> ch;
				if (ch == 'Y')
					memory.debugHeap(); // просмотреть содержимое памяти после загрузки программы
				cout << "dump SysReg & execute programm 1: Y/N-any" << endl;
				cin >> ch;
				if (ch == 'Y') {
					cout << "begin P1 cpu.execute" << endl;
					scheduler.DebugSysReg(0); // трассировка состояния регистров до выполнения
					// выполнение программы, расположенной по адресу 0 идентификатор процесса id = 0 для Р1
					interrupt = scheduler.execute(0, 0, cpu);
					scheduler.DebugSysReg(0); // трассировка состояния регистров после выполнения
					cout << "end P1 cpu.execute interrupt = " << interrupt << endl;
					cout << endl;
				}

				cout << "clear memory & dump: Y/N-any" << endl;
				cin >> ch;
				if (ch == 'Y') {
					memory.clearMemory();
					memory.debugHeap();
					scheduler.getSysReg(0)->clearSysReg(); // очистить регистры
				}
			} // end execute programm 1

			cout << " execute programm 2: Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				/* Programm 2: умножение двух чисел 15*2 
				mmu.setAlloc(O); — запись на место программы в памяти,
				память не очищается, запись производится в свободную память */ 
				mmu.setAlloc(20);
				memory.setCmd(Mov, r1, 11);
				memory.setCmd(Mov, r2, 3);
				memory.setCmd(Mul, r1, r2);	//регистр * регистр
				memory.setCmd(Sub, r1, 3);
				memory.setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
				memory.setCmd(Rmem, r1, 70); // чтение содержимого r1 из памяти по адресу 70
				memory.setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
				memory.setCmd(Int, 0, Exit);
				cout << endl;
				cout << "programm 2 in memory: dump memory Y/-any" << endl;
				cin >> ch;
				if (ch == 'Y')
					memory.debugHeap(); // просмотреть содержимое памяти после загрузки программы
				cout << " SysReg & execute programm 2: Y/-any" << endl;
				cin >> ch;

				if (ch == 'Y') {
					cout << "begin P2 cpu.execute" << endl;
					scheduler.DebugSysReg(1); //трассировка состояния регистров 
					/* выполнение программы, расположенной по адресу 20
					идентификатор процесса id = 1 для Р2 */ 
					interrupt = scheduler.execute(20, 1, cpu);
					scheduler.DebugSysReg(1); // трассировка состояния регистров 
					cout << "end Р2 cpu.execute interrupt = " << interrupt << endl;
					cout << endl;
				}
				cout << "clear memory & execute programm 2 - Error: Y/N-any" << endl;
				cin >> ch;
				if (ch == 'Y') {
					memory.clearMemory();
					cout << " begin P2 cpu.execute " << endl;
					interrupt = scheduler.execute(20, 1, cpu);
					cout << " end P2 cpu.execute interrupt =" << interrupt << endl;
					cout << endl;
					scheduler.getSysReg(1)->clearSysReg(); // очистить регистры
				}
			} // end execute programm 2 
			break;

		case '2':
			cout << endl;
			cout << "clear memory & load P1: addr=10 & P2 addr=40" << endl;
			memory.clearMemory();
			//(12 * 2 - 4) / 2 + 3 = 13
			mmu.setAlloc(10);
			memory.setCmd(Mov, r1, 12);
			memory.setCmd(Mov, r2, 2);
			memory.setCmd(Mul, r1, r2); // 12 * 2 = 24
			memory.setCmd(Sub, r1, 4);  // 24 - 4 = 20
			memory.setCmd(Mov, r2, 2);
			memory.setCmd(Div, r1, r2); // 20 / 2 = 10
			memory.setCmd(Add, r1, 3);  // 10 + 3 = 13
			memory.setCmd(Wmem, r1, 69);
			memory.setCmd(Int, r1, Dev);
			memory.setCmd(Int, r1, Lan);
			memory.setCmd(Int, 0, Exit);
			// 16 * 2 = 32
			mmu.setAlloc(40);
			memory.setCmd(Mov, r1, 16);
			memory.setCmd(Mov, r2, 2);
			memory.setCmd(Mul, r1, r2); // 16 * 2 = 32
			memory.setCmd(Wmem, r1, 70);
			memory.setCmd(Rmem, r1, 70);
			memory.setCmd(Int, r1, Dev);
			memory.setCmd(Int, 0, Exit);

			cout << "dump memory: Y/N-any" << endl;
			cin >> ch;

			if (ch == 'Y')
				memory.debugHeap();

			job[0].set(10, 0, true);
			job[1].set(40, 1, true);
			cout << "Multitasking execute p1, p2: Y/N-any" << endl;
			cin >> ch;

			if (ch == 'Y') {
				cout << "begin P1, p2 cpu.execute" << endl;
				scheduler.setSliceCPU(1);
				interrupt = scheduler.execute(job, NULL);
				cout << "end P1, P2 cpu.execute interrupt = " << interrupt << endl;
			}

			cout << "dump memory: Y/N-any" << endl;
			cin >> ch;

			if (ch == 'Y')
				memory.debugHeap();

			cout << "clear memory: Y/N-any" << endl;
			cin >> ch;

			if (ch == 'Y') {
				memory.clearMemory();
				memory.debugHeap();
				scheduler.getSysReg(1)->clearSysReg();
			}
			break;


		case '3':
			cout << endl;
			cout << "create Process P1 and P2 from code:" << endl;
			/* 1. для каждой программы должен создаваться свой объект 
			2. код в примерах записывается с 0 адреса */
			//(8 * 10 + 2) / 2 – 3 = 38
			code = new Code(11);
			code->setCmd(Mov, r1, 8); // перемещение содержимого второго операнда в первый (регистр)
			code->setCmd(Mov, r2, 10);
			code->setCmd(Mul, r1, r2); // регистр * регистр
			code->setCmd(Add, r1, 3);
			code->setCmd(Mov, r2, 2);
			code->setCmd(Div, r1, r2);
			code->setCmd(Sub, r1, 3);
			code->setCmd(Wmem, r1, 69); // запись содержимого r1 в память по адресу 69
			code->setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
			code->setCmd(Int, r1, Lan); // посыл содержимого r1 по сети 
			code->setCmd(Int, 0, Exit); // все программы должны заканчиваться прерыванием Exit

			cout << "programm 1 in code: dump code Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y')
				code->debugHeap(); // просмотреть содержимое памяти, в которой программа
			handle_1 = kernel.CreateProcess("P1", code); //создать процесс
			// 11 * 3 – 3 = 30
			code = new Code(8);
			code->setCmd(Mov, r1, 11);
			code->setCmd(Mov, r2, 3);
			code->setCmd(Mul, r1, r2); // регистр * регистр
			code->setCmd(Sub, r1, 3);
			code->setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
			code->setCmd(Rmem, r1, 70); // чтение содержимого r1 из памяти по адресу 70
			code->setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
			code->setCmd(Int, 0, Exit);
			handle_2 = kernel.CreateProcess("P2", code); // создать процесс 
			kernel.DebugProcessImage(handle_2); // последний 
			cout << "dump queue NotRunning Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y')
				dispatcher.DebugQueue(NotRunning);
			cout << "dispatcher queue & execute Y/N-any" << endl;
			cin >> ch;
			dispatcher.dispatch(); /* изменяет состояние системы,
								  т. к. ядро не под управлением CPU для модели из трех очередей 2 вызова */
			cout << "dump memory: Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y')
				memory.debugHeap();

			cout << "dump queue Running Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y')
				dispatcher.DebugQueue(Running);

			cout << "dispatch queue & dump queue ExitProcess Y/N-any" << endl;
			cin >> ch;
			dispatcher.dispatch(); // изменяет состояние системы
			if (ch == 'Y')
				dispatcher.DebugQueue(ExitProcess);

			cout << "TerminateProcess & dump ProcessImage Y/N-any" << endl;
			cin >> ch;
			kernel.TerminateProcess(handle_1); //завершить процесс 
			kernel.TerminateProcess(handle_2); // завершить процесс 
			if (ch == 'Y') {
				kernel.DebugProcessImage(handle_1); //проверка; виртуальная 
				kernel.DebugProcessImage(handle_2); // память освобождена
			}
			
			memory.clearMemory();
			break;

		case '4':
			cout << endl;
			cout << "create unsynchronized Process P1 and P2, execute, look Error:" << endl;
			code_1 = new Code(3);
			code_1->setCmd(Mov, r1, 15);
			code_1->setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
			code_1->setCmd(Int, 0, Exit);
			code_2 = new Code(5);
			code_2->setCmd(Rmem, r1, 70); // чтение содержимого r1 из памяти по адресу 70
			code_2->setCmd(Mov, r2, 2);
			code_2->setCmd(Mul, r1, r2); // регистр * регистр 
			code_2->setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
			code_2->setCmd(Int, 0, Exit);
			handle_1 = kernel.CreateProcess("P1", code_1); // создать процесс 
			handle_2 = kernel.CreateProcess("P2",code_2); //создать процесс 
			dispatcher.dispatch(); // изменяет состояние системы 2 вызова
			handle_1->ProcessTime();
			handle_2->ProcessTime();
			/* программы работают асинхронно, поэтому Р1 не успевает 
			осуществить запись содержимого r1 в память по адресу 70, 
			строка 2 программа Р2 читает содержимое r1 из памяти по адресу 70, но память пуста */
			cout << "Dump memory Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				memory.debugHeap();
			}

			kernel.TerminateProcess(handle_1); // завершить процесс 
			kernel.TerminateProcess(handle_2); // завершить процесс 
			memory.clearMemory();
			cout << endl;
			cout << "create synchronized Process P1 and P2" << endl;
			cout << "using CriticalSection Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				handle_1 = kernel.CreateProcess("P1", code_1); // создать процесс 
				handle_2 = kernel.CreateProcess("P2", code_2); // создать процесс
				kernel.EnterCriticalSection(handle_1);
				kernel.EnterCriticalSection(handle_2);
				// выполнение программы Р1
				cout << "----------P1 dispatch----------" << endl;
				dispatcher.dispatch(); // изменяет состояние системы 1-й шаг 
				kernel.DebugProcessImage(handle_1); //состояние процессов 
				kernel.DebugProcessImage(handle_2); //состояние процессов
				kernel.LeaveCriticalSection(handle_1);
				// выпополнение программы Р2
				cout << "----------P2 dispatch----------" << endl;
				dispatcher.dispatch(); // изменяет состояние системы 2-й шаг 
				kernel.DebugProcessImage(handle_2); // проверка: виртуальная память освобождена
				memory.debugHeap();
				kernel.LeaveCriticalSection(handle_2);
				cout << "dispatch" << endl;
				dispatcher.dispatch(); // изменяет состояние системы 3-й шаг
				handle_1->ProcessTime();
				handle_2->ProcessTime();
				kernel.TerminateProcess(handle_1); // завершить процесс 
				kernel.TerminateProcess(handle_2); // завершить процесс
			}
			memory.clearMemory();
			break;

		case '5':
			cout << endl;
			cout << "Management Memory Unit dump Y/N-any" << endl;
			/* пример программы показывает выполнение основных
			функций MMU — Management Memory Unit */ 
			code_1 = new Code(3);
			code_1->setCmd(Mov, r1, 15);
			code_1->setCmd(Wmem, r1, 70); // запись содержимого r1 в память по адресу 70
			code_1->setCmd(Int, 0, Exit);
			cin >> ch;
			if (ch == 'Y') {
				cout << "1.Create process using Management Memory Unit" << endl;
				handle_1 = kernel.CreateProcess("P1", code_1); //создать процесс 
				mmu.Debug(handle_1); // dump MMU — Management Memory Unit 
				cout << "2. Execute process" << endl;
				dispatcher.dispatch(); // изменяет состояние системы 2 вызова 
				dispatcher.dispatch();
				cout << "3. Terminate process" << endl;
				kernel.TerminateProcess(handle_1); //завершить процесс
				// dump Terminate 
				memory.clearMemory();
			}

			cout << "Main memory and MMU Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				cout << "1. Create process, load in main memory, execute" << endl;
				handle_1 = kernel.CreateProcess("P1", code_1); //создать процесс 
				dispatcher.dispatch(); // изменяет состояние системы 2 вызова 
				memory.debugHeap(); // программа удаляется из памяти; данные расположенные по адресу 70
				dispatcher.dispatch();
				cout << "2. Terminate process" << endl;
				kernel.TerminateProcess(handle_1); // завершить процесс 
				cout << "3. Clear all memory" << endl;
				memory.clearMemory();
				memory.debugHeap();
			}
			break;

		case '6':
			cout << endl;
			cout << "Virtual memory Y/N-any" << endl;
			/* пример иллюстрирует выполнение основных функций 
			Virtual memory и process control block — PCB, Process Image */
			code_1 = new Code(3);
			code_1->setCmd(Mov, r1, 15);
			// запись содержимого r1 в память по адресу 70 
			code_1->setCmd(Wmem, r1, 70);
			code_1->setCmd(Int, 0, Exit);
			cin >> ch;
			if (ch == 'Y') {
				// 1. состояние виртуальной памяти в образах 
				cout << "1. Virtual Memory state" << endl;
				dispatcher.DebugVirtualMemory(); // структура виртуальной памяти
				// 2. выделение виртуальной памяти в образах 
				cout << "2. Create process" << endl;
				handle_1 = kernel.CreateProcess("P1", code_1); // создать процесс 
				dispatcher.DebugVirtualMemory();
				// изменяет состояние системы и выполнение программы 
				cout << "3. Process execute" << endl;
				dispatcher.dispatch();
				dispatcher.dispatch();
				cout << "4. Terminate process" << endl;
				kernel.TerminateProcess(handle_1); // завершить процесс
				// 3. освобождение виртуальной памяти в образах 
				dispatcher.DebugVirtualMemory();
				// 4. очистить основную память 
				memory.clearMemory();
			}

			cout << endl;
			cout << "dump PCB and process image Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				cout << "1. Create process" << endl;
				handle_1 = kernel.CreateProcess("P1", code_1); //создать процесс 
				// 1. состояние блока PCB
				dispatcher.DebugPCB(handle_1);
				cout << "2. Process execute" << endl;
				dispatcher.dispatch(); // изменяет состояние системы 2 вызова 
				// 2. состояние блока PCB после выполнения программы.
				// см. состояние PC
				dispatcher.DebugPCB(handle_1);
				dispatcher.dispatch();
				cout << "3. Terminate process" << endl;
				kernel.TerminateProcess(handle_1); // завершить процесс
				// 3. состояние блока PCB после удаления программы из системы 
				dispatcher.DebugPCB(handle_1);
				memory.clearMemory();
			}
			
			break; 
		
		case '7':
			/* 1. для программы "Pl" и "P2" создаются объекты процессы 
			сравнение методов планирования 
			Tenter — Время входа 
			Tbegin — Время начала 
			Tservice — Время обслуживания в очередях 
			Техес — Время выполнения 
			Tterminate — Время завершения 
			Tround = Tservice + Техес — Время оборота 
			Tround/Tservice — Нормализованное время */

			code_1 = new Code(11);
			// перемещение содержимого второго операнда в первый(регистр)
			code_1->setCmd(Mov, r1, 2);
			code_1->setCmd(Mov, r2, 10);
			code_1->setCmd(Mul, r1, r2); // регистр * регистр
			code_1->setCmd(Add, r1, 3);
			code_1->setCmd(Mov, r2, 2);
			code_1->setCmd(Div, r1, r2);
			code_1->setCmd(Sub, r1, 3);
			code_1->setCmd(Wmem, r1, 69); // запись содержимого r1 в память по адресу 69
			code_1->setCmd(Int, r1, Dev); // обращение к устройству для печати содержимого r1
			code_1->setCmd(Int, r1, Lan); // посыл содержимого r1 по сети 
			code_1->setCmd(Int, 0, Exit);
			code_2 = new Code(7);
			code_2->setCmd(Mov, r1, 15);
			code_2->setCmd(Mov, r2, 2);
			code_2->setCmd(Mul, r1, r2);
			code_2->setCmd(Wmem, r1, 70);
			code_2->setCmd(Rmem, r1, 70);
			code_2->setCmd(Int, r1, Dev);
			code_2->setCmd(Int, 0, Exit);
			cout << endl;
			cout << "scheduler FCFS Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				Timer::setTime(); // устанавливаем таймер в нулевое значение 
				handle_1 = kernel.CreateProcess("P1", code_1); //создать процесс 
				handle_2 = kernel.CreateProcess("P2", code_2); // создать процесс 
				dispatcher.dispatch(); // изменяет состояние системы для модели из трех очередей 2 вызова
				dispatcher.dispatch();
				handle_1->ProcessTime();
				handle_2->ProcessTime();
				kernel.TerminateProcess(handle_1); // завершить процесс 
				kernel.TerminateProcess(handle_2); // завершить процесс 
				/* очереди должны быть пусты после эксперимента
				scheduler.DebugSPNQueue(); */
				memory.clearMemory();
			}

			cout << endl;
			cout << "scheduler RR Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				int slice = 5;
				Timer::setTime(); // устанавливаем таймер в нулевое значение 
				// в зависимости от размера программы и квоты, изменяется число строк dispatcher.dispatch();
				dispatcher.setTimeSlice(slice);
				handle_1 = kernel.CreateProcess("P1", code_1); //создать процесс 
				handle_2 = kernel.CreateProcess("P2", code_2); //создать процесс 
				int disp = 4; // подобрано по диаграмме slice и организации	очередей
				for (int i = 0; i < disp; i++) {
					dispatcher.dispatch(); // изменяет состояние системы
				}

				handle_1->ProcessTime();
				handle_2->ProcessTime();
				kernel.TerminateProcess(handle_1); // завершить процесс 
				kernel.TerminateProcess(handle_2); // завершить процесс 
				memory.clearMemory();
				// очереди должны быть пусты после эксперимента scheduler.DebugSPNQueue();
				dispatcher.resetTimeSlice();
			}

			cout << endl;
			cout << "scheduler SPN (shortest process next) Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				int numProcess = 5; // количество объектов одного процесса для расчета его предсказуемого времени выполнения
				Timer::setTime(); // устанавливаем таймер в нулевое значение 
				// включить регистрацию наблюдений 
				dispatcher.setTpredict(TimeExecNotPreem);
				/* для невытесняющего метода
				alfa = 0.8; установлен взвешенный коэффициент 0 < alfa < 1 
				в ShortestProcessNext(), открыт отладчик */
				for (int i = 0; i < numProcess; i++) {
					cout << endl;
					cout << "--------------------experiment i = " << i << endl;
					handle_1 = kernel.CreateProcess("user_1", code_1);
					handle_2 = kernel.CreateProcess("user_2", code_2);
					dispatcher.dispatch(); // изменяет состояние системы 
					dispatcher.dispatch();
					if (i < numProcess - 1) {
						// -1, для сохранения информации о последнем эксперименте
						kernel.TerminateProcess(handle_1); //завершить процесс 
						kernel.TerminateProcess(handle_2); // завершить процесс 
					}

					// наблюдения сохраняются, только после завершения процесса для первого процесса нет наблюдений
					memory.clearMemory();
				}

				handle_1->ProcessTime();
				handle_2->ProcessTime();
				kernel.TerminateProcess(handle_1); //завершить процесс 
				kernel.TerminateProcess(handle_2); //завершить процесс
				//scheduler.DebugSPNQueue(); 
				dispatcher.resetTpredict(); // выключить регистрацию наблюдений
			}

			cout << endl;
			cout << "scheduler SRT (shortest remaining time) Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				// представляет собой вытесняющую версию SPN 
				int numProcess = 3; // количество объектов одного процесса для расчета его предсказуемого времени выполнения
				Timer::setTime(); //устанавливаем таймер в нулевое значение 
				// включить регистрацию наблюдений процессов 
				dispatcher.setTpredict(TimeExec);
				// и вытеснение
				for (int i = 0; i < numProcess; i++) {
					cout << endl;
					cout << "----------------------experiment i = " << i << endl;
					// параметр 0 включает наименьшее оставшееся время процесса
					handle_1 = kernel.CreateProcess("user_1", code_1);
					handle_2 = kernel.CreateProcess("user_2", code_2);
					dispatcher.dispatch(); // изменяет состояние системы 
					dispatcher.dispatch(); // вытеснение user_1 и выполнение 
					dispatcher.dispatch(); // длинный процесс не в swapOut, не успевает
					if (i < numProcess - 1) {
						// -1, для сохранения информации о последнем эксперименте
						kernel.TerminateProcess(handle_1); // завершить процесс 
						kernel.TerminateProcess(handle_2); // завершить процесс
					}
					// наблюдения сохраняются, только после завершения процесса для первого экземпляра процесса нет наблюдений
					memory.clearMemory();
				}
					
				handle_1->ProcessTime();
				handle_2->ProcessTime();
				kernel.TerminateProcess(handle_1); // завершить процесс
				kernel.TerminateProcess(handle_2); // завершить процесс 
				//scheduler. DebugSPNQueue(); 
				dispatcher.resetTpredict(); // выключить регистрацию наблюдений
			}

			cout << endl;
			cout << "scheduler HRRN (hightst response ratio next) Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				// представляет собой вытесняющую версию SPN 
				int numProcess = 3; // количество объектов одного процесса для расчета его предсказуемого времени выполнения
				Timer::setTime(); // устанавливаем таймер в нулевое значение 
				// включить регистрацию наблюдений процессов 
				dispatcher.setTpredict(TimeServ);
				// в ShortestProcessNext(), открыт отладчик 
				for (int i =0; i < numProcess; i++) {
					cout << endl;
					cout << "--------------------experiment i = " << i << endl;
					// параметр 0, включает наименьшее оставшееся время процесса
					handle_1 = kernel.CreateProcess("user_1", code_1);
					handle_2 = kernel.CreateProcess("user_2", code_2);
					dispatcher.dispatch(); // изменяет состояние системы 
					dispatcher.dispatch(); // вытеснение user_1 и выполнение 
					dispatcher.dispatch(); // процесс swapOut
					if (i < numProcess - 1) {
						// -1, для сохранения информации о последнем эксперименте
						kernel.TerminateProcess(handle_1); // завершить процесс 
						kernel.TerminateProcess(handle_2); // завершить процесс
						// наблюдения сохраняются, только после завершения процесса для первого экземпляра процесса нет наблюдений
					}

					memory.clearMemory();
				}

				handle_1->ProcessTime();
				handle_2->ProcessTime();
				kernel.TerminateProcess(handle_1); // завершить процесс 
				kernel.TerminateProcess(handle_2); //
				// scheduler.DebugSPNQueue(); 
				dispatcher.resetTpredict(); // выключить регистрацию наблюдений
			}

			cout << endl;
			cout << "scheduler dynamic priority DP Y/N-any" << endl;
			cin >> ch;
			if (ch == 'Y') {
				int slice = 5;
				// priority 0..3 индекс массива. TimeSlice — значение элемента в массиве
				int* prSlice = new int[3];
				prSlice[0] = 5;
				prSlice[1] = 3;
				prSlice[2] = 2;
				Timer::setTime(); // устанавливаем таймер в нулевое значение
				// в зависимости от размера программы и TimeSlice изменяется число строк dispatcher.dispatch();
				dispatcher.setTimeSlice(3, prSlice);
				handle_1 = kernel.CreateProcess("P1", code_1); // создать процесс 
				dispatcher.dispatch(); // изменяет состояние системы 
				handle_2 = kernel.CreateProcess("P2", code_2); // создать процесс
				int disp = 5; // где
				for (int i = 0; i < disp; i++) {
					dispatcher.dispatch(); // изменяет состояние системы
				}

				handle_1->ProcessTime();
				handle_2->ProcessTime();
				kernel.TerminateProcess(handle_1); // завершить процесс 
				kernel.TerminateProcess(handle_2); // завершить процесс 
				memory.clearMemory();
				//scheduler.DebugSPNQueue(); 
				dispatcher.resetTimeSlice();
			}
			break;

		case '8':
		/* 1. сравнение методов планирования многопроцессорной
		системы для программ "Р1" и "Р2" создаются объекты процессы
		Tenter — Время входа
		Tbegin — Время начала
		Tservice — Время обслуживания в очередях
		Техес — Время выполнения
		Tterminate — Время завершения
		Tround = Tservice + Техес — Время оборота
		Тround/Tservice — Нормализованное время */
		{
			// block begin
			code = new Code(5);
			code->setCmd(Mov, r1, 15);
			code->setCmd(Mov, r2, 2);
			code->setCmd(Mul, r1, r2);
			code->setCmd(Int, r1, Dev);
			code->setCmd(Int, 0, Exit);
			cout << endl;
			cout << "Assignment of Processes to Processors and FCFS Y/N-any" << endl;
			/* Multiprocessor scheduling (MP) — многопроцессорное
			планирование. Метод Global queue — планирование очередей */ 
			int MAX_PROCESSOR = 4; // максимальное количество процессоров
			Scheduler mpScheduler(cpu, MAX_PROCESS);
			MPDispatcher mpDispatcher
				(SIZE_OF_VIRTUAL_MEMORY_IN_IMAGES,
					&mpScheduler, &mmu, MAX_PROCESSOR);
			Kernel mpKernel(&mpDispatcher);
			valarray <HANDLE*> vaHANDLE(MAX_PROCESS);
			cin >> ch;
			if (ch == 'Y') {
				Timer::setTime(); // устанавливаем таймер в нулевое значение
				// 1. Состояние процессоров 
				mpDispatcher.MPDebug();
				for (int i = 0; i < MAX_PROCESS; i++) {
					vaHANDLE[i] = mpKernel.CreateProcess("P", code); // создать процесс
				}

				mpDispatcher.dispatch(); // изменяет состояние системы 
				mpDispatcher.dispatch(); //для модели из трех очередей 2 вызова 
				mpDispatcher.dispatch();
				for (int i = 0; i < MAX_PROCESS; i++) {
					vaHANDLE[i]->ProcessTime(); // создать процесс 
					mpKernel.TerminateProcess(vaHANDLE[i]); // завершить процесс
				}
				
				memory.clearMemory();
				// mpSchcduler.DebugSPNQueue();
			}
		} // block end
		break;

		default:;
		} // end case
	} // end while

return 0;
}
