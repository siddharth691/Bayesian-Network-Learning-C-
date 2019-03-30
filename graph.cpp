#include<iostream>
#include<iomanip>
#include<fstream>
#include<bits/stdc++.h>
#include<time.h>
#include<memory>

using namespace std;
#include"graph.h"

helper h;

vector<bool> helper::ProbSampleReplace(vector<double> p, vector<bool> perm, int nans, default_random_engine& gen)
{
	int n = p.size();
    double rU;
    int i, j;
    vector<bool> ans;

    uniform_real_distribution<double> distribution(0.0,1.0);

    for (i = 1 ; i < n; i++)
        p[i] += p[i - 1];

    for (i = 0; i < nans; i++) {
        double rU = distribution(gen);
        for (j = 0; j < n - 1; j++) {
            if (rU <= p[j])
                break;
            }
        ans.push_back(perm[j]);
    }

    return ans;
}

data_given helper::read_data(string joint_file, string dataset_file)
{
	ifstream file;
	

	//parsing joint distribution file
	file.open(joint_file);
	vector<double> joint_prob;
	int data;
	double prob;
	for(int i=0; i<4096; i++)
	{
		file >> data >> prob;
		joint_prob.push_back(prob);
	}
	file.close();

	// parsing dataset file
	vector<int> dataset;
	file.open(dataset_file);

	while(file >> data)
	{
		dataset.push_back(data);
	}
	file.close();
	data_given d;
	d.joint_prob = joint_prob;
	d.dataset = dataset;
	return d;
}


vector<bool> helper::get_assignment_from_integer(int n, int no_of_nodes)
{
	vector<bool> ret (no_of_nodes, false);

	int i = 0; 
	int rem;
    while (n > 0) { 
  
        // storing remainder in binary array 
        rem = n % 2; 
        if(rem==1)
        	ret[i]=true;
        n = n / 2; 
        i++; 
    }
	return ret;

}

int helper::get_integer_from_assignment(vector<bool> assignment)
{
	int tot =0;
	for(int i=0; i<assignment.size(); i++)
	{
		if(assignment[i]==true)
			tot+=int(pow(2,i));
	}
	return tot;
}


//constructor for assigning the memory and no of nodes
graph::graph(int nodes)
{
	no_nodes = nodes;
	adj = shared_ptr<list<int>[]>(new list<int>[no_nodes]);
	// adj = new list<int>[no_nodes];
	node_info = shared_ptr<struct node[]>(new node[no_nodes]);
	// node_info = new struct node[no_nodes];
	parents = shared_ptr<unordered_map<int, list<int>>>(new unordered_map<int, list<int>>);
	// parents = new unordered_map<int, list<int>>;
}

//function to add children
void graph::add_children(int v, list<int> w)
{
	adj[v] = w;
}

//function to create graph (adj) and add random data to node info
void graph::create_graph(unordered_map<int, list<int>> parent_child)
{
	/*
	1. Creating graph from given parent_child map
	2. Assigning parents to parents map
	3. Assigning no of parents and random probabilities to each node
	   while assigning random probabilities True -> 0th row or column, 1 correspond to 1st row or column
	*/
	srand(time(0));
	for(auto i=parent_child.begin(); i!=parent_child.end(); i++)
	{
		add_children(i->first, i->second);
	}

	//assigning parents for each node
	for(int i=0; i<no_nodes; i++)
		(*parents)[i] = list<int> {};

	for(int i=0; i<no_nodes; i++)
		for(auto j=adj[i].begin(); j!=adj[i].end(); j++)
			(*parents)[*j].push_back(i);


	//assigning the data to each node -> no of parents and random probabilities
	for(int i=0; i<no_nodes; i++)
	{
		node_info[i].no_parents = (*parents)[i].size();

		if(node_info[i].no_parents ==0)
		{
			// there are no parents for this node
			// assigning random double cpd[2] = { P[node=false], P[node=true]}
			node_info[i].cpd = shared_ptr<double[]>(new double[2]);
			// node_info[i].cpd = new double[2];
			double f = (double)rand() /RAND_MAX;
			node_info[i].cpd[0] = f;
			node_info[i].cpd[1] = 1-f;

		}
		else
		{
			//some random Conditional probability for each (child,parent1,parent2..) combination vector<double> of size 2^(no_of parents + 1)
			node_info[i].cpd = shared_ptr<double[]>(new double[int(pow(2, node_info[i].no_parents+1))]);
			// node_info[i].cpd = new double[int(pow(2, node_info[i].no_parents+1))];
			int cur_index=0;
			for(int j=0; j<int(pow(2, node_info[i].no_parents)); j++)
			{	
				double f = (double)rand() / RAND_MAX;
				node_info[i].cpd[cur_index]= f;
				cur_index++;
				node_info[i].cpd[cur_index]= 1-f;
				cur_index++;

			}
		}
	}
}

