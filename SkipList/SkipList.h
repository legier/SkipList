#pragma once
#include <memory>
#include <chrono>
#include <random>

using namespace std;
using namespace chrono;

template<class T,
class CComparer = less<T>,
class CAllocator = allocator<T>>
class SkipList
{
private:
	struct TElement
	{
		T Key;
		TElement* Previous;
		TElement** Next;
	};

public:
	typedef CComparer key_compare;
	typedef CComparer value_compare;

	class Iterator: public std::iterator<std::bidirectional_iterator_tag, typename SkipList<T>::TElement*>
	{
	private:
		typename SkipList<T>::TElement* Element;

	public:
		Iterator()
		{
			this->Element = NULL;
		}
		Iterator(typename SkipList<T>::TElement* Element)
		{
			this->Element = Element;
		}
		Iterator(const Iterator& Iterator)
		{
			this->Element = Iterator.Element;
		}
		T& operator*()
		{
			return Element->Key;
		}
		Iterator& operator ++()
		{
			if(Element != NULL)
				Element = Element->Next[0];

			return *this;
		}
		Iterator operator ++(int a)
		{
			Iterator Temp(Element);
			if(Element != NULL)
				Element = Element->Next[0];

			return Temp;
		}
		Iterator& operator --()
		{
			//tylko glowa wskazuje na null
			if(Element != NULL && Element->Previous != NULL && Element->Previous->Previous != NULL)
				Element = Element->Previous;

			return *this;
		}
		Iterator operator --(int a)
		{
			Iterator Temp(Element);
			if(Element != NULL && Element->Previous != NULL && Element->Previous->Previous != NULL)
				Element = Element->Previous;

			return Temp;
		}
		Iterator operator =(Iterator Iterator)
		{
			Element = Iterator.Element;
			return *this;
		}
		bool operator ==(Iterator& Iterator)
		{
			return Element == Iterator.Element;
		}
		bool operator !=(Iterator& Iterator)
		{
			return Element != Iterator.Element;
		}
	};

private:
	struct TFields
	{
		size_t Length;

		int Rank;
		double Probability;
		bool DynamicRank;
		geometric_distribution<int> Random;
		default_random_engine RandomGenerator;

		TElement* Head;
		TElement** PointersToModify;
		CComparer Comparer;
		CAllocator Allocator;

		int* RankStats;
	};

	TFields* Fields;

public:
	SkipList(int Rank = 32, double Probability = 0.5, bool DynamicRank = false)
	{
		Fields = new TFields();

		Fields->Rank = Rank;
		Fields->Probability = Probability;
		Fields->DynamicRank = DynamicRank;
		Fields->Comparer = CComparer();
		Fields->Allocator = CAllocator();
		Fields->Random = geometric_distribution<int>(1 - Probability);
		Fields->RandomGenerator = default_random_engine((unsigned int) system_clock::now().time_since_epoch().count());
		Fields->Length = 0;
		//Head->Rank = Rank;
		Fields->Head = new TElement();
		Fields->Head->Next = new TElement*[Rank];
		//Head->Next = shared_ptr<shared_ptr<TElement<T>>>(new shared_ptr<TElement<T>>[Rank], default_delete<shared_ptr<TElement<T>>[]>());
		for(int i = 0; i < Rank; i++)
			//Head->Next[i] = NULL;
			Fields->Head->Next[i] = NULL;
		Fields->Head->Previous = NULL;

		Fields->PointersToModify = new TElement*[Rank];//shared_ptr<TElement<T>*>(new TElement<T>*[Rank], default_delete<TElement<T>*[]>());
		Fields->RankStats = new int[Rank];
		for(int i = 0; i < Rank; i++)
			Fields->RankStats[i] = 0;
	}
	template<class _Iter>
	SkipList(_Iter first, _Iter last, int Rank = 32, double Probability = 0.5, bool DynamicRank = false) : SkipList(Rank, Probability)
	{
		insert(first, last);
	}
	SkipList(const SkipList &SkipList2) : SkipList(SkipList2.Fields->Rank, SkipList2.Fields->Probability)
	{
		for(int i = 0; i < Fields->Rank; i++)
			Fields->PointersToModify[i] = SkipList2.Fields->Head->Next[i];

		TElement* Current = SkipList2.Fields->Head->Next[0];
		int CurrRank;

		while(Current != NULL)
		{
			for(CurrRank = 1; CurrRank < Fields->Rank; CurrRank++)
			{
				if(Fields->PointersToModify[CurrRank] != Current)
					break;
				else
					Fields->PointersToModify[CurrRank] = Current->Next[CurrRank];
			}
			insert(Current->Key, CurrRank);
			Current = Current->Next[0];
		}
	}
	~SkipList()
	{
		clear();

		delete[] Fields->RankStats;
		delete[] Fields->PointersToModify;
		delete[] Fields->Head->Next;
		delete Fields->Head;
		delete Fields;
		//.Next = NULL;
	}

