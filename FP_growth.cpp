#include<sstream>
#include<fstream>
#include<iostream>
#include<vector>
#include<stdlib.h>
#include<map>
#include<set>
#include<algorithm>
#include<iomanip>
using namespace std;

//created by Chih Teng

// Data structure of node of FP-growth tree
struct TreeNode{
	int ID; //Product ID
	int frq; // Frequency of this node
	map<int,TreeNode*> childs; //Childrens of this node
	TreeNode* parent; //Parent of this node
	TreeNode* next; //Point to next same frequent item
	TreeNode(int _ID, int _frq,TreeNode* parent) : ID(_ID), frq(_frq),parent(parent) {
		next = nullptr;
		childs.clear();
	}
};

struct compare_frq {
	bool operator()(const pair<int, int>& p1, const pair<int, int>& p2) {
		return p1.second > p2.second;
	}
};

struct rev_compare_frq {
	bool operator()(const pair<int, int>& p1, const pair<int, int>& p2) {
		return p1.second < p2.second;
	}
};

struct compare_id {
	bool operator()(const pair<int, int>& p1, const pair<int, int>& p2) {
		return p1.first < p2.first;
	}
};

struct compare_len {
	bool operator()(const vector<int>& v1, const vector<int>& v2) {
		return v1.size() < v2.size();
	}
};

struct compare_vec {
	bool operator()(const vector<int>& v1, const vector<int>& v2) {
		for (int i = 0; i < v1.size(); ++i) {
			if (v1[i]<v2[i]) return true;
			if (v1[i]>v2[i]) return false;
		}
	}
};

vector<vector<int>> load_data(string file_path) {
	ifstream input_file(file_path);
	stringstream ss;
	string line,tmp;
	int product_id;
	vector<int> transaction;
	vector<vector<int>> transactions;
	while (!input_file.eof()) {
		getline(input_file,line);
		stringstream linestr(line);
		while (getline(linestr, tmp, ',')) {
			ss << tmp;
			ss >> product_id;
			transaction.push_back(product_id);
			ss.clear();
		}
		transactions.push_back(transaction);
		linestr.clear();
		transaction.clear();
	}
	return transactions;
}

pair<TreeNode*,pair<vector<pair<int, int>>, map<int, TreeNode*>>> build_FP_tree(vector<vector<int>> &transactions, const int min_support) {
	pair<TreeNode*, pair<vector<pair<int, int>>, map<int, TreeNode*>>> root_and_table;
	map<int,TreeNode*> headertable;
	map<int, int> frq_counter;
	vector<int> products_list,tmp_vec;
	vector<pair<int, int>> counter_vec ,tmp_counter_vec;
	TreeNode* root, * curr_node, * tmp_node;
	int product_id;
	bool notCreate;

	for (unsigned int i = 0; i < transactions.size(); ++i) {
		for (unsigned int j = 0; j < transactions[i].size(); j++) {
			product_id = transactions[i][j];
			if (frq_counter.find(product_id) == frq_counter.end()) { //product is not in frq_counter
				frq_counter[product_id] = 1;
				products_list.push_back(product_id);
			}
			else { //product is in frq_counter
				frq_counter[product_id]++;
			}
		}
	}
	//We will delete this product if total frequency of it is less than min_support
	for (unsigned int i = 0; i < products_list.size(); ++i) {
		product_id = products_list[i];
		if (frq_counter[product_id] < min_support) {
			frq_counter.erase(product_id);
		}
	}
	//Program will return NULL ,if no one frequency of product is greater than or equal to min_support
	if (frq_counter.size() == 0) {
		root_and_table.first = nullptr;
		root_and_table.second.first = counter_vec;
		root_and_table.second.second = headertable;
		return root_and_table;
	}

	//All of data in "map" are sorted by "key", so smaller ID always more front than bigger one.
	//translate map into vector 
	//We sort "map" by "value"(total frequency),sorted vector is not strictly decreasing
	for (auto itr = frq_counter.begin(); itr != frq_counter.end(); ++itr) {
		counter_vec.push_back(make_pair(itr->first, itr->second));
	}
	sort(counter_vec.begin(), counter_vec.end(), compare_frq());

	//Exclude infrequent product in transactions
	for (unsigned int i = 0; i < transactions.size(); ++i) {
		for (unsigned int j = 0; j < transactions[i].size(); ++j) {
			product_id = transactions[i][j];
			if (frq_counter.find(product_id) != frq_counter.end()) {
				tmp_counter_vec.push_back(make_pair(product_id, frq_counter[product_id]));
			}
		}
		sort(tmp_counter_vec.begin(), tmp_counter_vec.end(), compare_id());
		sort(tmp_counter_vec.begin(), tmp_counter_vec.end(), compare_frq());
		for (auto iter = tmp_counter_vec.begin(); iter != tmp_counter_vec.end(); ++iter) {
			tmp_vec.push_back(iter->first);
		}
		transactions[i] = tmp_vec;
		tmp_counter_vec.clear();
		tmp_vec.clear();
	}
	//build FP-tree
	headertable.clear();
	root = new TreeNode(-1, 0, nullptr);; // ID= -1 represent this node is root
	for (unsigned int i = 0; i < transactions.size(); ++i) {
		curr_node = root;
		notCreate = true;
		for (unsigned int j = 0; j < transactions[i].size(); ++j) {
			product_id = transactions[i][j];
			if (notCreate & (curr_node->childs.find(product_id) != curr_node->childs.end()) ) {//find this product in this node's childs
				curr_node->childs[product_id]->frq++;
			}
			else {//not found this product in the node'childs,so we create new node
				notCreate = false;
				curr_node->childs.insert(make_pair(product_id,new TreeNode(product_id, 1, curr_node)));
				if (headertable.find(product_id)!=headertable.end()) {//pointer in headertable has pointed to another same id but different node
					tmp_node = headertable[product_id];
					while (tmp_node->next != nullptr) {
						tmp_node = tmp_node->next;
					}
					tmp_node->next = curr_node->childs[product_id];
				}
				else {//pointer in headertable not point to any node
					headertable.insert(make_pair(product_id,curr_node->childs[product_id])) ;
				}
			}
			curr_node = curr_node->childs[product_id];
		}
	}
	return make_pair(root, make_pair(counter_vec,headertable));
}

