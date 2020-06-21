#pragma once
#include "KVPair.h"
#include "Base.h"
#include <list>
#include <cstring>
#include <chrono>
#include <thread>

template<class T, class U>
class BTNode;

template<class T, class U>
class BTree;

template<class T, class U>
class BTNode : public TreeNode<T, U>
{
protected:
	KVPair<T, U> *kvec;		// 关键字向量
public:
	BTNode(int m);
	BTNode(int m, BTNode<T, U> *parent);
	BTNode(BTNode<T, U> *p);
	BTNode(BTNode<T, U> *root1, BTNode<T, U> *root2, KVPair<T, U> mid);
	virtual T key(int i) { return this->kvec[i].key; }
	virtual ~BTNode();
	static int max_m(size_t page_size);
	friend class BTree<T, U>;
};

// 空树根节点构造函数
template<class T, class U>
BTNode<T, U>::BTNode(int m) : BTNode(m, nullptr)
{
}

// 普通节点构造函数
template<class T, class U>
BTNode<T, U>::BTNode(int m, BTNode<T, U> *parent) : TreeNode<T, U>::TreeNode(m, parent)
{
	// 最大关键字数量是m-1，但插入后分裂前关键字数量会达到m，因此需要m个位置
	this->kvec = new KVPair<T, U>[m];
	this->pvec = nullptr;
}

// 分裂构造函数，结点p已满时分裂产生新结点
template<class T, class U>
BTNode<T, U>::BTNode(BTNode<T, U> *p) : BTNode(p->m, dynamic_cast<BTNode<T, U>*>(p->parent))
{
	// 以mid为界将结点p一分为二
	int i, j, mid = p->knum / 2;
	// 复制后半部分关键字向量
	for (i = mid + 1, j = 0; i < p->knum; i++, j++)
	{
		this->kvec[j] = p->kvec[i];
	}
	this->knum = j;
	// 复制后半部分子结点向量
	if (p->pvec != nullptr)
	{
		this->pvec = new TreeNode<T, U>*[this->m + 1];
		for (i = mid + 1, j = 0; i <= p->knum; i++, j++)
		{
			this->pvec[j] = p->pvec[i];
			this->pvec[j]->parent = this;
		}
	}
	p->knum = mid;
}

// 根结点分裂后生成新根结点构造函数
template<class T, class U>
BTNode<T, U>::BTNode(BTNode<T, U> *root1, BTNode<T, U> *root2, KVPair<T, U> mid) : BTNode(root1->m)
{
	this->knum = 1;
	this->kvec[0] = mid;
	this->pvec = new TreeNode<T, U>*[this->m + 1]{ root1, root2 };
	root1->parent = root2->parent = this;
}

// 析构函数
template<class T, class U>
BTNode<T, U>::~BTNode()
{
	delete[] this->kvec;
	// 递归析构
}

// 大致计算在给定的磁盘页大小下一个结点的最大阶数
template<class T, class U>
int BTNode<T, U>::max_m(size_t page_size)
{
	// 只算父结点指针、关键字向量、子结点向量
	// 解不等式sizeof(parent) + (m-1)*sizeof(KVPair<T,U>) + m*sizeof(TreeNode*) <= page_size
	// 得 m <= (page_size-sizeof(parent)+sizeof(KVPair<T,U>)) / (sizeof(KVPair<T,U>)+sizeof(TreeNode*))
	return (page_size - sizeof(BTNode<T, U>::parent) + sizeof(KVPair<T, U>)) /
		(sizeof(KVPair<T, U>) + sizeof(TreeNode<T, U>*));
}

template<class T, class U>
class BTree : public Tree<T, U>
{
private:
	// 插入数据到结点中，结点已满时进行分裂
	void split_insert(BTNode<T, U> *p, BTNode<T, U> *pnew, KVPair<T, U> data);
	// 中序遍历，用于范围查询
	void inorder_traverse(std::vector<KVPair<T, U>*> &vec, BTNode<T, U> *p, T lowerbound, T upperbound);
#ifdef BENCHMARK
	// 中序遍历过程委托给递归函数，难以统计IO次数，只能通过成员变量传递
	int io;
#endif // BENCHMARK

public:
	BTree(int m) : Tree<T, U>::Tree(m) { this->root = new BTNode<T, U>(m); }
	virtual ~BTree();
	virtual void add(KVPair<T, U> data);
	virtual KVPair<T, U>* find(T key);
	virtual std::vector<KVPair<T, U>*> search(T lowerbound, T upperbound);
};

