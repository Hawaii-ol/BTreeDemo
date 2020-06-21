#pragma once
#include "KVPair.h"
#include "Base.h"
#include <vector>
#include <chrono>
#include <thread>

/* 和B树的主要不同点
1. 所有数据均存放于叶子结点，非叶子结点不存放数据
2. 叶子结点之间有链表连接
3. 分裂叶子结点时，中间键值同时进入新的叶子结点和父结点中
*/

template<class T, class U>
class BPlusTree;

/* 索引结点(非叶子结点) */
template<class T, class U>
class BPTIndexNode : public TreeNode<T, U>
{
protected:
	T *kvec;					// 索引向量
public:
	BPTIndexNode(int m) : BPTIndexNode(m, nullptr) {}
	BPTIndexNode(int m, BPTIndexNode<T, U> *parent) : TreeNode<T, U>::TreeNode(m, parent)
	{
		this->kvec = new T[m];
		this->pvec = new TreeNode<T, U>*[m];
	}
	BPTIndexNode(BPTIndexNode<T, U> *p);
	BPTIndexNode(TreeNode<T, U> *root1, TreeNode<T, U> *root2, T mid);
	virtual T key(int i) { return this->kvec[i]; }
	virtual ~BPTIndexNode();
	static int max_m(size_t page_size);
	friend class BPlusTree<T, U>;
};

// 分裂构造函数，结点p已满时分裂产生新结点
template<class T, class U>
BPTIndexNode<T, U>::BPTIndexNode(BPTIndexNode<T, U> *p) 
	: BPTIndexNode(p->m, dynamic_cast<BPTIndexNode<T, U>*>(p->parent))
{
	// 以mid为界将结点一分为二
	int i, j, mid = p->knum / 2;
	// 复制后半部分关键字向量
	for (i = mid + 1, j = 0; i < p->knum; i++, j++)
	{
		this->kvec[j] = p->kvec[i];
	}
	this->knum = j;
	// 复制后半部分子结点向量
	this->pvec = new TreeNode<T, U>*[this->m + 1];
	for (i = mid + 1, j = 0; i <= p->knum; i++, j++)
	{
		this->pvec[j] = p->pvec[i];
		this->pvec[j]->parent = this;
	}
	p->knum = mid;
}

// 根结点分裂后生成新根结点构造函数
template<class T, class U>
BPTIndexNode<T, U>::BPTIndexNode(TreeNode<T, U> *root1, TreeNode<T, U> *root2, T mid)
	: BPTIndexNode(root1->m)
{
	this->knum = 1;
	this->kvec[0] = mid;
	this->pvec = new TreeNode<T, U>*[this->m + 1]{ root1, root2 };
	root1->parent = root2->parent = this;
}

// 析构函数
template<class T, class U>
BPTIndexNode<T, U>::~BPTIndexNode()
{
	delete[] this->kvec;
	// 递归析构
}

// 大致计算在给定的磁盘页大小下一个索引结点的最大阶数
template<class T, class U>
int BPTIndexNode<T, U>::max_m(size_t page_size)
{
	// 只算父结点指针、关键字向量、子结点向量
	// 公式见BTNode
	return (page_size - sizeof(BPTIndexNode<T, U>::parent) + sizeof(T)) /
		(sizeof(T) + sizeof(TreeNode<T, U>*));
}

/* 数据结点(叶子结点) */
template<class T, class U>
class BPTDataNode : public TreeNode<T, U>
{
protected:
	KVPair<T, U> *kvec;			// 关键字向量（索引及数据）
	BPTDataNode<T, U> *next;	// 叶子结点链表
public:
	BPTDataNode(int m) : BPTDataNode(m, nullptr) {}
	BPTDataNode(int m, BPTIndexNode<T, U> *parent) : TreeNode<T, U>::TreeNode(m, parent)
	{
		this->kvec = new KVPair<T, U>[m];
		this->pvec = nullptr;
		this->next = nullptr;
	}
	BPTDataNode(BPTDataNode<T, U> *p);
	virtual T key(int i) { return this->kvec[i].key; }
	virtual ~BPTDataNode();
	friend class BPlusTree<T, U>;
};