	//Iterators
	Iterator begin()
	{
		return Iterator(Fields->Head->Next[0]);
	}
	Iterator end()
	{
		return Iterator(NULL);
	}

	//capacity
	bool empty()
	{
		return Fields->Length == 0;
	}
	size_t size()
	{
		return Fields->Length;
	}
	size_t max_size()
	{
		return RAND_MAX;
	}

	//modifiers
	pair<Iterator, bool> insert(const T &x)
	{
		int InsRank = GetRank();
		return insert(x, InsRank);
	}
	template<class _Iter>
	void insert(_Iter firstPtr, _Iter lastPtr)
	{
		while(firstPtr != lastPtr)
		{
			int InsRank = GetRank();
			insert(*firstPtr, InsRank);
			firstPtr++;
		}
	}
	size_t erase(const T &x)
	{
		TElement *Current = Fields->Head;
		int Rank = Fields->Rank;
		while(true)
		{
			if(Current->Next[Rank - 1] == NULL)	//Current->Next[Rank - 1]->Key > x)
			{
				Rank--;
				if(Rank == 0)
					return 0;
			}
			else if(Fields->Comparer(Current->Next[Rank - 1]->Key, x))
				Current = Current->Next[Rank - 1];
			else if(Fields->Comparer(x, Current->Next[Rank - 1]->Key))
			{
				Rank--;
				if(Rank == 0)
					return 0;
			}
			else
			{
				if(Rank == 1)
				{
					if(Current->Next[0]->Next[0] != NULL)
						Current->Next[0]->Next[0]->Previous = Current->Next[0]->Previous;
					TElement * Temp = Current->Next[0];
					Current->Next[0] = Current->Next[0]->Next[0];

					Fields->Length--;

					delete[] Temp->Next;
					Fields->Allocator.destroy(&(Temp->Key));
					delete Temp;
					return 1;
				}
				else	//usuniêcie goœcia z najwy¿szego jego poziomu i szukanie kolejnych
					Current->Next[Rank - 1] = Current->Next[Rank - 1]->Next[Rank - 1];
			}
		}
	}
	void swap(SkipList &SkipList2)
	{
		TFields *TempFields = Fields;
		Fields = SkipList2.Fields;
		SkipList2.Fields = TempFields;
	}
	void clear()
	{
		TElement *Current = Fields->Head;
		TElement *Temp;

		if(Fields->Length)
		{
			Current = Current->Next[0];

			while(Fields->Length && Current->Next[0] != NULL)
			{
				Temp = Current;
				Current = Current->Next[0];

				delete[] Temp->Next;
				Fields->Allocator.destroy(&(Temp->Key));
				delete Temp;

				Fields->Length--;
			}
			for(int i = 0; i < Fields->Rank; i++)
				Fields->Head->Next[i] = NULL;
		}
	}

	//observers
	key_compare key_comp()
	{
		return CComparer(Comparer);
	}
   value_compare value_comp()
   {
      return CComparer(Comparer);
   }

