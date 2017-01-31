#include <iostream>
#include <ratio>
#include <chrono>
#include <algorithm>
#include "SkipList.h"
#include <set>
#include <vector>
#include <list>
#include <iomanip>
#include <string>

#include <fstream>
#include <cstdlib>

using namespace std;
using namespace chrono;

void Test1(int NumberOfData, double Probability);
void Functional_Test();

void AddTime(void);
double AddData(SkipList<int> *list, int *Data, int numberOfData);

void DictionaryTest(string Path);
void IntTest(int NumberOfData);
int GetFileSize(ifstream &ifs);
int NumberOfLines(ifstream &ifs);
vector<string> ReadAllLines(ifstream &ifs, int &LinesNo);

template<typename T>
double TestAdd(set<T> &SetData, vector<T> &Lines);
template<typename T>
double TestAdd(SkipList<T> &SkipListData, vector<T> &Lines);
template<typename T>
double TestFind(set<T> &SetData, vector<T> &Lines, vector<int>& Indexes);
template<typename T>
double TestFind(SkipList<T> &SkipListData, vector<T> &Lines, vector<int>& Indexes);
template<typename T>
double TestRemove(set<T> &SetData, vector<T> &Lines, vector<int>& Indexes);
template<typename T>
double TestRemove(SkipList<T> &SkipListData, vector<T> &Lines, vector<int>& Indexes);

fstream report_file;

int main()
{
	report_file = fstream("D:\\Dropbox\\Visual Studio\\SkipList\\report.csv", ios::out);

	DictionaryTest("D:\\Dropbox\\Studia\\Informatyka\\ZBP\\polish.dic");
	DictionaryTest("D:\\Dropbox\\Studia\\Informatyka\\ZBP\\english.dic");
	IntTest(1000000);
	//Functional_Test();
	//Test1(1000000, 0.7);

	//AddTime();

	report_file.close();

	system("shutdown.exe -s -t 120");

	return 0;
}

void Test1(int NumberOfData, double Probability)
{
	high_resolution_clock::time_point Timestamp[2];

	int *Data = new int[NumberOfData];
	for(int i = 0; i < NumberOfData; i++)
		Data[i] = i;
	random_shuffle(Data, Data + NumberOfData);

	set<int> setObj;
	vector<int> vectorObj;
	list<int> listObj;
	SkipList<int> skipList(32 , Probability);

	Timestamp[0] = high_resolution_clock::now();
	for(int i = 0; i < NumberOfData; i++)
		setObj.insert(Data[i]);
	Timestamp[1] = high_resolution_clock::now();
	cout << left << setw(15) << "set" << duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]).count() << endl;

	Timestamp[0] = high_resolution_clock::now();
	for(int i = 0; i < NumberOfData; i++)
		vectorObj.push_back(Data[i]);
	sort(vectorObj.begin(), vectorObj.end());
	Timestamp[1] = high_resolution_clock::now();
	cout << left << setw(15) << "vector" << duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]).count() << endl;

	Timestamp[0] = high_resolution_clock::now();
	for(int i = 0; i < NumberOfData; i++)
		listObj.push_back(Data[i]);
	listObj.sort();
	Timestamp[1] = high_resolution_clock::now();
	cout << left << setw(15) << "list" << duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]).count() << endl;

	Timestamp[0] = high_resolution_clock::now();
	for(int i = 0; i < NumberOfData; i++)
		skipList.insert(Data[i]);
	Timestamp[1] = high_resolution_clock::now();
	cout << left << setw(15) << "skip list" << duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]).count() << endl;

	delete[] Data;
}
void AddTime(void)
{
	int numberOfData = 1000000;
	int step = 10;

	int *Data = new int[numberOfData];
	for(int i = 0; i < numberOfData; i++)
		Data[i] = i;
	random_shuffle(Data, Data + numberOfData);

	double p;
	for(int k = 1; k < 100; k++)
	{
		p = k * 0.01;
		report_file << p << ";";
		cout << p << " ";
		SkipList<int> list(32 , p);
		double time =  AddData(&list, Data, numberOfData);
		report_file << time << ";";
		int64_t MemoryConsumption = list.GetMemoryConsumption();
		if(MemoryConsumption < 1024)
			report_file << MemoryConsumption << " B" << endl;
		else if(MemoryConsumption < 1048576)
			report_file << MemoryConsumption / 1024.0 << " kB" << endl;
		else if(MemoryConsumption < 1073741824)
			report_file << MemoryConsumption / 1048576.0 << " MB" << endl;

		cout << time << endl;
		list.printStats();
	}

	delete[] Data;
}
double AddData(SkipList<int> *list, int *Data, int numberOfData)
{
	high_resolution_clock::time_point Timestamp[2];
	list->clear();

	Timestamp[0] = high_resolution_clock::now();
	for(int i = 0; i < numberOfData; i++)
		list->insert(Data[i]);
	Timestamp[1] = high_resolution_clock::now();

	double duartionTime = duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]).count();
	return duartionTime;
}