void graph::topologicalSortUtil(int v, shared_ptr<bool[]> visited, stack<int> &Stack) 
{ 
    visited[v] = true; 
  
    for (auto i = adj[v].begin(); i != adj[v].end(); ++i) 
        if (!visited[*i]) 
            topologicalSortUtil(*i, visited, Stack); 
  
    Stack.push(v); 
} 

stack<int> graph::topologicalSort() 
{ 
    stack<int> Stack; 
  
    shared_ptr<bool[]> visited = shared_ptr<bool[]>(new bool[no_nodes]); 
    for (int i = 0; i < no_nodes; i++) 
        visited[i] = false; 

    for (int i = 0; i < no_nodes; i++) 
      if (visited[i] == false) 
        topologicalSortUtil(i, visited, Stack); 
  
   return Stack;
} 

//function to return joint probability given the assignment
double graph::get_joint_probability(vector<bool> assignment)
{
	double prob = 1;
	unordered_map<bool, int> m;
	m[true]=1;
	m[false]=0;


	for(int i=0; i<no_nodes; i++)
	{
		if(node_info[i].no_parents ==0)
		{
			prob*=node_info[i].cpd[m[assignment[i]]];
		}
		else
		{
			
			//first create vector of boolean which are values from assignment (child + parents => in order stored in (*parents)) 
			//and then convert to integer
			vector<bool> index_of_prob; 

			//pushing the value of child
			index_of_prob.push_back(assignment[i]);

			//pushing the values of all the parents in the order stored in (*parents)[i]
			for(auto parent_index = (*parents)[i].begin(); parent_index!=(*parents)[i].end(); parent_index++)
			{
				index_of_prob.push_back(assignment[*parent_index]);
			}
			
			//converting index_of_prob to integer
			int index = h.get_integer_from_assignment(index_of_prob);
			prob*=node_info[i].cpd[index];

		}
		
	}

	return prob;
}

double graph::get_child_val_given_parent_val(vector<vector<bool>> assignments, int child, bool child_val, vector<int> p, vector<bool> parent_val)
{
	int tot_count = 0;
	int tot_child_count =0;
	bool sum_it = true;
	for(int i=0; i<assignments.size(); i++)
	{
		sum_it = true;
		for(int j=0; j<p.size(); j++)
		{
			if(assignments[i][p[j]] != parent_val[j])
				{
					sum_it = false;
					break;
				}
		}

		if(sum_it)
		{
			tot_count +=1;
			if(assignments[i][child] == child_val)
				tot_child_count+=1;
		}
	}

	return (double)(tot_child_count/(double)tot_count);

}

double graph::get_node_node_val(vector<vector<bool>> assignments, int node, bool node_val)
{
	int tot_count= assignments.size();
	int tot_count_val =0;
	for(int i=0; i<assignments.size(); i++)
	{
		if(assignments[i][node] == node_val)
		{
			tot_count_val+=1;
		}
	}

	return (double)(tot_count_val/(double)tot_count);
}

