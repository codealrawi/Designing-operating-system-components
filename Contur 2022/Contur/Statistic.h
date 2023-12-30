#pragma warning (disable:4267)
#ifndef __Statistic_H__
#define __Statistic_H__
#define _USE_MATH_DEFINES

#include "ArchitectureCPU.h"
#include "Process.h"
#include <queue>
//#include <hash_map>
#include <unordered_map>
#include <math.h>

using namespace stdext; // for hash_map
//using namespace System; // for Math::

						// ���������� ���������� �� ���� ���������� 
class Table
{
public:
	Table(int tExec, int tServ)
	{
		this->tExec = tExec;
		this->tServ = tServ;
	}
	int getTime(StatTime time)
	{
		if (time == TimeExec)
			return tExec;
		if (time == TimeServ)
			return tServ;
		return -1;// Error
	}
private:
	int tExec; // ����� ���������� 
	int tServ; // ����� ������������
};

class Statistic
{
public:
	Statistic()
	{}

	void setObservation(string user, int tExec, int tServ)
	{
		hmIter = ovserv.find(user);
		if (hmIter == ovserv.end())
		{
			//If no match is found, end() is returned 
			vector <Table> tb;
			tb.push_back(Table(tExec, tServ));
			ovserv.insert(pairObserv(user, tb));
		}
		else
		{ // tServ ��� ������ HRRN
			hmIter->second.push_back(Table(tExec, tServ));
		}
	}

	// ���������� ������� �� 0 < alfa < 1, 
	double getTpredict(string user, StatTime time)
	{
		hmIter = ovserv.find(user);
		if (hmIter == ovserv.end())
			return -1.0;
		// ����� ����������� 
		unsigned n = hmIter->second.size();
		if (n == 0) return -1;
		/* ���������� ���������� ����������� ��� ���������� ����������, �.�. � 0, �� �-1 */
		double alfa = 0.8;
		double Tpredict = alfa * hmIter->second[n - 1].getTime(time);
		/* �������� � �������������� ���������� �� ������� i > 0 ��� ����� ���������� */
		for (unsigned i = n - 1; i > 0; i--)
		{
			Tpredict = Tpredict + pow((1 - alfa), i) * alfa * hmIter->second[i - 1].getTime(time);
		}
		return Tpredict;
	}

	double getTpredictSimple(string user, StatTime time)
	{
		// ������� ������� �� ������������ hmlter = ovserv.find(user);
		if (hmIter == ovserv.end())
			return -1.0;
		// ����� �����������
		unsigned n = hmIter->second.size();
		double Tpredict = 0;
		// ������������� ����������
		for (unsigned i = 0; i < n; i++)
		{
			Tpredict = Tpredict + hmIter->second[i].getTime(time);
		}
		return Tpredict / n; // ����� �� ����� ����������
	}

	void clearTpredict()
	{
		ovserv.erase(ovserv.begin(), ovserv.end());
		// �������� ����������� ������
	}
	/* ����� ������������ � ���������� �������������� �������� � ������������ � ���������� ���������� � ������ ���������� */
	double getTimeThreshold(StatTime time)
	{
		int size_ = ovserv.size();
		// �����������, ������������ ���������� ������� 
		double beta = 1 / (1 + pow(M_E, -size_));
		hmIter = ovserv.begin();
		double TpredictMin = getTpredict(hmIter->first, time);
		/* 1. ������������� ��� ���������� ��������� �������� �������, ������� �� ������� �������� */
		for (hmIter = hmIter++; hmIter != ovserv.end(); hmIter++)
		{
			// �������� ������� � ���������� �������������� �������� 
			if (TpredictMin > getTpredict(hmIter->first, time))
			{
				TpredictMin = getTpredict(hmIter->first, time);
			}
		}
		return TpredictMin * beta; // ����� ���������
	}

private:
	/* �������� ���������� ��� ���������� ��������������� ������� �� ������ SPN */
	unordered_map <string, vector<Table>> ovserv;
	unordered_map <string, vector<Table>>::iterator hmIter;
	typedef pair <string, vector<Table>> pairObserv;
};
#endif