void Functional_Test()
{
	int NumberOfData = 20;
	int *Data = new int[NumberOfData];
	for(int i = 0; i < NumberOfData; i++)
		Data[i] = i;
	random_shuffle(Data, Data + NumberOfData);

	vector<int> vec(Data, Data + NumberOfData);
	delete[] Data;

	SkipList<int> *list1, *list2;
	pair<SkipList<int>::Iterator, bool> p;
	SkipList<int>::Iterator itr;

	cout << "empty constructor" << endl;
	list1 = new SkipList<int>();
	cout << "empty " << list1->empty() << endl;
	cout << "size " << list1->size() << endl;
	list1->printAll();
	delete list1;
	cout << endl;

	cout << "iterator constructor" << endl;
	list1 = new SkipList<int>(vec.begin(), vec.end() - 10);
	cout << "empty " << list1->empty() << endl;
	cout << "size " << list1->size() << endl;
	list1->printAll();
	cout << endl;

	cout << "iterator constructor" << endl;
	list2 = new SkipList<int>(vec.begin() + 10, vec.end());
	cout << "empty " << list2->empty() << endl;
	cout << "size " << list2->size() << endl;
	list2->printAll();
	cout << endl;

	cout << "swap" << endl;
	list1->swap(*list2);
	list1->printAll();
	list2->printAll();
	delete list2;
	cout << endl;

	cout << "copy constructor" << endl;
	list2 = new SkipList<int>(*list1);
	list2->printAll();
	delete list1;
	delete list2;
	cout << endl;

	cout << "iterator constructor" << endl;
	list1 = new SkipList<int>(vec.begin(), vec.end());
	list1->printAll();
	cout << endl;

	cout << "add 30, 10, 45" << endl;
	p = list1->insert(30);
	cout << p.second;
	(p.second ? cout << " " << *p.first << endl : cout << endl);
	p = list1->insert(10);
	cout << p.second;
	(p.second ? cout << " " << *p.first << endl : cout << endl);
	p = list1->insert(45);
	cout << p.second;
	(p.second ? cout << " " << *p.first << endl : cout << endl);
	list1->printAll();
	cout << endl;

	cout << "erase 15, 11, 6" << endl;
	list1->erase(15);
	list1->erase(11);
	list1->erase(6);
	list1->printAll();
	cout << endl;

	cout << "find 10, 9, 6, 11, 30" << endl;
	itr = list1->find(10);
	cout << (itr != list1->end());
	(itr != list1->end() ? cout << " " << *itr << endl : cout << endl);
	itr = list1->find(9);
	cout << (itr != list1->end());
	(itr != list1->end() ? cout << " " << *itr << endl : cout << endl);
	itr = list1->find(6);
	cout << (itr != list1->end());
	(itr != list1->end() ? cout << " " << *itr << endl : cout << endl);
	itr = list1->find(11);
	cout << (itr != list1->end());
	(itr != list1->end() ? cout << " " << *itr << endl : cout << endl);
	itr = list1->find(30);
	cout << (itr != list1->end());
	(itr != list1->end() ? cout << " " << *itr << endl : cout << endl);
	cout << endl;
	
	itr = list1->begin();
	for(int i = 0; i < (int) list1->size() - 1; i++)
	{
		cout << i << " " << *itr << endl;
		cout << i + 1 << " " << *++itr << endl;
		cout << i << " " << *--itr << endl;
		itr++;
		cout << endl;
	}
}

