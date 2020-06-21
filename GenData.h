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

/* ����100�����������ݣ�����txt�ļ��� */
void gen_data();

/* ���ļ��м��ز������� */
std::vector<KVPair<int, std::string>> import_data();

/* ���ɲ�ѯ����
miss_ratio��δ�����ʣ�����������ٱ�������Ч��ѯ
*/
std::vector<int> gen_query(const std::vector<KVPair<int, std::string>> &vec, double miss_ratio);

/* ���ɷ�Χ��ѯ���� */
std::vector<std::pair<int, int>> gen_search();