#pragma once
#include "KVPair.h"
#include <vector>
#ifdef BENCHMARK
// ģ�ⵥ��IO�����ĺ�ʱ(ms)
#define SIMULATED_IO_MS 5
#endif // BENCHMARK

/* һЩ�������Ķ��� */

/* �������� */
template<class T, class U>
class TreeNode
{
public:
	int knum;					// �ؼ�������
	const int m;				// B������
	TreeNode<T, U> *parent;		// ���ڵ�ָ��
	TreeNode<T, U> **pvec;		// �ӽڵ�ָ������
	// ���ص�i��key
	virtual T key(int i) = 0;
	TreeNode(int m) : TreeNode(m, nullptr) {}
	TreeNode(int m, TreeNode<T, U> *parent) : m(m), parent(parent), knum(0) {}
	virtual ~TreeNode()
	{
		if (pvec)
		{
			delete[] pvec;
		}
	}
};

/* ������ */
template<class T, class U>
class Tree
{
protected:
	const int m;
	TreeNode<T, U> *root;
#ifdef BENCHMARK // ���ܲ���
	// ���β�ѯƽ��IO����
	double queryMeanIO;
	// ���β�ѯ���IO����
	int bestIO;
	// ���β�ѯ���IO����
	int worstIO;
	// ���β�ѯƽ����ʱ(ms)
	double queryMeanElapsed;
	// ��ǰIO����ƫ��ֵ�ܺͣ�������*(n-1)
	double queryIODeviation;
	// ��Χ��ѯƽ��IO����
	double searchMeanIO;
	// ��Χ��ѯƽ����ʱ(ms)
	double searchMeanElapsed;
	// ��ѯ������������find()�Ĵ���
	int queryCount;
	// ��Χ��ѯ������������search()�Ĵ���
	int searchCount;
	/* ���Ƹ��²�ѯƽ��ֵ��ƫ��ֵ
	���壺
	Mn=��(i=1->n)Xi/n
	Vn=��(i=1->n)(xi-Mn)^2
	���ƹ�ʽ��
	Mn=M(n-1)+(xn-M(n-1))/n, M1=x1
	Vn=V(n-1)+(xn-M(n-1))*(xn-Mn), V1=0
	*/
	void bm_update_query_stat(int io, int elapsed)
	{
		if (queryCount == 0)
		{
			queryMeanIO = io;
			queryMeanElapsed = elapsed;
			queryIODeviation = 0.0;
			bestIO = worstIO = io;
		}
		else
		{
			double currentMeanIO = queryMeanIO;
			queryMeanIO = queryMeanIO + (io - queryMeanIO) / (queryCount + 1);
			queryIODeviation = queryIODeviation + (io - currentMeanIO) * (io - queryMeanIO);
			queryMeanElapsed = queryMeanElapsed + (elapsed - queryMeanElapsed) / (queryCount + 1);
			if (io < bestIO)
			{
				bestIO = io;
			}
			else if (io > worstIO)
			{
				worstIO = io;
			}
		}
		queryCount++;
	}
	/* ���Ƹ��·�Χ��ѯƽ��ֵ */
	void bm_update_search_stat(int io, int elapsed)
	{
		if (searchCount == 0)
		{
			searchMeanIO = io;
			searchMeanElapsed = elapsed;
		}
		else
		{
			searchMeanIO = searchMeanIO + (io - searchMeanIO) / (searchCount + 1);
			searchMeanElapsed = searchMeanElapsed + (elapsed - searchMeanElapsed) / (searchCount + 1);
		}
		searchCount++;
	}
#endif // BENCHMARK
public:
#ifdef BENCHMARK
	// ���ص��β�ѯƽ��IO����
	double bm_query_mean_io()
	{
		return queryMeanIO;
	}
	// ���ص��β�ѯ���IO����
	int bm_query_best_io()
	{
		return bestIO;
	}
	// ���ص��β�ѯ���IO����
	int bm_query_worst_io()
	{
		return worstIO;
	}
	// ���ص��β�ѯƽ����ʱ
	double bm_query_mean_elapsed()
	{
		return queryMeanElapsed;
	}
	// ���ص�ǰIO��������
	double bm_query_io_variance()
	{
		// Ҫ��������2�β�ѯ�����򷽲�������
		return queryIODeviation / (queryCount - 1);
	}
	// ���ط�Χ��ѯƽ����ʱ
	double bm_search_mean_elapsed()
	{
		return searchMeanElapsed;
	}
	// ���ط�Χ��ѯƽ��IO����
	double bm_search_mean_io()
	{
		return searchMeanIO;
	}
#endif // BENCHMARK

	Tree(int m);
	virtual ~Tree() {}
	// ���������һ��Ԫ��
	virtual void add(KVPair<T, U> data) = 0;
	// �����в�ѯ�ؼ���
	virtual KVPair<T, U>* find(T key) = 0;
	// ��Χ��ѯ
	virtual std::vector<KVPair<T, U>*> search(T lowerbound, T upperbound) = 0;
};

template<class T, class U>
Tree<T, U>::Tree(int m) : m(m)
{
#ifdef BENCHMARK
	queryCount = 0;
	searchCount = 0;
#endif // BENCHMARK
}