void DictionaryTest(string Path)
{
	vector<string> Lines;
	ifstream ifs;

	ifs.open(Path, ios::in);
	int FileSize = GetFileSize(ifs);
	int LinesNo;

	Lines = ReadAllLines(ifs, LinesNo);

	ifs.close();

	vector<string> ShuffledLines(Lines);
	random_shuffle(ShuffledLines.begin(), ShuffledLines.end());

	int CollectionSize = ShuffledLines.size();
	int ItemsToFind = 50000;
	mt19937 rng;
	vector<int> Indexes;
	for(int i = 0; i < ItemsToFind; i++)
		Indexes.push_back(rng() % CollectionSize);

	report_file << "plik;" << Path << endl;

	set<string> SetCollection;
	double AddTime, FindTime, RemoveTime;

	report_file << "set" << endl;

	AddTime = TestAdd(SetCollection, ShuffledLines);
	FindTime = TestFind(SetCollection, Lines, Indexes);
	RemoveTime = TestRemove(SetCollection, Lines, Indexes);

	report_file << AddTime << ";" << FindTime << ";" << RemoveTime << endl;
	cout << AddTime << "\t" << FindTime << "\t" << RemoveTime << endl;

	report_file << "SkipList" << endl;

	bool Dynamic = true;
	int MaxP = 0;
	for(int i = 0; i < 2; i++)
	{
		Dynamic = !Dynamic;
		MaxP = Dynamic ? 96 : 91;
		
		for(int k = 1; k < 91; k++)
		{
			double p = 0.01 * k;
			SkipList<string> SkipListCollection(32, p, Dynamic);
			report_file << p << ";";
			cout << p << "\t";

			AddTime = TestAdd(SkipListCollection, ShuffledLines);
			FindTime = TestFind(SkipListCollection, Lines, Indexes);
			RemoveTime = TestRemove(SkipListCollection, Lines, Indexes);

			report_file << AddTime << ";" << FindTime << ";" << RemoveTime << endl;
			cout << AddTime << "\t" << FindTime << "\t" << RemoveTime << endl;
		}
	}

}
void IntTest(int NumberOfData)
{
	vector<int> Lines;
	Lines.reserve(NumberOfData * 2);
	for(int i = 0; i < NumberOfData * 2; i++)
		Lines.push_back(i);

	vector<int> ShuffledLines(Lines);
	random_shuffle(ShuffledLines.begin(), ShuffledLines.end());
	ShuffledLines.erase(ShuffledLines.begin() + ShuffledLines.size() / 2, ShuffledLines.end());

	int CollectionSize = ShuffledLines.size();
	int ItemsToFind = 50000;
	mt19937 rng;
	vector<int> Indexes;
	Indexes.reserve(ItemsToFind);
	for(int i = 0; i < ItemsToFind; i++)
		Indexes.push_back(rng() % CollectionSize);

	report_file << "plik int;" << NumberOfData << endl;

	set<int> SetCollection;
	double AddTime, FindTime, RemoveTime;

	report_file << "set" << endl;

	AddTime = TestAdd(SetCollection, ShuffledLines);
	FindTime = TestFind(SetCollection, Lines, Indexes);
	RemoveTime = TestRemove(SetCollection, Lines, Indexes);

	report_file << AddTime << ";" << FindTime << ";" << RemoveTime << endl;
	cout << AddTime << "\t" << FindTime << "\t" << RemoveTime << endl;

	report_file << "SkipList" << endl;

	bool Dynamic = true;
	int MaxP = 0;
	for(int i = 0; i < 2; i++)
	{
		Dynamic = !Dynamic;
		MaxP = Dynamic ? 96 : 91;

		for(int k = 1; k < 91; k++)
		{
			double p = 0.01 * k;
			SkipList<int> SkipListCollection(32, p, Dynamic);
			report_file << p << ";";
			cout << p << "\t";

			AddTime = TestAdd(SkipListCollection, ShuffledLines);
			FindTime = TestFind(SkipListCollection, Lines, Indexes);
			RemoveTime = TestRemove(SkipListCollection, Lines, Indexes);

			report_file << AddTime << ";" << FindTime << ";" << RemoveTime << endl;
			cout << AddTime << "\t" << FindTime << "\t" << RemoveTime << endl;
		}
	}

}
int GetFileSize(ifstream &ifs)
{
	if(ifs.is_open())
	{
		int CurrSeek = (int) ifs.tellg();
		ifs.seekg(0, ios_base::end);
		int Size = (int) ifs.tellg();
		ifs.seekg(CurrSeek);
		return Size;
	}
	else
		return 0;
}
int NumberOfLines(ifstream &ifs)
{
	if(ifs.is_open())
	{
		int CurrSeek = (int) ifs.tellg();
		int LinesNo = 0;
		string line;
		ifs.seekg(0);
		while(getline(ifs, line))
			LinesNo++;
		ifs.clear();
		ifs.seekg(CurrSeek);
		return LinesNo;
	}
	else
		return 0;
}
vector<string> ReadAllLines(ifstream &ifs, int &LinesNo)
{
	vector<string> lines;
	if(ifs.is_open())
	{
		int CurrSeek = (int) ifs.tellg();

		string s;
		ifs.seekg(0);
		LinesNo = 0;
		while(getline(ifs, s))
		{
			lines.push_back(s);
			LinesNo++;
		}
		ifs.clear();
		ifs.seekg(CurrSeek);
	}
	return lines;
}