//Find all prefix paths on FP-tree with the node be a suffix
vector<vector<int>> FindPrefixPath(TreeNode* &node) {
	vector<vector<int>> prefix_paths;
	vector<int> path;
	TreeNode* curr_node,*next_node;
	next_node = node;
	prefix_paths.clear();
	while (next_node!=nullptr) {
		curr_node = next_node;
		path.clear();
		while (curr_node->parent!=nullptr) {
			path.push_back(curr_node->ID);
			curr_node = curr_node->parent;
		}
		if (path.size()>0) {
			path.erase(path.begin());
			for (int i = 0; i < next_node->frq; ++i) {
				prefix_paths.push_back(path);
			}
		}
		next_node = next_node->next;
	}
	return prefix_paths;
}

//mining FP-tree
void mining(TreeNode* &root, pair<vector<pair<int, int>>, map<int, TreeNode*>> &table, const int min_support, vector<int> &prefix, vector<vector<int>> &frqPattern) {
	pair<TreeNode*, pair<vector<pair<int, int>>, map<int, TreeNode*>>> cond_root_table;
	vector<pair<int, int>> counter_vec = table.first;
	vector<pair<int, int>> rev_counter_vec;
	map<int, TreeNode*> headertable = table.second;
	vector<vector<int>> cond_path_base;
	vector<int> new_frqset;
	int suffix;
	rev_counter_vec = counter_vec;
	sort(rev_counter_vec.begin(), rev_counter_vec.end(), compare_id());
	sort(rev_counter_vec.begin(), rev_counter_vec.end(), rev_compare_frq());
	//choose one product to be a "suffix" ,order is from smallest frequency product to largest frequency one
	for (unsigned int i = 0;i<rev_counter_vec.size();++i) {
		suffix = rev_counter_vec[i].first;
		new_frqset = prefix;
		new_frqset.push_back(suffix);
		frqPattern.push_back(new_frqset);
		cond_path_base = FindPrefixPath(headertable[suffix]);
		cond_root_table = build_FP_tree(cond_path_base, min_support);//construct conditional FP-tree
		if (cond_root_table.first != nullptr) {
			mining(cond_root_table.first, cond_root_table.second, min_support, new_frqset, frqPattern);
		}
	}
}

//process the frqPattern to make it fit our needs
void Process_freqPattern(vector<vector<int>>& frqPattern) {
	map<int, int> len_counter;
	vector<int> tmp_vec;
	int ids;
	int ide;
	bool smaller;
	for (unsigned int i = 0; i < frqPattern.size(); ++i) {
		sort(frqPattern[i].begin(), frqPattern[i].end());
	}
	sort(frqPattern.begin(), frqPattern.end(), compare_len());
	for (unsigned int i = 0; i < frqPattern.size(); ++i) {
		len_counter[frqPattern[i].size()] = i;
	}
	ids = 0;
	for (auto iter = len_counter.begin(); iter != len_counter.end(); ++iter) {
		ide = iter->second;
		sort(frqPattern.begin()+ids, frqPattern.begin()+ide+1, compare_vec());
		ids = ide+1;
	}
}

