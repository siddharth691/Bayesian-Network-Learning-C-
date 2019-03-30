#include<iostream>
#include<iomanip>
#include<fstream>
#include<bits/stdc++.h>
#include<time.h>
#include<memory>

#include "graph.h"
using namespace std;

int main()
{
	helper h;
	///Reading the data
	cout << std::setprecision(16);
	string joint_file = "Problem-4-Data/joint.dat";
	string dataset_file = "Problem-4-Data/dataset.dat";
	data_given d = h.read_data(joint_file, dataset_file);

	///Initial graph based on the intuition
	unordered_map<int, list<int>> parent_child;
	parent_child[0] = list<int> {1, 2, 3, 4};
	parent_child[1] = list<int> {5, 6, 7, 11};
	parent_child[2] = list<int> {5, 6, 7, 11};
	parent_child[3] = list<int> {5, 6, 7, 11};
	parent_child[4] = list<int> {5, 6, 7, 11};
	parent_child[5] = list<int> {8, 9, 10};
	parent_child[6] = list<int> {8, 9, 10};
	parent_child[7] = list<int> {8, 9, 10};
	parent_child[8] = list<int> {};
	parent_child[9] = list<int> {};
	parent_child[10] = list<int> {};
	parent_child[11] = list<int> {8, 9, 10};


	cout << "QUESTION 4.1 " << endl;
	cout <<"----------------" <<endl;

	///Creating the graph with random parameters and adding random data to the nodes
	graph g(12);

	///Assign the random values to the distribution
	g.create_graph(parent_child);

	///Get joint probability
	vector<bool> assignment{false, false, true, true, true, true, true, true, true, true, true, true};
	cout << "Joint probability for a sample : " << g.get_joint_probability(assignment) <<endl;

	///Learning
	g.learn(d.dataset);
	g.display();

	///Calculating accuracy for between model learned and model with random parameters
	cout <<endl;
	cout <<"L1 Error from true joint distribution : " << g.calculate_L1_error(d.joint_prob) <<endl;

	///Querying 
	unordered_map<int,bool> observed_var_val;
	//Query1
	observed_var_val[11]=true;
	observed_var_val[8]=true;
	vector<int> query_var{1};
	vector<bool> query_val{true};
	cout << "Query 1 : " <<endl;
	cout << "Prob(HasFlu = True/HasFever=True, Coughs=True) : " << g.predict_prob(observed_var_val, query_var, query_val) <<endl;
	//Query2
	observed_var_val.clear();
	observed_var_val[4]=true;
	vector<int> query_var2{7, 8, 9, 10, 11};
	cout << "Query 2 distribution : " << endl;
	cout << "-------------------------------------------------------------------" << endl;
	cout << "P[HasRash, Coughs, IsFatigued, Vomits, HasFever / pneumonia = True] " <<endl;
	for(int i =0; i<pow(2, 5); i++)
	{
		vector<bool> query_val2 = h.get_assignment_from_integer(i, 5);
		cout << query_val2[0] << ", "<<query_val2[1] << ", "<<query_val2[2] << ", "<<query_val2[3] << ", "<<query_val2[4] << " / pneumonia = True : "<< g.predict_prob(observed_var_val, query_var2, query_val2) <<endl;

	}
	//Query3
	observed_var_val.clear();
	observed_var_val[0]=true;
	vector<int> query_var3{10};
	vector<bool> query_val3{true};
	cout << "Prob(Vomit = True/IsSummer = True) : " << g.predict_prob(observed_var_val, query_var3, query_val3) <<endl;



	///Sampling and reestimating the parameters
	cout << "Sampling and Restimating" <<endl;
	cout << "-------------------------" <<endl;
	cout << "-------------------------" <<endl;
	vector<int> numSamples {4000000, 4500000, 5000000, 6000000};
	vector<double> l_errors;
	for(int i=0; i<numSamples.size(); i++)
	{
		cout << "For number of Samples = " << numSamples[i] << endl;
		vector<int> samples = g.generate_samples(numSamples[i]);
		g.learn(samples);
		g.display();
		double error = g.calculate_L1_error(d.joint_prob);
		cout <<"L1 Error from true joint distribution for "<<numSamples[i] <<" samples : "<< error <<endl;
		l_errors.push_back(error);
	}

	cout<<endl;
	cout <<"Printing L1 error with samples :" <<endl;
	cout <<"Samples | L1 Error" <<endl;
	for(int i=0; i<numSamples.size(); i++)
	{
		cout << numSamples[i] << " | "<<l_errors[i] <<endl; 
	}


		

	
	return 0;
}