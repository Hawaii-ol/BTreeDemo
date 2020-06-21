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
	KVPair<T, U> *kvec;		// �ؼ�������
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

// �������ڵ㹹�캯��
template<class T, class U>
BTNode<T, U>::BTNode(int m) : BTNode(m, nullptr)
{
}

// ��ͨ�ڵ㹹�캯��
template<class T, class U>
BTNode<T, U>::BTNode(int m, BTNode<T, U> *parent) : TreeNode<T, U>::TreeNode(m, parent)
{
	// ���ؼ���������m-1������������ǰ�ؼ���������ﵽm�������Ҫm��λ��
	this->kvec = new KVPair<T, U>[m];
	this->pvec = nullptr;
}

// ���ѹ��캯�������p����ʱ���Ѳ����½��
template<class T, class U>
BTNode<T, U>::BTNode(BTNode<T, U> *p) : BTNode(p->m, dynamic_cast<BTNode<T, U>*>(p->parent))
{
	// ��midΪ�罫���pһ��Ϊ��
	int i, j, mid = p->knum / 2;
	// ���ƺ�벿�ֹؼ�������
	for (i = mid + 1, j = 0; i < p->knum; i++, j++)
	{
		this->kvec[j] = p->kvec[i];
	}
	this->knum = j;
	// ���ƺ�벿���ӽ������
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

// �������Ѻ������¸���㹹�캯��
template<class T, class U>
BTNode<T, U>::BTNode(BTNode<T, U> *root1, BTNode<T, U> *root2, KVPair<T, U> mid) : BTNode(root1->m)
{
	this->knum = 1;
	this->kvec[0] = mid;
	this->pvec = new TreeNode<T, U>*[this->m + 1]{ root1, root2 };
	root1->parent = root2->parent = this;
}

// ��������
template<class T, class U>
BTNode<T, U>::~BTNode()
{
	delete[] this->kvec;
	// �ݹ�����
}

// ���¼����ڸ����Ĵ���ҳ��С��һ������������
template<class T, class U>
int BTNode<T, U>::max_m(size_t page_size)
{
	// ֻ�㸸���ָ�롢�ؼ����������ӽ������
	// �ⲻ��ʽsizeof(parent) + (m-1)*sizeof(KVPair<T,U>) + m*sizeof(TreeNode*) <= page_size
	// �� m <= (page_size-sizeof(parent)+sizeof(KVPair<T,U>)) / (sizeof(KVPair<T,U>)+sizeof(TreeNode*))
	return (page_size - sizeof(BTNode<T, U>::parent) + sizeof(KVPair<T, U>)) /
		(sizeof(KVPair<T, U>) + sizeof(TreeNode<T, U>*));
}

template<class T, class U>
class BTree : public Tree<T, U>
{
private:
	// �������ݵ�����У��������ʱ���з���
	void split_insert(BTNode<T, U> *p, BTNode<T, U> *pnew, KVPair<T, U> data);
	// ������������ڷ�Χ��ѯ
	void inorder_traverse(std::vector<KVPair<T, U>*> &vec, BTNode<T, U> *p, T lowerbound, T upperbound);
#ifdef BENCHMARK
	// �����������ί�и��ݹ麯��������ͳ��IO������ֻ��ͨ����Ա��������
	int io;
#endif // BENCHMARK

public:
	BTree(int m) : Tree<T, U>::Tree(m) { this->root = new BTNode<T, U>(m); }
	virtual ~BTree();
	virtual void add(KVPair<T, U> data);
	virtual KVPair<T, U>* find(T key);
	virtual std::vector<KVPair<T, U>*> search(T lowerbound, T upperbound);
};

// ��������
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
				// �滻���н������
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
			// ������ؼ��ֽ���kvec[mid]~kvec[mid+1]֮�䣬��Ӧ��pvec[mid+1]���ӽ��
			p = p->pvec[mid + 1];
		}
		else
		{
			// ������ؼ��ֽ���kvec[mid-1]~kvec[mid]֮�䣬��Ӧ��pvec[mid]���ӽڵ�
			p = p->pvec[mid];
		}
	}
	// ��������
	split_insert(dynamic_cast<BTNode<T, U>*>(p), nullptr, data);
}

template<class T, class U>
void BTree<T, U>::split_insert(BTNode<T, U> *p, BTNode<T, U> *pnew, KVPair<T, U> data)
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
			// �滻���н������
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
	// ���β����ǵݹ���Ѳ��룬��Ҫ����ӽ��ָ��
	if (pnew)
	{
		p->pvec[i + 1] = pnew;
	}
	// ��������ؼ��ֳ������ޣ���Ҫ���з���
	if (p->knum > this->m - 1)
	{
		KVPair<T, U> kvmid = p->kvec[p->knum / 2];
		BTNode<T, U> *q = new BTNode<T, U>(p);
		if (p->parent == nullptr)
		{
			// �Ѿ��Ǹ���㣬�����µĸ����
			this->root = new BTNode<T, U>(p, q, kvmid);
		}
		else
		{
			// ���м�ؼ��ֲ��뵽��һ��
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
		// ���ֲ��ҹؼ���
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
			// ���ҵ�Ҷ�ӽ����δ�ҵ�
			retval = nullptr;
			goto ret;
		}
		if (p->key(mid) < key)
		{
			// �����ҹؼ��ֽ���kvec[mid]~kvec[mid+1]֮�䣬��Ӧ��pvec[mid+1]���ӽ��
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
ret:
#ifdef BENCHMARK
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
	// ����ͳ������
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
	// ����ͳ������
	this->bm_update_search_stat(io, static_cast<int>(duration.count()));
#endif // BENCHMARK
	return vec;
}


template<class T, class U>
void BTree<T, U>::inorder_traverse(std::vector<KVPair<T, U>*> &vec, BTNode<T, U> *p, T lowerbound, T upperbound)
{
	// ���ֲ����½���Ͻ��Ӧ���ӽ������
	int begin, end;
	int l = 0, r = p->knum - 1, mid;
	while (l <= r)
	{
		mid = (l + r) / 2;
		if (p->key(mid) == lowerbound)
		{
			// ֱ���ҵ��½磬���½��������
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
		// �������ڵ��������ʱ�½��Ѿ��������������½�+1��ʼ�ݹ����
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
			// ֱ���ҵ��Ͻ磬����ʱ���ܽ��Ͻ��������
			// �����ƻ����������˳��
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
		// ���ҵ��Ͻ�Ϊֹ���������ڵ����
		end = mid;
	}
	for (int i = begin; i <= end; i++)
	{
		// ��������½���Ͻ�֮����ӽ��
		if (p->pvec != nullptr)
		{
#ifdef BENCHMARK
			// ģ�����IO����
			std::this_thread::sleep_for(std::chrono::milliseconds(SIMULATED_IO_MS));
			io++;
#endif // BENCHMARK
			inorder_traverse(vec, dynamic_cast<BTNode<T, U>*>(p->pvec[i]), lowerbound, upperbound);
		}
		// ����ǰ����������
		if (i < p->knum && p->key(i) <= upperbound)
		{
			vec.push_back(p->kvec + i);
		}
	}
}
