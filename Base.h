#pragma once
#include "KVPair.h"
#include <vector>
#ifdef BENCHMARK
// 模拟单次IO操作的耗时(ms)
#define SIMULATED_IO_MS 5
#endif // BENCHMARK

/* 一些抽象基类的定义 */

/* 树结点基类 */
template<class T, class U>
class TreeNode
{
public:
	int knum;					// 关键字数量
	const int m;				// B树阶数
	TreeNode<T, U> *parent;		// 父节点指针
	TreeNode<T, U> **pvec;		// 子节点指针向量
	// 返回第i个key
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

/* 树基类 */
template<class T, class U>
class Tree
{
protected:
	const int m;
	TreeNode<T, U> *root;
#ifdef BENCHMARK // 性能测试
	// 单次查询平均IO次数
	double queryMeanIO;
	// 单次查询最好IO次数
	int bestIO;
	// 单次查询最差IO次数
	int worstIO;
	// 单次查询平均耗时(ms)
	double queryMeanElapsed;
	// 当前IO次数偏离值总和，即方差*(n-1)
	double queryIODeviation;
	// 范围查询平均IO次数
	double searchMeanIO;
	// 范围查询平均耗时(ms)
	double searchMeanElapsed;
	// 查询次数，即调用find()的次数
	int queryCount;
	// 范围查询次数，即调用search()的次数
	int searchCount;
	/* 递推更新查询平均值和偏离值
	定义：
	Mn=∑(i=1->n)Xi/n
	Vn=∑(i=1->n)(xi-Mn)^2
	递推公式：
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
	/* 递推更新范围查询平均值 */
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
	// 返回单次查询平均IO次数
	double bm_query_mean_io()
	{
		return queryMeanIO;
	}
	// 返回单次查询最好IO次数
	int bm_query_best_io()
	{
		return bestIO;
	}
	// 返回单次查询最差IO次数
	int bm_query_worst_io()
	{
		return worstIO;
	}
	// 返回单次查询平均耗时
	double bm_query_mean_elapsed()
	{
		return queryMeanElapsed;
	}
	// 返回当前IO次数方差
	double bm_query_io_variance()
	{
		// 要求至少有2次查询，否则方差无意义
		return queryIODeviation / (queryCount - 1);
	}
	// 返回范围查询平均耗时
	double bm_search_mean_elapsed()
	{
		return searchMeanElapsed;
	}
	// 返回范围查询平均IO次数
	double bm_search_mean_io()
	{
		return searchMeanIO;
	}
#endif // BENCHMARK

	Tree(int m);
	virtual ~Tree() {}
	// 向树中添加一个元素
	virtual void add(KVPair<T, U> data) = 0;
	// 在树中查询关键字
	virtual KVPair<T, U>* find(T key) = 0;
	// 范围查询
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