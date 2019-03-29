#include<iostream>
#include<iomanip>
#include<fstream>
#include<time.h>
#include<bits/stdc++.h>
using namespace std;

struct node
{
	int no_parents;
	double *cpd;
};

struct data_given
{
	vector<double> joint_prob;
	vector<int> dataset;
};

// struct factor
// {
// 	vector<int> scope;
// 	vector<int> prob;
// };

class helper
{
public:
	vector<bool> ProbSampleReplace(vector<double> p, vector<bool> perm, int nans, default_random_engine& gen);
	data_given read_data(string joint_file, string dataset_file);
	vector<bool> get_assignment_from_integer(int n, int no_of_nodes);
	int get_integer_from_assignment(vector<bool> assignment);
};


class graph
{
private:
	int no_nodes;
	list<int> *adj;
	struct node *node_info;
	unordered_map<int, list<int>> *parents;

public:
	
	graph(int nodes);

	void add_children(int v, list<int> w);

	void create_graph(unordered_map<int, list<int>> parent_child);

	void topologicalSortUtil(int v, bool visited[], stack<int> &Stack);

	stack<int> topologicalSort();

	double get_joint_probability(vector<bool> assignment);
	
	double get_child_val_given_parent_val(vector<vector<bool>> assignments, int child, bool child_val, vector<int> p, vector<bool> parent_val);

	double get_node_node_val(vector<vector<bool>> assignments, int node, bool node_val);

	void learn(vector<int> dataset);
	
	double calculate_L1_error(vector<double> gt_joint_prob);

	double predict_prob(unordered_map<int, bool> observed_var_val, vector<int> query_var, vector<bool> query_val);

	vector<int> generate_samples(int numSamples);
	
	void display();
	
};

	
	

	