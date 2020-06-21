#pragma once
#include "KVPair.h"
#include "Base.h"
#include <vector>
#include <chrono>
#include <thread>

/* ��B������Ҫ��ͬ��
1. �������ݾ������Ҷ�ӽ�㣬��Ҷ�ӽ�㲻�������
2. Ҷ�ӽ��֮������������
3. ����Ҷ�ӽ��ʱ���м��ֵͬʱ�����µ�Ҷ�ӽ��͸������
*/

template<class T, class U>
class BPlusTree;

/* �������(��Ҷ�ӽ��) */
template<class T, class U>
class BPTIndexNode : public TreeNode<T, U>
{
protected:
	T *kvec;					// ��������
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

// ���ѹ��캯�������p����ʱ���Ѳ����½��
template<class T, class U>
BPTIndexNode<T, U>::BPTIndexNode(BPTIndexNode<T, U> *p) 
	: BPTIndexNode(p->m, dynamic_cast<BPTIndexNode<T, U>*>(p->parent))
{
	// ��midΪ�罫���һ��Ϊ��
	int i, j, mid = p->knum / 2;
	// ���ƺ�벿�ֹؼ�������
	for (i = mid + 1, j = 0; i < p->knum; i++, j++)
	{
		this->kvec[j] = p->kvec[i];
	}
	this->knum = j;
	// ���ƺ�벿���ӽ������
	this->pvec = new TreeNode<T, U>*[this->m + 1];
	for (i = mid + 1, j = 0; i <= p->knum; i++, j++)
	{
		this->pvec[j] = p->pvec[i];
		this->pvec[j]->parent = this;
	}
	p->knum = mid;
}

// �������Ѻ������¸���㹹�캯��
template<class T, class U>
BPTIndexNode<T, U>::BPTIndexNode(TreeNode<T, U> *root1, TreeNode<T, U> *root2, T mid)
	: BPTIndexNode(root1->m)
{
	this->knum = 1;
	this->kvec[0] = mid;
	this->pvec = new TreeNode<T, U>*[this->m + 1]{ root1, root2 };
	root1->parent = root2->parent = this;
}

// ��������
template<class T, class U>
BPTIndexNode<T, U>::~BPTIndexNode()
{
	delete[] this->kvec;
	// �ݹ�����
}

// ���¼����ڸ����Ĵ���ҳ��С��һ����������������
template<class T, class U>
int BPTIndexNode<T, U>::max_m(size_t page_size)
{
	// ֻ�㸸���ָ�롢�ؼ����������ӽ������
	// ��ʽ��BTNode
	return (page_size - sizeof(BPTIndexNode<T, U>::parent) + sizeof(T)) /
		(sizeof(T) + sizeof(TreeNode<T, U>*));
}

/* ���ݽ��(Ҷ�ӽ��) */
template<class T, class U>
class BPTDataNode : public TreeNode<T, U>
{
protected:
	KVPair<T, U> *kvec;			// �ؼ������������������ݣ�
	BPTDataNode<T, U> *next;	// Ҷ�ӽ������
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

// ���ѹ��캯�������p����ʱ���Ѳ����½��
template<class T, class U>
BPTDataNode<T, U>::BPTDataNode(BPTDataNode<T, U> *p)
	: BPTDataNode(p->m, dynamic_cast<BPTIndexNode<T, U>*>(p->parent))
{
	// ��midΪ�罫���һ��Ϊ��
	int i, j, mid = p->knum / 2;
	// ���ݶ��彫mid�����½����
	for (i = mid, j = 0; i < p->knum; i++, j++)
	{
		this->kvec[j] = p->kvec[i];
	}
	this->knum = j;
	p->knum = mid;
	// �����ӽ������
	this->next = p->next;
	p->next = this;
}

// ��������
template<class T, class U>
BPTDataNode<T, U>::~BPTDataNode()
{
	delete[] this->kvec;
	// ����delete next��
}

// B+�����壬����ѡ���м��ֵ�����Ҳ�Ҷ�ӽ��
template<class T, class U>
class BPlusTree : public Tree<T, U>
{
private:
	// ����ؼ��ֵ�����У��������ʱ���з���
	void split_insert(BPTDataNode<T, U> *p, KVPair<T, U> data);
	void split_insert(BPTIndexNode<T, U> *p, TreeNode<T, U> *pnew, T key);
protected:
	BPTDataNode<T, U> *head;	// Ҷ�ӽ���ͷָ��
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

// ��������
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
	// �ҵ�Ҫ�����Ҷ�ӽ��
	while (p->pvec != nullptr)
	{
		// ���ֲ��ҹؼ���
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
			// ������ؼ��ֽ���kvec[mid]~kvec[mid+1]֮�䣬��Ӧ��pvec[mid+1]���ӽ��
			// ���ݶ��壬��ȵ����Ҳ�ᱻ�鵽�Ҳ�����
			p = p->pvec[mid + 1];
		}
		else
		{
			// ������ؼ��ֽ���kvec[mid-1]~kvec[mid]֮�䣬��Ӧ��pvec[mid]���ӽڵ�
			p = p->pvec[mid];
		}
	}
	// ��������
	split_insert(dynamic_cast<BPTDataNode<T, U>*>(p), data);
}

// �������ݵ�Ҷ�ӽ��
template<class T, class U>
void BPlusTree<T, U>::split_insert(BPTDataNode<T, U> *p, KVPair<T, U> data)
{
	// �ý��û�����ݣ�ֻ��һ�ֿ��ܣ����ǿ����ĸ���㣬ֱ�Ӳ���
	if (p->knum == 0)
	{
		p->kvec[0] = data;
		p->knum++;
		return;
	}
	// ���ֲ��Ҵ�����λ��
	int l = 0, r = p->knum - 1, mid;
	while (l <= r)
	{
		mid = (l + r) / 2;
		if (p->key(mid) == data.key)
		{
			// ֱ���滻��������
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
	// ��������ؼ��ֳ������ޣ���Ҫ���з���
	if (p->knum > this->m - 1)
	{
		// ��midΪ�罫���һ��Ϊ��
		T kmid = p->key(p->knum / 2);
		BPTDataNode<T, U> *q = new BPTDataNode<T, U>(p);
		if (p->parent == nullptr)
		{
			// �Ѿ��Ǹ���㣬�����µĸ����
			this->root = new BPTIndexNode<T, U>(p, q, kmid);
		}
		else
		{
			// ���½��ĵ�һ���ؼ��ֲ��뵽��һ���������
			split_insert(dynamic_cast<BPTIndexNode<T, U>*>(p->parent), q, kmid);
		}
	}
}

// ����ؼ��ֵ��������
template<class T, class U>
void BPlusTree<T, U>::split_insert(BPTIndexNode<T, U> *p, TreeNode<T, U> *pnew, T key)
{
	// ���ֲ��Ҵ�����λ��
	int l = 0, r = p->knum - 1, mid;
	while (l <= r)
	{
		mid = (l + r) / 2;
		// �����ϲ�����p->key(mid) == key�����
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
	// ��������ؼ��ֳ������ޣ���Ҫ���з���
	if (p->knum > this->m - 1)
	{
		T kmid = p->key(p->knum / 2);
		BPTIndexNode<T, U> *q = new BPTIndexNode<T, U>(p);
		if (p->parent == nullptr)
		{
			// �Ѿ��Ǹ���㣬�����µĸ����
			this->root = new BPTIndexNode<T, U>(p, q, kmid);
		}
		else
		{
			// ���½��ĵ�һ���ؼ��ֲ��뵽��һ���������
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
		// ���ֲ��ҹؼ���
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
			// ��ʱ
			auto stop = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
			// ����ͳ������
			this->bm_update_query_stat(io, static_cast<int>(duration.count()));
#endif // BENCHMARK
			BPTDataNode<T, U> *q = dynamic_cast<BPTDataNode<T, U>*>(p);
			return q->key(mid) == key ? q->kvec + mid : nullptr;
		}
		if (p->key(mid) <= key)
		{
			// �����ҹؼ��ֽ���kvec[mid]~kvec[mid+1]֮�䣬��Ӧ��pvec[mid+1]���ӽ��
			// ���ݶ��壬��ȵ����Ҳ�ᱻ�鵽�Ҳ�����
			p = p->pvec[mid + 1];
		}
		else
		{
			// �����ҹؼ��ֽ���kvec[mid-1]~kvec[mid]֮�䣬��Ӧ��pvec[mid]���ӽڵ�
			p = p->pvec[mid];
		}
#ifdef BENCHMARK
		// ģ�����IO����
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
	// ���ֲ�����ʼλ��
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
		// ģ�����IO����
		std::this_thread::sleep_for(std::chrono::milliseconds(SIMULATED_IO_MS));
		io++;
#endif // BENCHMARK
	}
begin:
	// �������
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
		// ģ�����IO����
		std::this_thread::sleep_for(std::chrono::milliseconds(SIMULATED_IO_MS));
		io++;
#endif // BENCHMARK
		q = q->next;
	}
end:
#ifdef BENCHMARK
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	// ����ͳ������
	this->bm_update_search_stat(io, static_cast<int>(duration.count()));
#endif // BENCHMARK
	return vec;
}