void graph::learn(vector<int> dataset)
{
	cout << "Learning : " <<endl;
	cout << "-------------"<<endl;
	cout << "-------------"<<endl;
	vector<vector<bool>> assignments;
	for(int i=0; i<dataset.size(); i++)
	{
		assignments.push_back(h.get_assignment_from_integer(dataset[i], no_nodes));
	}



	//assigning learned probabilities to each node
	for(int i=0; i<no_nodes; i++)
	{
		cout << "learning parameters for node : " << i <<endl;
		node_info[i].no_parents = (*parents)[i].size();

		if(node_info[i].no_parents ==0)
		{
			// there are no parents for this node
			// assigning double cpd[2] = { P[node=false], P[node=true]} based on data

			double f = get_node_node_val(assignments, i, true);
			node_info[i].cpd[1]=f;
			node_info[i].cpd[0]=1-f;

		}
		else
		{
			//Conditional probability for each (child,parent1,parent2..) combination vector<double> of size 2^(no_of parents + 1) 
			//based on data

			int cur_index=0;
			for(int j=0; j<int(pow(2, node_info[i].no_parents)); j++)
			{	
				/* probability[Child = false /parents(in order of (*parents)[i]) = boolen(j)]*/
				vector<int> p;
				for(auto it = (*parents)[i].begin(); it!=(*parents)[i].end(); it++)
					p.push_back(*it);

				double f_false = get_child_val_given_parent_val(assignments,i,false,p,h.get_assignment_from_integer(j, node_info[i].no_parents));
				node_info[i].cpd[cur_index]= f_false;
				cur_index++;

				/*probability[Child = true /parents(in order of (*parents)[i]) = boolen(j)]*/
				double f_true = get_child_val_given_parent_val(assignments,i,true,p,h.get_assignment_from_integer(j, node_info[i].no_parents));
				//ideally f_true+f_false should be 1;
				node_info[i].cpd[cur_index]= f_true;
				cur_index++;

			}

		}
	}

	cout <<"Parameter Learning Done" <<endl;

}

double graph::calculate_L1_error(vector<double> gt_joint_prob)
{
	vector<bool> assignment;
	double true_joint_prob;
	double model_predicted_joint_prob;

	double tot_error = 0;

	double tot_pred_prob=0;
	double tot_true_prob=0;
	cout << "total true : total predicted joint distribution"<<endl;
	for(int i=0; i<gt_joint_prob.size(); i++)
	{
		assignment = h.get_assignment_from_integer(i, no_nodes);
		model_predicted_joint_prob = get_joint_probability(assignment);
		
		true_joint_prob = gt_joint_prob[i];

		tot_pred_prob+=model_predicted_joint_prob;
		tot_true_prob+=true_joint_prob;
		tot_error+=abs(true_joint_prob - model_predicted_joint_prob);
	}
	cout << tot_true_prob << " " << tot_pred_prob <<endl;

	return tot_error;
}

double graph::predict_prob(unordered_map<int, bool> observed_var_val, vector<int> query_var, vector<bool> query_val)
{
	vector<bool> assignment;
	bool sum_this = true;
	bool sum_num = true;
	double den_factor = 0.0000000000001;
	double num_factor = 0.0000000000001;
	for(int i=0; i<int(pow(2,no_nodes)); i++)
	{
		sum_this = true;
		assignment = h.get_assignment_from_integer(i, no_nodes);
		for(auto i=observed_var_val.begin(); i!=observed_var_val.end(); i++)
			if(assignment[i->first]!=i->second)
			{
				sum_this=false;
				break;
			}

		if(sum_this)
		{	
			double prob = get_joint_probability(assignment);
			den_factor+=prob;
			sum_num = true;
			for(int j=0; j<query_var.size(); j++)
			{
				if(assignment[query_var[j]] != query_val[j])
					sum_num = false;
			}

			if(sum_num)
			{
				num_factor+=prob;
			}
			
		}

	}

	return num_factor/den_factor;
}

