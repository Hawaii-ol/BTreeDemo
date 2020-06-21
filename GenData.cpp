#include <random>
#include <unordered_set>
#include "GenData.h"

using namespace std;

void gen_data()
{
	random_device rd;
	mt19937 eng(rd());
	uniform_int_distribution<> ch_dist{ 0, 61 };

	int range = KEY_UPPERBOUND - KEY_LOWERBOUND;
	int *record = new int[range];
	int i, j;
	int key;
	char value[6];
	for (i = 0; i < range; i++)
	{
		record[i] = i + KEY_LOWERBOUND;
	}
	ofstream out(DATAFILE_NAME);
	for (i = 0; i < DATA_NUM; i++)
	{
		j = uniform_int_distribution<>{ 0, range }(eng);
		key = record[j];
		record[j] = record[range - 1];
		record[range - 1] = key;
		range--;
		for (j = 0; j < 5; j++)
		{
			int t = ch_dist(eng);
			if (t < 10)
			{
				value[j] = t + '0';
			}
			else if (t < 36)
			{
				value[j] = t + 'A' - 10;
			}
			else
			{
				value[j] = t + 'a' - 36;
			}
		}
		value[j] = '\0';
		out << key << ' ' << value << endl;
	}
	out.close();
	delete record;
}

vector<KVPair<int, string>> import_data()
{
	ifstream in(DATAFILE_NAME);
	int key;
	string value;
	vector<KVPair<int, string>> vec;
	while (in >> key >> value)
	{
		vec.push_back(KVPair<int, string>{key, value});
	}
	return vec;
}

vector<int> gen_query(const vector<KVPair<int, string>> &vec, double miss_ratio)
{
	random_device rd;
	mt19937 eng(rd());
	uniform_int_distribution<> dist_key{ KEY_LOWERBOUND, KEY_UPPERBOUND };
	uniform_int_distribution<> dist_index{ 0, static_cast<int>(vec.size()) };
	uniform_real_distribution<> dist_ratio{ 0.0, 1.0 };

	vector<int> ret;
	unordered_set<int> htable;
	for (auto &r : vec)
	{
		htable.insert(r.key);
	}
	for (int i = 0; i < QUERY_TIMES; i++)
	{
		if (dist_ratio(eng) < miss_ratio)
		{
			int invalid_key;
			while (htable.count(invalid_key = dist_key(eng)));
			ret.push_back(invalid_key);
		}
		else
		{
			ret.push_back(vec[dist_index(eng)].key);
		}
	}
	return ret;
}

vector<pair<int, int>> gen_search()
{
	random_device rd;
	mt19937 eng(rd());
	uniform_int_distribution<> dist{ KEY_LOWERBOUND, KEY_UPPERBOUND - SEARCH_RANGE };

	vector<pair<int, int>> ret;
	for (int i = 0; i < SEARCH_TIMES; i++)
	{
		pair<int, int> p;
		p.first = dist(eng);
		p.second = p.first + SEARCH_RANGE;
		ret.push_back(p);
	}
	return ret;
}