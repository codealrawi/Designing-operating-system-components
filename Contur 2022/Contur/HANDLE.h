#ifndef __HANDLE_H__
#define __HANDLE_H__

class HANDLE {
public:
	HANDLE() {
		ID = -1; /* ������������� �������������
				 ��������, -1 - ������������� �� �������� */
				 // process scheduling
		Tenter = -1; // ����� �����
		Tbegin = -1; // ����� ������
		Tservice = 0; // ����� ������������ � ��������
		����� = 0; // ����� ����������
		Tterminate = 0; // ����� ����������
	}
	virtual ~HANDLE()
	{
		cout << " object HANDLE deleted" << endl;
	}

	int getTenter() { return Tenter; }
	int getTbegin() { return Tbegin; }
	int	getTservice() { return Tservice; }
	int	getTterminate() { return Tterminate; }
	int	getTexec() { return �����; }
	int	getTround() { return ����� + Tservice; } // ����� �������

	float getTnorm() { return (float)(����� + Tservice) / Tservice; }

	void ProcessTime() {
		cout << "        ProcessTime ID =  " << ID << endl;
		cout << "Tenter:	" << Tenter << endl; // ����� �����
		cout << "Tbegin:	" << Tbegin << endl; // ����� ������
		cout << "Tservice:	" << Tservice << endl; // ����� ������������ � ��������
		cout << "Tterminate:	" << Tterminate << endl; // ����� ����������
		cout << "Texec:	   " << ����� << endl; // ����� ����������
		cout << "Tround:	" << getTround() << endl; //����� �������
		cout << "Tnorm:	" << getTnorm() << endl; // ��������������� �����
	}

private:
	int ID;
	// process scheduling
	int Tenter;	// ����� �����
	int Tbegin;	// ����� ������
	int Tservice;	// ����� ������������ � ��������
	int Tterminate;	// ����� ����������
	int �����;	// ����� ����������
protected:
	int getID() { return ID; }
	void setID(int ID) { this->ID = ID; } // process scheduling
	void setTenter(int Tenter) { this->Tenter = Tenter; }
	void setTbegin(int Tbegin) { this->Tbegin = Tbegin; }
	void setTservice(int Tservice) { this->Tservice = Tservice; }
	void setTterminate(int Tterminate) { this->Tterminate = Tterminate; }
	void setTexec(int �����) { this->����� = �����; }
	void clearTime() { // ����������� ��� ����������
		Tenter = -1;// ����� �����
		Tbegin = -1; // ����� ������
		Tservice = 0; // ����� ������������
		����� = 0; // ����� ����������
		Tterminate = 0; // ����� ����������
	}
};

// ��� CriticalSection - ����������� ������ 
class CS : HANDLE {
public:
	CS() {
		cs = false;
		// false - ����������� ������ �� ������
		// true - ����������� ������ ������
	}
	virtual ~CS() { cout << " object CS deleted" << endl; }
	bool getCS() { return cs; }
	void setCS(bool cs) { this->cs = cs; }
private:
	bool cs;
protected:
};
#endif