template<typename T>
double TestAdd(set<T> &SetData, vector<T> &Lines)
{
	high_resolution_clock::time_point Timestamp[2];
	duration<double> time_span;

	Timestamp[0] = high_resolution_clock::now();
	SetData.insert(Lines.begin(), Lines.end());
	Timestamp[1] = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]);

	return time_span.count();
}
template<typename T>
double TestAdd(SkipList<T> &SkipListData, vector<T> &Lines)
{
	high_resolution_clock::time_point Timestamp[2];
	duration<double> time_span;

	Timestamp[0] = high_resolution_clock::now();
	SkipListData.insert(Lines.begin(), Lines.end());
	Timestamp[1] = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]);

	return time_span.count();
}
template<typename T>
double TestFind(set<T> &SetData, vector<T> &Lines, vector<int>& Indexes)
{
	high_resolution_clock::time_point Timestamp[2];
	duration<double> time_span;
	int Length = (int) Indexes.size();

	Timestamp[0] = high_resolution_clock::now();
	for(int i = 0; i < Length; i++)
		SetData.find(Lines[Indexes[i]]);

	Timestamp[1] = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]);

	return time_span.count();
}
template<typename T>
double TestFind(SkipList<T> &SkipListData, vector<T> &Lines, vector<int>& Indexes)
{
	high_resolution_clock::time_point Timestamp[2];
	duration<double> time_span;
	int Length = (int) Indexes.size();

	Timestamp[0] = high_resolution_clock::now();
	for(int i = 0; i < Length; i++)
		SkipListData.find(Lines[Indexes[i]]);

	Timestamp[1] = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]);

	return time_span.count();
}
template<typename T>
double TestRemove(set<T> &SetData, vector<T> &Lines, vector<int>& Indexes)
{
	high_resolution_clock::time_point Timestamp[2];
	duration<double> time_span;
	int Length = (int) Indexes.size();

	Timestamp[0] = high_resolution_clock::now();
	for(int i = 0; i < Length; i++)
		SetData.erase(Lines[Indexes[i]]);

	Timestamp[1] = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]);

	return time_span.count();
}
template<typename T>
double TestRemove(SkipList<T> &SkipListData, vector<T> &Lines, vector<int>& Indexes)
{
	high_resolution_clock::time_point Timestamp[2];
	duration<double> time_span;
	int Length = (int) Indexes.size();

	Timestamp[0] = high_resolution_clock::now();
	for(int i = 0; i < Length; i++)
		SkipListData.erase(Lines[Indexes[i]]);

	Timestamp[1] = high_resolution_clock::now();
	time_span = duration_cast<duration<double>>(Timestamp[1] - Timestamp[0]);

	return time_span.count();
}