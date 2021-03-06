// BTreeDemo.cpp: 定义控制台应用程序的入口点。
//

// 性能测试开关
#define BENCHMARK

// 定义磁盘页大小
#define PAGE_SIZE 512

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <utility>
#include "KVPair.h"
#include "BTree.h"
#include "BPlusTree.h"
#include "GenData.h"
using namespace std;

int main()
{
	Tree<int, string> *tree;
	int model, m;
	cout << "选择模型：" << endl << "1.B-tree" << endl << "2.B+tree" << endl;
	cin >> model;
	cout << "选择阶数m：" << endl << "1.输入m" << endl << "2.自动计算m" << endl;
	cin >> m;
	if (m == 1)
	{
		cout << "输入m：";
		cin >> m;
	}
	else if (m == 2)
	{
		m = model == 1 ? BTNode<int, string>::max_m(PAGE_SIZE) : BPTIndexNode<int, string>::max_m(PAGE_SIZE);
		cout << "当前设定磁盘页大小为" << PAGE_SIZE << "，自动设定m为" << m << endl;
	}
	else
	{
		cout << "非法输入" << endl;
		return 1;
	}
	if (model == 1)
	{
		tree = new BTree<int, string>(m);
	}
	else if (model == 2)
	{
		tree = new BPlusTree<int, string>(m);
	}
	else
	{
		cout << "非法输入" << endl;
		return 1;
	}
	// 若无数据，生成数据
	ifstream in(DATAFILE_NAME);
	if (!in)
	{
		// 也可能是权限等问题，但暂且默认为是文件不存在
		cout << "当前无测试数据，生成测试数据中..." << endl;
		gen_data();
	}
	// 导入数据
	cout << "导入数据中..." << endl;
	vector<KVPair<int, string>> vec_data = import_data();
	for (auto &data : vec_data)
	{
		tree->add(data);
	}
#ifdef BENCHMARK
	// 生成查询
	cout << "生成查询中..." << endl;
	vector<int> vec_query = gen_query(vec_data, 0.2);
	auto start = chrono::high_resolution_clock::now();
	for (auto &key : vec_query)
	{
		KVPair<int, string>* p = tree->find(key);
	}
	auto stop = chrono::high_resolution_clock::now();
	auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
	// 打印测试结果
	cout << "==============================" << endl;
	cout << "测试结果" << endl;
	cout << "总查询次数：" << vec_query.size() << endl;
	cout << "总查询耗时：" << duration.count() / 1000.0 << "s" << endl;
	cout << "单次查询平均耗时：" << tree->bm_query_mean_elapsed() << "ms" << endl;
	cout << "单次查询最好IO次数：" << tree->bm_query_best_io() << endl;
	cout << "单次查询最差IO次数：" << tree->bm_query_worst_io() << endl;
	cout << "单次查询平均IO次数：" << tree->bm_query_mean_io() << endl;
	cout << "IO次数方差：" << tree->bm_query_io_variance() << endl;
	cout << "==============================" << endl;
	// 生成范围查询
	cout << "生成范围查询中..." << endl;
	vector<pair<int, int>> vec_search = gen_search();
	start = chrono::high_resolution_clock::now();
	for (auto &p : vec_search)
	{
		tree->search(p.first, p.second);
	}
	stop = chrono::high_resolution_clock::now();
	duration = chrono::duration_cast<chrono::milliseconds>(stop - start);
	// 打印测试结果
	cout << "==============================" << endl;
	cout << "测试结果" << endl;
	cout << "总范围查询次数：" << vec_search.size() << endl;
	cout << "总范围查询耗时：" << duration.count() / 1000.0 << "s" << endl;
	cout << "单次范围查询平均耗时：" << tree->bm_search_mean_elapsed() << "ms" << endl;
	cout << "单次范围查询平均IO次数：" << tree->bm_search_mean_io() << endl;
	cout << "==============================" << endl;
#else
	// 用户输入查询
	int key;
	cout << "输入查询关键字：" << endl;
	while (cin >> key)
	{
		KVPair<int, string>* p = tree->find(key);
		cout << key << ' ';
		if (p != nullptr)
		{
			cout << p->value << endl;
		}
		else
		{
			cout << "[NOT FOUND]" << endl;
		}
	}
#endif // BENCHMARK
	delete tree;
    return 0;
}