vector<double> CalSuppData(const pair<vector<pair<int, int>>, map<int, TreeNode*>>& table,const vector<vector<int>>& frqPattern,const int num_tx) {
	vector<double> SuppData;
	vector<int> items, path;
	vector<vector<int>> base_prefix_paths;
	vector<pair<int, int>> counter_vec = table.first;
	vector<pair<int, int>>rev_counter_vec;
	map<int, TreeNode*> headertable = table.second;
	set<int> tmp_set,path_set;
	int support;
	bool issubset;
	rev_counter_vec = counter_vec;
	sort(rev_counter_vec.begin(), rev_counter_vec.end(), compare_id());
	sort(rev_counter_vec.begin(), rev_counter_vec.end(), rev_compare_frq());
	for (unsigned int i = 0; i < frqPattern.size(); ++i) {
		items.clear();
		tmp_set.clear();
		for (unsigned int j = 0; j < frqPattern[i].size(); ++j) {
			tmp_set.insert(frqPattern[i][j]);
		}
		for (unsigned int j = 0;j<rev_counter_vec.size();++j) {
			if (tmp_set.find(rev_counter_vec[j].first) != tmp_set.end()) items.push_back(rev_counter_vec[j].first);
		}
		base_prefix_paths = FindPrefixPath(headertable[items[0]]);
		tmp_set.erase(items[0]);
		support = 0;
		for (unsigned int j = 0; j < base_prefix_paths.size(); ++j) {
			issubset = true;
			path_set.clear();
			for (unsigned int k=0; k < base_prefix_paths[j].size(); ++k) {
				path_set.insert(base_prefix_paths[j][k]);
			}
			for (auto iter=tmp_set.begin(); iter != tmp_set.end(); ++iter) {
				if (path_set.find(*iter) == path_set.end()) {
					issubset = false;
					break;
				}
			}
			if (issubset) support++;
		}
		if (base_prefix_paths.size() == 0 & items.size() == 1) {
			support = counter_vec[items[0]].second;
		}
		SuppData.push_back( (double)support/(double)num_tx );
	}
	return SuppData;
}

void Write_output(string file_path,const vector<vector<int>>& frqPattern,const vector<double> &SuppData) {
	ofstream output_file(file_path);
	for (unsigned int i = 0; i < frqPattern.size(); ++i) {
		for (unsigned int j = 0; j < frqPattern[i].size()-1; ++j) {
			output_file << frqPattern[i][j] << ",";
		}
		output_file << *(frqPattern[i].end()-1) << ":" << fixed << setprecision(4) << SuppData[i] << endl;
	}
}

//just for checking
void levelorder(TreeNode* &root) {
	vector<TreeNode*>LastLayer, CurrLayer, NextLayer;
	vector<int>level;
	vector<vector<int>> seq;
	CurrLayer.push_back(root);
	while (!CurrLayer.empty()) {
		for (int i = 0; i < CurrLayer.size(); ++i) {
			for (auto iter = CurrLayer[i]->childs.begin(); iter != CurrLayer[i]->childs.end(); ++iter) {
				NextLayer.push_back(iter->second);
			}
			level.push_back(CurrLayer[i]->ID);
		}
		seq.push_back(level);
		LastLayer = CurrLayer;
		CurrLayer = NextLayer;
		NextLayer.clear();
		level.clear();
	}
	for (int i = 0; i < seq.size(); ++i) {
		for (int j = 0; j < seq[i].size(); ++j) {
			cout << seq[i][j] << " ";
		}
		cout << endl;
	}
}
int main(int argc,char **argv) {
	pair<TreeNode*, pair<vector<pair<int, int>>, map<int, TreeNode*>>> root_and_table;
	pair<vector<pair<int, int>>, map<int, TreeNode*>> headertable;
	vector<vector<int>> transactions, frqPattern;
	vector<int> prefix;
	vector<double> SuppData;
	int num_tx, min_support;
	double ratio = atof(argv[1]);
	TreeNode* root;
	transactions = load_data(argv[2]);
	num_tx = transactions.size();
	min_support = (int)(num_tx * ratio);
	root_and_table = build_FP_tree(transactions, min_support);
	root = root_and_table.first;
	headertable = root_and_table.second;	
	mining(root, headertable, min_support, prefix, frqPattern);
	Process_freqPattern(frqPattern);
	SuppData = CalSuppData(headertable, frqPattern, num_tx);
	Write_output(argv[3], frqPattern, SuppData);
}