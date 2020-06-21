#pragma once
#include "KVPair.h"
#include <fstream>
#include <vector>
#include <utility>
#include <string>
#define DATA_NUM 1000000
#define QUERY_TIMES 5000
#define SEARCH_TIMES 1000
#define SEARCH_RANGE 1000
#define KEY_LOWERBOUND 1000000
#define KEY_UPPERBOUND 10000000
#define DATAFILE_NAME "testing.txt"

/* 生成100万条测试数据，存入txt文件中 */
void gen_data();

/* 从文件中加载测试数据 */
std::vector<KVPair<int, std::string>> import_data();

/* 生成查询队列
miss_ratio：未命中率，即会产生多少比例的无效查询
*/
std::vector<int> gen_query(const std::vector<KVPair<int, std::string>> &vec, double miss_ratio);

/* 生成范围查询队列 */
std::vector<std::pair<int, int>> gen_search();