// 分裂构造函数，结点p已满时分裂产生新结点
template<class T, class U>
BPTDataNode<T, U>::BPTDataNode(BPTDataNode<T, U> *p)
	: BPTDataNode(p->m, dynamic_cast<BPTIndexNode<T, U>*>(p->parent))
{
	// 以mid为界将结点一分为二
	int i, j, mid = p->knum / 2;
	// 根据定义将mid划到新结点中
	for (i = mid, j = 0; i < p->knum; i++, j++)
	{
		this->kvec[j] = p->kvec[i];
	}
	this->knum = j;
	p->knum = mid;
	// 连接子结点链表
	this->next = p->next;
	p->next = this;
}

// 析构函数
template<class T, class U>
BPTDataNode<T, U>::~BPTDataNode()
{
	delete[] this->kvec;
	// 不能delete next！
}

// B+树定义，这里选择中间键值进入右侧叶子结点
template<class T, class U>
class BPlusTree : public Tree<T, U>
{
private:
	// 插入关键字到结点中，结点已满时进行分裂
	void split_insert(BPTDataNode<T, U> *p, KVPair<T, U> data);
	void split_insert(BPTIndexNode<T, U> *p, TreeNode<T, U> *pnew, T key);
protected:
	BPTDataNode<T, U> *head;	// 叶子结点表头指针
public:
	BPlusTree(int m) : Tree<T, U>::Tree(m)
	{
		this->root = this->head = new BPTDataNode<T, U>(m);
	}
	virtual ~BPlusTree();
	virtual void add(KVPair<T, U> data);
	virtual KVPair<T, U>* find(T key);
	virtual std::vector<KVPair<T, U>*> search(T lowerbound, T upperbound);
};

// 析构函数
template<class T, class U>
BPlusTree<T, U>::~BPlusTree()
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
void BPlusTree<T, U>::add(KVPair<T, U> data)
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
				break;
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
		if (p->key(mid) <= data.key)
		{
			// 待插入关键字介于kvec[mid]~kvec[mid+1]之间，对应第pvec[mid+1]个子结点
			// 根据定义，相等的情况也会被归到右侧结点中
			p = p->pvec[mid + 1];
		}
		else
		{
			// 待插入关键字介于kvec[mid-1]~kvec[mid]之间，对应第pvec[mid]个子节点
			p = p->pvec[mid];
		}
	}
	// 插入数据
	split_insert(dynamic_cast<BPTDataNode<T, U>*>(p), data);
}

// 插入数据到叶子结点
template<class T, class U>
void BPlusTree<T, U>::split_insert(BPTDataNode<T, U> *p, KVPair<T, U> data)
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
			// 直接替换已有数据
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
	}
	p->kvec[i] = data;
	p->knum++;
	// 如果插入后关键字超过上限，需要进行分裂
	if (p->knum > this->m - 1)
	{
		// 以mid为界将结点一分为二
		T kmid = p->key(p->knum / 2);
		BPTDataNode<T, U> *q = new BPTDataNode<T, U>(p);
		if (p->parent == nullptr)
		{
			// 已经是根结点，生成新的根结点
			this->root = new BPTIndexNode<T, U>(p, q, kmid);
		}
		else
		{
			// 将新结点的第一个关键字插入到上一层索引结点
			split_insert(dynamic_cast<BPTIndexNode<T, U>*>(p->parent), q, kmid);
		}
	}
}