	//operations
	Iterator find(const T& x)
	{
		TElement* Current = Fields->Head;
		int Rank = Fields->Rank;
		while(true)
		{
			if(Current->Next[Rank - 1] == NULL)
			{
				Rank--;
				if(Rank == 0)
					return Iterator(NULL);
			}
			else if(Fields->Comparer(Current->Next[Rank - 1]->Key, x))
				Current = Current->Next[Rank - 1];
			else if(Fields->Comparer(x, Current->Next[Rank - 1]->Key))
			{
				Rank--;
				if(Rank == 0)
					return Iterator(NULL);
			}
			else
				return Iterator(Current->Next[Rank - 1]);
		}
	}
	size_t count(const T &x)
	{
		if(find(x).second)
			return 1;
		else
			return 0;
	}
	Iterator lower_bound(const T &x)
	{
		TElement* Current = Fields->Head;
		int Rank = Fields->Rank;
		while(true)
		{
			if(Current->Next[Rank - 1] == NULL)
			{
				Rank--;
				if(Rank == 0)
					return Iterator(Current->Next[Rank - 1]);
			}
			else if(Fields->Comparer(Current->Next[Rank - 1]->Key, x))	//wiêksze
				Current = Current->Next[Rank - 1];
			else if(Fields->Comparer(x, Current->Next[Rank - 1]->Key))	//mniejsze
			{
				Rank--;
				if(Rank == 0)
					return Iterator(Current->Next[Rank - 1]);
			}
			else
				return Iterator(Current->Next[Rank - 1]);
		}
		return Iterator(NULL);
	}
	Iterator upper_bound(const T &x)
	{
		TElement* Current = Fields->Head;
		int Rank = Fields->Rank;
		while(true)
		{
			if(Current->Next[Rank - 1] == NULL)
			{
				Rank--;
				if(Rank == 0)
					return Iterator(Current->Next[Rank - 1]);
			}
			else if(Fields->Comparer(Current->Next[Rank - 1]->Key, x))	//wiêksze
				Current = Current->Next[Rank - 1];
			else if(Fields->Comparer(x, Current->Next[Rank - 1]->Key))	//mniejsze
			{
				Rank--;
				if(Rank == 0)
					return Iterator(Current->Next[Rank - 1]);
			}
			else
				return Iterator(Current->Next[Rank - 1]->Next[0]);
		}
		return Iterator(NULL);
	}