// 析构函数
template<class T, class U>
BTree<T, U>::~BTree()
{
	std::list<TreeNode<T, U>*> list;
	list.push_back(this->root);
	while (!list.empty())
	{
		TreeNode<T, U> *p = list.front();
		if (p->pvec != nullptr)
		{
			for (int i = 0; i <= p->knum; i++)
			{
				list.push_back(p->pvec[i]);
			}
		}
		list.pop_front();
		delete p;
	}
}

template<class T, class U>
void BTree<T, U>::add(KVPair<T, U> data)
{
	TreeNode<T, U> *p = this->root;
	// 找到要插入的叶子结点
	while (p->pvec != nullptr)
	{
		// 二分查找关键字
		int l = 0, r = p->knum - 1, mid;
		while (l <= r)
		{
			mid = (l + r) / 2;
			if (p->key(mid) == data.key)
			{
				// 替换已有结点数据
				dynamic_cast<BTNode<T, U>*>(p)->kvec[mid] = data;
				return;
			}
			else if (p->key(mid) < data.key)
			{
				l = mid + 1;
			}
			else
			{
				r = mid - 1;
			}
		}
		if (p->key(mid) < data.key)
		{
			// 待插入关键字介于kvec[mid]~kvec[mid+1]之间，对应第pvec[mid+1]个子结点
			p = p->pvec[mid + 1];
		}
		else
		{
			// 待插入关键字介于kvec[mid-1]~kvec[mid]之间，对应第pvec[mid]个子节点
			p = p->pvec[mid];
		}
	}
	// 插入数据
	split_insert(dynamic_cast<BTNode<T, U>*>(p), nullptr, data);
}

template<class T, class U>
void BTree<T, U>::split_insert(BTNode<T, U> *p, BTNode<T, U> *pnew, KVPair<T, U> data)
{
	// 该结点没有数据，只有一种可能，就是空树的根结点，直接插入
	if (p->knum == 0)
	{
		p->kvec[0] = data;
		p->knum++;
		return;
	}
	// 二分查找待插入位置
	int l = 0, r = p->knum - 1, mid;
	while (l <= r)
	{
		mid = (l + r) / 2;
		if (p->key(mid) == data.key)
		{
			// 替换已有结点数据
			p->kvec[mid] = data;
			return;
		}
		else if (p->key(mid) < data.key)
		{
			l = mid + 1;
		}
		else
		{
			r = mid - 1;
		}
	}
	int i = p->key(mid) < data.key ? mid + 1 : mid, j;
	for (j = p->knum; j > i; j--)
	{
		p->kvec[j] = p->kvec[j - 1];
		if (p->pvec != nullptr)
		{
			p->pvec[j + 1] = p->pvec[j];
		}
	}
	p->kvec[i] = data;
	p->knum++;
	// 本次插入是递归分裂插入，需要添加子结点指针
	if (pnew)
	{
		p->pvec[i + 1] = pnew;
	}
	// 如果插入后关键字超过上限，需要进行分裂
	if (p->knum > this->m - 1)
	{
		KVPair<T, U> kvmid = p->kvec[p->knum / 2];
		BTNode<T, U> *q = new BTNode<T, U>(p);
		if (p->parent == nullptr)
		{
			// 已经是根结点，生成新的根结点
			this->root = new BTNode<T, U>(p, q, kvmid);
		}
		else
		{
			// 将中间关键字插入到上一层
			split_insert(dynamic_cast<BTNode<T, U>*>(p->parent), q, kvmid);
		}
	}
}