vector<int> graph::generate_samples(int numSamples)
{
	//find topological order
	stack<int> topolog = topologicalSort();
	vector<int> assignments;

	default_random_engine gen(time(0));

	for(int i =0; i<numSamples; i++)
	{
		stack<int> topological = topolog;

		//creating a default sample
		vector<bool> assignment(no_nodes, false);

		//calculate samples according to topological order
		while(!topological.empty())
		{
			int cur_node = topological.top();
			topological.pop();
			vector<bool> perm{false, true};

			if(node_info[cur_node].no_parents==0)
			{	
				vector<double> p{node_info[cur_node].cpd[0], node_info[cur_node].cpd[1]};
				vector<bool> sample = h.ProbSampleReplace(p, perm, 1, gen);
				assignment[cur_node] = sample[0];
			}
			else
			{
				
				vector<bool> a;
				a.push_back(false);

				for(auto it = (*parents)[cur_node].begin(); it!=(*parents)[cur_node].end(); it++)
					a.push_back(assignment[*it]);
				int index = h.get_integer_from_assignment(a);
				vector<double> p{node_info[cur_node].cpd[index], node_info[cur_node].cpd[index+1]};
				vector<bool> sample = h.ProbSampleReplace(p,perm, 1, gen);
				assignment[cur_node] = sample[0];
			}


		}

		assignments.push_back(h.get_integer_from_assignment(assignment));
	}
	return assignments;
}


// void graph::sum_product_eliminate_var(vector<factor> f, int cur_node)
// {	
// 	vector<factor> cur_node_factor;
// 	vector<factor> rem_factor;
// 	bool present = false;
// 	for(int i=0; i<f.size(); i++)
// 	{
// 		present = false;
// 		for(int k=0; k<f[i].scope.size(); k++)
// 		{
// 			if(cur_node==f[i].scope[k])
// 				{
// 					cur_node_factor.push_back(f[i]);
// 					present = true;
// 					break;
// 				}

// 		}

// 		if(!present)
// 			rem_factor.push_back(f[i]);
// 	}

	

// }

//function to display graph contents
void graph::display()
{
	cout << "DISPLAY GRAPHICAL MODEL PARAMETERS " <<endl;
	cout << "-------------------------------------"<<endl;
	cout << "-------------------------------------"<<endl;
	//display adj
	cout << "ADJ LIST" <<endl;
	for(int i=0; i<no_nodes; i++)
	{
		cout << i << " :" <<endl;
		for(auto j=adj[i].begin(); j!=adj[i].end(); j++)
		{
			cout << *j << "->";
		}
		cout<<endl;
	}

	// display parents
	cout << endl;
	cout << "PARENTS "<<endl;
	for(auto i=(*parents).begin(); i!=(*parents).end(); i++)
	{
		cout << i->first << "  :";
		for(auto j=(*parents)[i->first].begin(); j!=(*parents)[i->first].end(); j++)
			cout << *j << " ";
		cout <<endl;
	}

	cout<<endl;

	//display random data
	cout << "NODE INFO" <<endl;
	cout <<	"FOR NODE 0" << endl;
	cout << "No of parents : ";
	cout << node_info[0].no_parents << endl;
	cout << "P(node0 =F)  P(node0 =T)" <<endl;
	for(int i=0; i<2; i++)
	{
		cout << node_info[0].cpd[i] << " ";
	}
	cout <<endl;
	cout <<endl;

	for(int which_node=1; which_node <no_nodes; which_node++)
	{
		cout <<"FOR NODE : " << which_node <<endl;
		cout <<"No of parents: " << node_info[which_node].no_parents <<endl;

		cout << "----------------------------------------------" <<endl;

		cout << which_node << " | ";
		for(auto it = (*parents)[which_node].begin(); it!=(*parents)[which_node].end(); it++)
		{
			cout << *it << " ";
		}
		cout <<"| Probability"<< endl;
		cout << "----------------------------------------------" <<endl;
		int index= 0;
		for(int i=0; i<int(pow(2,node_info[which_node].no_parents)); i++)
		{
			vector<bool> parent_assignment = h.get_assignment_from_integer(i, node_info[which_node].no_parents);
			cout << "0 | ";
			for(int cur_parent=0; cur_parent<parent_assignment.size(); cur_parent++)
				cout << parent_assignment[cur_parent] << " ";
			cout << "| "<< node_info[which_node].cpd[index] <<endl;
			index++;
			cout << "1 | ";
			for(int cur_parent=0; cur_parent<parent_assignment.size(); cur_parent++)
				cout << parent_assignment[cur_parent] << " ";
			cout << "| " <<node_info[which_node].cpd[index] <<endl;
			index++;
		}
		cout <<"----------------------------------------------"<<endl;
	}
}