	void printAll()
	{
		//PointersToModify = Head->Next;
		cout << "h" << endl;
		for(int i = 0; i < Fields->Rank; i++)
		{
			Fields->PointersToModify[i] = Fields->Head->Next[i];
			if(Fields->PointersToModify[i] != NULL)
				cout << Fields->PointersToModify[i]->Key << "\t";
			else
				cout << "-\t";
		}
		cout << endl << endl;
		TElement* Current = Fields->Head->Next[0];
		int i;
		while(Current != NULL)
		{
			cout << Current->Key << "\t";
			for(i = 1; i < Fields->Rank; i++)
			{
				if(Fields->PointersToModify[i] == Current)
				{
					Fields->PointersToModify[i] = Current->Next[i];
					if(Current->Next[i] != NULL)
						cout << Current->Next[i]->Key << "\t";
					else
						cout << "-\t";
				}
			}
			cout << endl;
			Current = Current->Next[0];
		}
	}
	void printStats()
	{
		int64_t MemoryConsumption = 0;
		for(int i = 0; i < Fields->Rank; i++)
			cout << i + 1 << "\t";
		cout << endl;

		for(int i = 0; i < Fields->Rank; i++)
		{
			cout << Fields->RankStats[i] << "\t";
			MemoryConsumption += Fields->RankStats[i] * (sizeof(TElement) + sizeof(TElement*) * (i + 1));
		}
		cout << endl;

		if(MemoryConsumption < 1024)
			cout << "memory consumption " << MemoryConsumption << " B" << endl;
		else if(MemoryConsumption < 1048576)
			cout << "memory consumption " << MemoryConsumption / 1024.0 << " kB" << endl;
		else if(MemoryConsumption < 1073741824)
			cout << "memory consumption " << MemoryConsumption / 1048576.0 << " MB" << endl;
	}
	int64_t GetMemoryConsumption(void)
	{
		int64_t MemoryConsumption = 0;

		for(int i = 0; i < Fields->Rank; i++)
			MemoryConsumption += Fields->RankStats[i] * (sizeof(TElement) + sizeof(TElement*) * (i + 1));
		
		return MemoryConsumption;
	}

private:
	int GetRank()
	{
		int InsRank = (Fields->Random(Fields->RandomGenerator) % Fields->Rank) + 1;

		if(InsRank > Fields->Rank)
			InsRank = Fields->Rank;

		Fields->RankStats[InsRank - 1]++;

		return InsRank;
	}
	pair<Iterator, bool> insert(const T &x, int InsRank)
	{
		Iterator itr(NULL);
		bool found = false;

		TElement* InsElement = new TElement();

		//InsElement->Key = x;	//wpisanie klucza
		Fields->Allocator.construct(&(InsElement->Key), x);
		//InsElement->Rank = Rank;
		InsElement->Next = new TElement*[InsRank];
		//InsElement->Next = shared_ptr<shared_ptr<TElement<T>>>(new shared_ptr<TElement<T>>[Rank], default_delete<shared_ptr<TElement<T>>[]>());	//tablica wskaŸników

		TElement *Current = Fields->Head;
		int Rank = Fields->Rank;

		//tu siê krêcimy po elementach, które maj¹ wy¿szy rank ni¿ ten co dodajemy
		while(Rank > InsRank)
		{
			if(Current->Next[Rank - 1] == NULL)//Current->Next[CurrRank - 1]->Key > InsElement->Key)	//element na nic nie wskazuje, lub jest wiêkszy -> dekrementacja rank
				Rank--;
			else if(Fields->Comparer(Current->Next[Rank - 1]->Key, InsElement->Key))	//element jest mniejszy - przepisanie wskaŸnika -> krok bli¿ej
				Current = Current->Next[Rank - 1];
			else if(Fields->Comparer(InsElement->Key, Current->Next[Rank - 1]->Key))	//element jest mniejszy - przepisanie wskaŸnika -> krok bli¿ej
				Rank--;
			else// if(Current->Next[CurrRank - 1]->Key == InsElement->Key)	//element jest równy - nie dodajemy
				return pair<Iterator, bool>(Iterator(Current->Next[Rank - 1]), false);
		}
		//tu krêcimy siê po elementach, którym ju¿ bêdzie trzeba zmieniæ wskaŸnik
		while(Rank)
		{
			if(Current->Next[Rank - 1] == NULL || Fields->Comparer(InsElement->Key, Current->Next[Rank - 1]->Key))	//element na nic nie wskazuje, lub jest wiêkszy -> dekrementacja rank
			{
				Fields->PointersToModify[Rank - 1] = Current;	//jemu ustawimy pointer na nas
				Rank--;
			}
			else if(Fields->Comparer(Current->Next[Rank - 1]->Key, InsElement->Key))	//element jest mniejszy - przepisanie wskaŸnika -> krok bli¿ej
				Current = Current->Next[Rank - 1];
			else// if(Current->Next[CurrRank - 1]->Key == InsElement->Key)	//element jest równy - nie dodajemy
				return pair<Iterator, bool>(Iterator(Current->Next[Rank - 1]), false);
		}
		//przepisywanie wskaŸników
		for(int i = 0; i < InsRank; i++)
		{
			InsElement->Next[i] = Fields->PointersToModify[i]->Next[i];
			Fields->PointersToModify[i]->Next[i] = InsElement;
		}
		InsElement->Previous = Fields->PointersToModify[0];
		if(InsElement->Next[0] != NULL)
			InsElement->Next[0]->Previous = InsElement;
		Fields->Length++;

		if(Fields->DynamicRank)
			RecalculateRank();
		
		return pair<Iterator, bool>(Iterator(InsElement), true);
	}
	void RecalculateRank(void)
	{
		int NewRank = (int) (log(Fields->Length) / log(1.0 / Fields->Probability));
		if(NewRank > Fields->Rank)
		{
			TElement** NewNext = new TElement*[NewRank];
			int* NewRankStats = new int[NewRank];
			for(int i = 0; i < Fields->Rank; i++)
			{
				NewNext[i] = Fields->Head->Next[i];
				NewRankStats[i] = Fields->RankStats[i];
			}
			for(int i = Fields->Rank; i < NewRank; i++)
			{
				NewNext[i] = NULL;
				NewRankStats[i] = 0;
			}

			delete[] Fields->Head->Next;
			delete[] Fields->PointersToModify;
			delete[] Fields->RankStats;
			Fields->Head->Next = NewNext;
			Fields->Rank = NewRank;
			Fields->PointersToModify = new TElement*[NewRank];
			Fields->RankStats = NewRankStats;
		}
	}
};