template<class T, class U>
KVPair<T, U>* BTree<T, U>::find(T key)
{
#ifdef BENCHMARK
	int io = 0;
	auto start = std::chrono::high_resolution_clock::now();
#endif // BENCHMARK
	TreeNode<T, U> *p = this->root;
	KVPair<T, U>* retval;
	while (1)
	{
		// 二分查找关键字
		int l = 0, r = p->knum - 1, mid;
		while (l <= r)
		{
			mid = (l + r) / 2;
			if (p->key(mid) == key)
			{
				retval = dynamic_cast<BTNode<T, U>*>(p)->kvec + mid;
				goto ret;
			}
			else if (p->key(mid) < key)
			{
				l = mid + 1;
			}
			else
			{
				r = mid - 1;
			}
		}
		if (p->pvec == nullptr)
		{
			// 查找到叶子结点仍未找到
			retval = nullptr;
			goto ret;
		}
		if (p->key(mid) < key)
		{
			// 待查找关键字介于kvec[mid]~kvec[mid+1]之间，对应第pvec[mid+1]个子结点
			p = p->pvec[mid + 1];
		}
		else
		{
			// 待查找关键字介于kvec[mid-1]~kvec[mid]之间，对应第pvec[mid]个子节点
			p = p->pvec[mid];
		}
#ifdef BENCHMARK
		// 模拟磁盘IO过程
		std::this_thread::sleep_for(std::chrono::milliseconds(SIMULATED_IO_MS));
		io++;
#endif // BENCHMARK
	}
ret:
#ifdef BENCHMARK
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	// 更新统计数据
	this->bm_update_query_stat(io, static_cast<int>(duration.count()));
#endif // BENCHMARK
	return retval;
}

template<class T, class U>
std::vector<KVPair<T, U>*> BTree<T, U>::search(T lowerbound, T upperbound)
{
#ifdef BENCHMARK
	io = 0;
	auto start = std::chrono::high_resolution_clock::now();
#endif // BENCHMARK
	BTNode<T, U> *p = dynamic_cast<BTNode<T, U>*>(this->root);
	std::vector<KVPair<T, U>*> vec;
	inorder_traverse(vec, p, lowerbound, upperbound);
#ifdef BENCHMARK
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	// 更新统计数据
	this->bm_update_search_stat(io, static_cast<int>(duration.count()));
#endif // BENCHMARK
	return vec;
}


template<class T, class U>
void BTree<T, U>::inorder_traverse(std::vector<KVPair<T, U>*> &vec, BTNode<T, U> *p, T lowerbound, T upperbound)
{
	// 二分查找下界和上界对应的子结点向量
	int begin, end;
	int l = 0, r = p->knum - 1, mid;
	while (l <= r)
	{
		mid = (l + r) / 2;
		if (p->key(mid) == lowerbound)
		{
			// 直接找到下界，将下界加入结果集
			vec.push_back(p->kvec + mid);
			break;
		}
		else if (p->key(mid) < lowerbound)
		{
			l = mid + 1;
		}
		else
		{
			r = mid - 1;
		}
	}
	if (p->key(mid) <= lowerbound)
	{
		// 包括等于的情况，此时下界已经进入结果集，从下界+1开始递归查找
		begin = mid + 1;
	}
	else
	{
		begin = mid;
	}

	l = 0, r = p->knum - 1;
	while (l <= r)
	{
		mid = (l + r) / 2;
		if (p->key(mid) == upperbound)
		{
			// 直接找到上界，但此时不能将上界加入结果集
			// 否则将破坏结果集递增顺序
			break;
		}
		else if (p->key(mid) < upperbound)
		{
			l = mid + 1;
		}
		else
		{
			r = mid - 1;
		}
	}
	if (p->key(mid) < upperbound)
	{
		end = mid + 1;
	}
	else
	{
		// 查找到上界为止，包括等于的情况
		end = mid;
	}
	for (int i = begin; i <= end; i++)
	{
		// 中序遍历下界和上界之间的子结点
		if (p->pvec != nullptr)
		{
#ifdef BENCHMARK
			// 模拟磁盘IO过程
			std::this_thread::sleep_for(std::chrono::milliseconds(SIMULATED_IO_MS));
			io++;
#endif // BENCHMARK
			inorder_traverse(vec, dynamic_cast<BTNode<T, U>*>(p->pvec[i]), lowerbound, upperbound);
		}
		// 将当前结点加入结果集
		if (i < p->knum && p->key(i) <= upperbound)
		{
			vec.push_back(p->kvec + i);
		}
	}
}