// 插入关键字到索引结点
template<class T, class U>
void BPlusTree<T, U>::split_insert(BPTIndexNode<T, U> *p, TreeNode<T, U> *pnew, T key)
{
	// 二分查找待插入位置
	int l = 0, r = p->knum - 1, mid;
	while (l <= r)
	{
		mid = (l + r) / 2;
		// 理论上不存在p->key(mid) == key的情况
		if (p->key(mid) < key)
		{
			l = mid + 1;
		}
		else
		{
			r = mid - 1;
		}
	}
	int i = p->key(mid) < key ? mid + 1 : mid, j;
	for (j = p->knum; j > i; j--)
	{
		p->kvec[j] = p->kvec[j - 1];
		p->pvec[j + 1] = p->pvec[j];
	}
	p->kvec[i] = key;
	p->knum++;
	p->pvec[i + 1] = pnew;
	// 如果插入后关键字超过上限，需要进行分裂
	if (p->knum > this->m - 1)
	{
		T kmid = p->key(p->knum / 2);
		BPTIndexNode<T, U> *q = new BPTIndexNode<T, U>(p);
		if (p->parent == nullptr)
		{
			// 已经是根结点，生成新的根结点
			this->root = new BPTIndexNode<T, U>(p, q, kmid);
		}
		else
		{
			// 将新结点的第一个关键字插入到上一层索引结点
			split_insert(dynamic_cast<BPTIndexNode<T, U>*>(p->parent), q, kmid);
		}
	}
}

template<class T, class U>
KVPair<T, U>* BPlusTree<T, U>::find(T key)
{
#ifdef BENCHMARK
	int io = 0;
	auto start = std::chrono::high_resolution_clock::now();
#endif // BENCHMARK
	TreeNode<T, U> *p = this->root;
	while (1)
	{
		// 二分查找关键字
		int l = 0, r = p->knum - 1, mid;
		while (l <= r)
		{
			mid = (l + r) / 2;
			if (p->key(mid) == key)
			{
				break;
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
#ifdef BENCHMARK
			// 计时
			auto stop = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
			// 更新统计数据
			this->bm_update_query_stat(io, static_cast<int>(duration.count()));
#endif // BENCHMARK
			BPTDataNode<T, U> *q = dynamic_cast<BPTDataNode<T, U>*>(p);
			return q->key(mid) == key ? q->kvec + mid : nullptr;
		}
		if (p->key(mid) <= key)
		{
			// 待查找关键字介于kvec[mid]~kvec[mid+1]之间，对应第pvec[mid+1]个子结点
			// 根据定义，相等的情况也会被归到右侧结点中
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
}

template<class T, class U>
std::vector<KVPair<T, U>*> BPlusTree<T, U>::search(T lowerbound, T upperbound)
{
#ifdef BENCHMARK
	int io = 0;
	auto start = std::chrono::high_resolution_clock::now();
#endif // BENCHMARK
	// 二分查找起始位置
	std::vector<KVPair<T, U>*> vec;
	int i;
	TreeNode<T, U> *p = this->root;
	BPTDataNode<T, U> *q;
	while (1)
	{
		int l = 0, r = p->knum - 1, mid;
		while (l <= r)
		{
			mid = (l + r) / 2;
			if (p->key(mid) == lowerbound)
			{
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
		if (p->pvec == nullptr)
		{
			q = dynamic_cast<BPTDataNode<T, U>*>(p);
			if (q->key(mid) >= lowerbound)
			{
				vec.push_back(q->kvec + mid);
			}
			i = mid + 1;
			goto begin;
		}
		if (p->key(mid) <= lowerbound)
		{
			p = p->pvec[mid + 1];
		}
		else
		{
			p = p->pvec[mid];
		}
#ifdef BENCHMARK
		// 模拟磁盘IO过程
		std::this_thread::sleep_for(std::chrono::milliseconds(SIMULATED_IO_MS));
		io++;
#endif // BENCHMARK
	}
begin:
	// 链表查找
	while (q != nullptr)
	{
		for (; i < q->knum; i++)
		{
			if (q->key(i) > upperbound)
			{
				goto end;
			}
			vec.push_back(q->kvec + i);
		}
		i = 0;
#ifdef BENCHMARK
		// 模拟磁盘IO过程
		std::this_thread::sleep_for(std::chrono::milliseconds(SIMULATED_IO_MS));
		io++;
#endif // BENCHMARK
		q = q->next;
	}
end:
#ifdef BENCHMARK
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	// 更新统计数据
	this->bm_update_search_stat(io, static_cast<int>(duration.count()));
#endif // BENCHMARK
	return vec;
}
