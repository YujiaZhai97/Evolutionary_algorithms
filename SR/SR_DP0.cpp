#include<stdio.h>
#include<iostream>
#include<fstream>
#include<cstring>
#include<cmath>
#include<math.h>
#include<limits>
#include<string>
#include<sstream>
#include<array>
#include<deque>
#include<time.h>
#include<ctime>
#include<vector>
#include<set>
#include<stdlib.h>
#include  <iterator>
#include  <random>
#include<thread>
#include<future>
//#include <amp.h> 
//#include <amp_math.h> 


//layer:   #ele    # total
//	1       1			1
//	2		2			3
//	3		4			7
//	4		8			15
//	5		16			31
//	6		32			63
//	7		64			127
//	8		128			255
//	9		256			511

using namespace std;

// operatorset: plus, minus, multiple, divide, sin, cos
const string operators[] = { "+","-","*","/", "s","c" };
// terminal set: value, x, T (for sin and cos)
const string terminals[] = { "val", "x", "T" };

#define TRAIN_SIZE 700
#define VALID_SIZE 300

struct myFunction {
	vector<string> function;
	int depth = 0;
	int totalDepth = 0;
	int length = 0;
	int maxdepth = 0;   // [5,9]

	double error = 0.0;
	double valid = 0.0;

};

// global variables

const int popsize = 30;    // 30
float matrx[5][5] = { {0.1, 0.3, 0.3, 0.5, 1.0},
					  {0.3, 0.1, 0.3, 0.5, 1.0},
					  {0.3, 0.3, 0.1, 0.5, 1.0},
					  {0.5, 0.5, 0.5, 0.1, 1.0},
					  {1.0, 1.0, 1.0, 1.0, 0.1} };
int pairs = popsize * (popsize - 1) / 2;

double x_o[1000];
double y_o[1000];
double x_train[TRAIN_SIZE];
double y_train[TRAIN_SIZE];
double y_t[TRAIN_SIZE];
double x_valid[VALID_SIZE];
double y_valid[VALID_SIZE];
double y_v[VALID_SIZE];

vector<myFunction> myPopulation;
myFunction Bestresult;

template<typename Iter, typename RandomGenerator>
Iter select_randomly(Iter start, Iter end, RandomGenerator& g) {
	std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
	std::advance(start, dis(g));
	return start;
}

template<typename Iter>
Iter select_randomly(Iter start, Iter end) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	return select_randomly(start, end, gen);
}

int randomValue(int b) {

	vector<int> foo;
	for (int i = 0; i < b; i++) {
		foo.push_back(i);
	}

	vector<int> num;
	set<int> used;
	while (num.size() < 1) {
		int r = *select_randomly(foo.begin(), foo.end());
		if (used.count(r) == 0) {
			num.push_back(r);
			used.insert(r);
		}
	}
	return num[0];
}

// get a random number in (-10, 10)
double randomNum() {

	vector<int> foo;
	for (int i = 0; i < 200; i++) {
		foo.push_back(i);
	}

	vector<int> num;
	set<int> used;
	while (num.size() < 1) {
		int r = *select_randomly(foo.begin(), foo.end());
		if (used.count(r) == 0) {
			num.push_back(r);
			used.insert(r);
		}
	}
	return (num[0] - 100) / 20.0;
}

set<int> randomTrain() {

	vector<int> foo;
	for (int i = 0; i < 1000; i++) {
		foo.push_back(i);
	}

	set<int> num;
	set<int> used;
	while (num.size() < 700) {
		int r = *select_randomly(foo.begin(), foo.end());
		if (used.count(r) == 0) {
			num.insert(r);
			used.insert(r);
		}
	}
	return num;
}

void fileReader(string filename);
void myInit();
void myCleanTail(int order);
//void myClean();
void showFunction(int order);

void functionRandom(int order, int index);
double errGet(int order);
double valGet(int order);
void mySelection();
float myDistance();
void myCrossover(int k);
void myMutation(int order);


int main() {
	string filename = "SR_div_1000.txt";
	fileReader(filename);
	ofstream output;

	double err;
	double val;
	vector<double> error_record;
	vector<double> valid_record;
	vector<float> distance_record;
	float dist;
	int ite_round = 100;
	int count_round = 500;
	int cut = popsize * 0.6;
	int cross_num = ((popsize - 2) - cut) / 2;

	for (int run = 1; run < 2; run++) {
		myInit();
		Bestresult.error = DBL_MAX;
		std::cout << myPopulation.size() << endl;

		for (int i = 0; i < popsize; i++) {
			myPopulation[i].maxdepth = randomValue(5) + 5;
			functionRandom(i, 0);
			myCleanTail(i);

			auto trainError = std::async(errGet, i);
			auto valError = std::async(valGet, i);
			err = trainError.get();
			val = valError.get();
			myPopulation[i].error = err;
			myPopulation[i].valid = val;

			if (err < Bestresult.error) {
				Bestresult = myPopulation[i];
			}
		}
		error_record.push_back(Bestresult.error);
		valid_record.push_back(Bestresult.valid);
		dist = myDistance();
		//std::cout <<"this distance is "<< dist << endl;
		distance_record.push_back(dist);

		for (int ite = 0; ite < ite_round; ite++) {

			for (int count = 0; count < count_round; count++) {
				std::cout << "run " << run << " ite " << ite << " count " << count << endl;

				// 1. selection: select top 60% individuals according to error
				mySelection();

				// 2. crossover: generate new 40% individuals

				for (int i = 0; i < cross_num; i++) {
					myCrossover(i);
					myPopulation[i].maxdepth = 9;
				}
				for (int i = popsize - 2; i < popsize; i++) {
					myPopulation[i].function.clear();
					myPopulation[i].function.resize(3);
					myPopulation[i].length = 3;
					myPopulation[i].totalDepth = 2;
					myPopulation[i].maxdepth = randomValue(5) + 5;
					functionRandom(i, 0);
					myPopulation[i].maxdepth = 9;
					myCleanTail(i);
					myPopulation[i].totalDepth = ceil(log2(myPopulation[i].length + 1));
				}


				// 3. mutation:
				for (int i = 0; i < popsize - 2; i++) {
					if (randomValue(10) == 8) {
						myMutation(i);
					}
				}
				//std::cout<< "finish mutation" << endl;
				/*for (int i = 0; i < popsize; i++) {
					std::cout << i << "th individual" << endl;
					showFunction(i);
				}*/

				// 4. Diversity
				dist = myDistance();
				distance_record.push_back(dist);

				// 5. Update
				for (int i = 0; i < popsize; i++) {
					//std::cout << i << endl;
					auto trainError = std::async(errGet, i);
					auto valError = std::async(valGet, i);
					err = trainError.get();
					val = valError.get();
					myPopulation[i].error = err;
					myPopulation[i].valid = val;

					myPopulation[i].length = myPopulation[i].function.size();
					myPopulation[i].totalDepth = ceil(log2(myPopulation[i].length + 1));
					if (err < Bestresult.error) {
						Bestresult = myPopulation[i];
					}
				}
				error_record.push_back(Bestresult.error);
				valid_record.push_back(Bestresult.valid);
			}

			output.open("SR_GP_error_run_" + to_string(run) + ".txt", fstream::app);
			for (double i : error_record) {
				output << i << endl;
			}
			output.close();
			error_record.clear();

			output.open("SR_GP_valid_run_" + to_string(run) + ".txt", fstream::app);
			for (double i : valid_record) {
				output << i << endl;
			}
			output.close();
			valid_record.clear();

			output.open("SR_GP_distance_run_" + to_string(run) + ".txt", fstream::app);
			for (double i : distance_record) {
				output << i << endl;
			}
			output.close();
			distance_record.clear();

			output.open("SR_GP_Function_run_" + to_string(run) + ".txt");
			for (string i : Bestresult.function) {
				output << i << ",";
			}
			output.close();

		}
	}
	return 0;
}

void fileReader(string filename) {
	ifstream inf;
	inf.open(filename);
	string sline;
	string sx, sy;
	int count = 0;


	while (getline(inf, sline)) {
		istringstream sin(sline);
		sin >> sx >> sy;

		x_o[count] = atof(sx.c_str());
		y_o[count] = atof(sy.c_str());
		count++;
	}
	int m = 0, n = 0;

	for (int i = 0; i < 1000; i++) {
		if (i % 10 == 2 || i % 10 == 5 || i % 10 == 8) {
			x_valid[m] = x_o[i];
			y_valid[m] = y_o[i];
			m++;
		} else {
			x_train[n] = x_o[i];
			y_train[n] = y_o[i];
			n++;
		}
	}

	//cout << count << endl;
}

void myInit() {
	myPopulation.clear();
	//std::cout << myPopulation.size() << endl;
	for (int i = 0; i < popsize; i++) {
		myFunction newFunction;
		newFunction.function.clear();
		newFunction.function.resize(3);
		newFunction.totalDepth = 2;

		myPopulation.push_back(newFunction);
		//std::cout << myPopulation.size() << endl;
	}

	std::fill(y_t, y_t + TRAIN_SIZE, 0);
	std::fill(y_v, y_v + VALID_SIZE, 0);

}

void expand(int order) {
	int l = myPopulation[order].length;
	int d = myPopulation[order].depth;

	if (myPopulation[order].totalDepth < myPopulation[order].depth) {
		myPopulation[order].totalDepth = myPopulation[order].depth;
		if (d < (myPopulation[order].maxdepth - 1)) {
			for (int i = l; i < (pow(2, d + 1) - 1); i++) {
				myPopulation[order].function.push_back("");
				myPopulation[order].length++;
			}
		} else {
			for (int i = l; i < (pow(2, d) - 1); i++) {
				myPopulation[order].function.push_back("");
				myPopulation[order].length++;
			}
		}
		return;
	}
	// add more empty place in the deepest level
	if (myPopulation[order].depth == myPopulation[order].maxdepth && myPopulation[order].length < (pow(2, myPopulation[order].maxdepth) - 1)) {
		for (int i = l; i < (pow(2, myPopulation[order].maxdepth) - 1); i++) {
			myPopulation[order].function.push_back("");
			myPopulation[order].length++;
		}
		return;
	}
	if (myPopulation[order].depth == myPopulation[order].totalDepth && myPopulation[order].length < (pow(2, myPopulation[order].totalDepth) - 1)) {
		for (int i = l; i < (pow(2, myPopulation[order].totalDepth) - 1); i++) {
			myPopulation[order].function.push_back("");
			myPopulation[order].length++;
		}
		return;
	}
}

void myCleanTail(int order) {
	int l = myPopulation[order].function.size();
	while (myPopulation[order].function[l - 1] == "") {
		l--;
	}
	myPopulation[order].function.resize(l);
	myPopulation[order].length = myPopulation[order].function.size();
}

void showFunction(int order) {
	int level = 1;
	std::cout << "++++++++Start Function++++++++ maxdepth " << myPopulation[order].maxdepth << " length " << myPopulation[order].length;
	std::cout << " total depth " << myPopulation[order].totalDepth << endl;
	for (int i = 0; i < myPopulation[order].function.size(); i++) {
		if (myPopulation[order].function[i] == "") {
			std::cout << "N ";
		} else {
			std::cout << myPopulation[order].function[i] << " ";
		}
		if (i == pow(2, level) - 2) {
			std::cout << endl;
			level++;
		}
	}
	std::cout << endl;
	std::cout << "++++++++End Function++++++++" << endl;
}

void functionEncoder(int order, int index) {
	int o = randomValue(6);
	myPopulation[order].function[index] = operators[o];

	if (o == 4 || o == 5) {
		myPopulation[order].depth++;
		expand(order);
		myPopulation[order].function[index * 2 + 2] = "T";
		myPopulation[order].depth--;
	}

}

void terminalEncoder(int order, int index) {
	if (randomValue(3) == 0) {
		// add x
		myPopulation[order].function[index] = "x";
	} else {
		// add a value [-10, 10]
		myPopulation[order].function[index] = to_string(randomNum());
	}
	return;
}

void functionRandom(int order, int index) {
	// operate on myPopulation[order] 
	srand(time(NULL));

	myPopulation[order].depth++;

	expand(order);

	if (myPopulation[order].function[index] == "T") {
		myPopulation[order].depth--;
		if (myPopulation[order].length < index + 1) myPopulation[order].length = index + 1;
		return;
	}
	// 1. add an operator
	if ((randomValue(8) < 5 || myPopulation[order].depth == 1) &&
		myPopulation[order].depth < myPopulation[order].maxdepth - 1) {
		// current node
		functionEncoder(order, index);
		// left child
		functionRandom(order, index * 2 + 1);
		// right child
		functionRandom(order, index * 2 + 2);
	}
	// 2. add a value/x
	else {
		terminalEncoder(order, index);
	}


	myPopulation[order].depth--;
	return;
}

double decodeTrain(int order, int index, double x) {

	double tmp1;
	double tmp2;
	string k = myPopulation[order].function[index];

	if (k == "+") {
		tmp1 = decodeTrain(order, index * 2 + 1, x);
		tmp2 = decodeTrain(order, index * 2 + 2, x);
		return tmp1 + tmp2;
	} else if (k == "-") {
		tmp1 = decodeTrain(order, index * 2 + 1, x);
		tmp2 = decodeTrain(order, index * 2 + 2, x);
		return tmp1 - tmp2;
	} else if (k == "*") {
		tmp1 = decodeTrain(order, index * 2 + 1, x);
		tmp2 = decodeTrain(order, index * 2 + 2, x);
		return tmp1 * tmp2;
	} else if (k == "/") {
		tmp1 = decodeTrain(order, index * 2 + 1, x);
		tmp2 = decodeTrain(order, index * 2 + 2, x);
		if (tmp2 == 0) {
			return 0.0;
		}
		return tmp1 / tmp2;
	} else if (k == "s") {
		tmp1 = decodeTrain(order, index * 2 + 1, x);
		return sin(tmp1);
	} else if (k == "c") {
		tmp1 = decodeTrain(order, index * 2 + 1, x);
		return cos(tmp1);
	} else if (k == "x") {
		return x;
	} else {
		return stof(k);
	}
}

double errGet(int order) {
	double err = 0;
	double tmp;
	for (int i = 0; i < TRAIN_SIZE; i++) {
		tmp = decodeTrain(order, 0, x_train[i]);
		err += (abs(y_train[i] - tmp));
	}
	err = err / TRAIN_SIZE;
	return err;
}

double decodeVal(int order, int index, double x) {

	double tmp1;
	double tmp2;
	string k = myPopulation[order].function[index];

	if (k == "+") {
		tmp1 = decodeVal(order, index * 2 + 1, x);
		tmp2 = decodeVal(order, index * 2 + 2, x);
		return tmp1 + tmp2;
	} else if (k == "-") {
		tmp1 = decodeVal(order, index * 2 + 1, x);
		tmp2 = decodeVal(order, index * 2 + 2, x);
		return tmp1 - tmp2;
	} else if (k == "*") {
		tmp1 = decodeVal(order, index * 2 + 1, x);
		tmp2 = decodeVal(order, index * 2 + 2, x);
		return tmp1 * tmp2;
	} else if (k == "/") {
		tmp1 = decodeVal(order, index * 2 + 1, x);
		tmp2 = decodeVal(order, index * 2 + 2, x);
		if (tmp2 == 0) {
			return 0.0;
		}
		return tmp1 / tmp2;
	} else if (k == "s") {
		tmp1 = decodeVal(order, index * 2 + 1, x);
		return sin(tmp1);
	} else if (k == "c") {
		tmp1 = decodeVal(order, index * 2 + 1, x);
		return cos(tmp1);
	} else if (k == "x") {
		return x;
	} else {
		return stof(k);
	}
}

double valGet(int order) {
	double err = 0;
	double tmp;
	for (int i = 0; i < VALID_SIZE; i++) {
		tmp = decodeVal(order, 0, x_valid[i]);
		err += (abs(y_valid[i] - tmp));
	}
	err = err / VALID_SIZE;
	return err;
}

int getGroup(vector<string> function, int idx) {

	string k = function[idx];

	if (k == "+" || k == "-") {
		return 0;
	} else if (k == "*" || k == "/") {
		return 1;
	} else if (k == "s" || k == "c") {
		return 2;
	} else if (k == "" || k == "T") {
		return 4;
	}
	return 3;
}

void mySwap(int a, int b) {
	myFunction tmp;
	tmp = myPopulation[a];
	myPopulation[a] = myPopulation[b];
	myPopulation[b] = tmp;
}

void deleteTree(int order, int node_index) {

	myPopulation[order].function[node_index] = "";
	int nextr = node_index * 2 + 2;
	if (nextr < myPopulation[order].length && myPopulation[order].function[nextr] != "") {
		deleteTree(order, nextr - 1);
		deleteTree(order, nextr);
	}

}

void myExchange(int a, int b, int ct) {

	int len1 = myPopulation[a].length;
	int len2 = myPopulation[b].length;
	int subl = ct * 2 + 1;
	int subr = ct * 2 + 2;

	string tmp = myPopulation[a].function[ct];
	myPopulation[a].function[ct] = myPopulation[b].function[ct];
	myPopulation[b].function[ct] = tmp;

	if (len1 < subr) {
		return;
	} else {
		myExchange(a, b, subl);
		myExchange(a, b, subr);
	}
}

// 1. Selection
void mySelection() {

	Bestresult = myPopulation[0];
	double err = Bestresult.error;
	int best_idx = 0;

	double e1;
	double e2;
	int cut = popsize * 0.6;  // 30 * 0.6
	// 1.1 select the Best individual 
	// 1.2 and the 12 highest result are at the tail part
	for (int i = 0; i < cut; i++) {

		for (int j = cut; j < popsize; j++) {
			e1 = myPopulation[i].error;
			e2 = myPopulation[j].error;

			if (e1 > e2) {
				mySwap(i, j);
			}
			if (myPopulation[i].error < Bestresult.error) {
				Bestresult = myPopulation[i];
				best_idx = i;
			}
		}
	}
	// 1.3 Always put the best result at the top
	mySwap(0, best_idx);
}

// 4. Distance
float myDistance() {
	float dist = 0;
	vector<string> func1;
	vector<string> func2;
	int len1;
	int len2;
	int minlen;
	int m;
	int n;
	for (int a = 0; a < popsize; a++) {
		func1 = myPopulation[a].function;
		len1 = myPopulation[a].length;
		for (int b = a + 1; b < popsize; b++) {
			func2 = myPopulation[b].function;
			len2 = myPopulation[b].length;
			if (len1 <= len2) {
				minlen = len1;
			} else {
				minlen = len2;
			}
			dist += float(abs(len1 - len2));

			for (int i = 0; i < minlen; i++) {
				if (func1[i] != func2[i]) {
					m = getGroup(func1, i);
					n = getGroup(func2, i);
					dist += matrx[m][n];
				}
			}
		}
	}
	return dist / pairs;
}

// 2. Crossover
void myCrossover(int k) {
	//std::cout << "~~~~~~~~~~~here in the cross over!~~~~~~~~~~~~" << endl;

	// 2.0 inital and get 2 individuals
	int cut = popsize * 0.6;
	int num1 = randomValue(cut + 1) % cut;
	int num2 = randomValue(cut + 1) % cut;
	while (num1 == num2) { num2 = randomValue(cut + 1) % cut; }

	vector<string> func1 = myPopulation[num1].function;
	vector<string> func2 = myPopulation[num2].function;
	myFunction n1;
	myFunction n2;
	//showFunction(num1);
	//showFunction(num2);
	n1.function = func1;
	n2.function = func2;
	int len1 = myPopulation[num1].length;
	int len2 = myPopulation[num2].length;

	n1.length = len1;
	n2.length = len2;

	// 2.0.5 add them to the population
	int a = cut + k;
	int b = cut + k + 1;
	myPopulation[a] = n1;
	myPopulation[b] = n2;

	// 2.1 get cut point and the subtree 
	// note that the element in this point should not be "" or "T"
	int p1;
	if (len1 <= len2) {
		p1 = randomValue(len1);
		while (func1[p1] == "" || func1[p1] == "T" || func2[p1] == "" || func2[p1] == "T") {
			p1 = randomValue(len1);
		}
	} else {
		p1 = randomValue(len2);
		while (func1[p1] == "" || func1[p1] == "T" || func2[p1] == "" || func2[p1] == "T") {
			p1 = randomValue(len2);
		}
	}

	if (len1 < len2) {
		for (int i = len1; i < len2; i++) {
			myPopulation[a].function.push_back("");
			myPopulation[a].length += 1;
		}
	} else if (len2 < len1) {
		for (int i = len2; i < len1; i++) {
			myPopulation[b].function.push_back("");
			myPopulation[b].length += 1;
		}
	}

	// 2.2 exchang the subtree to generate 2 individuals

	myExchange(a, b, p1);

	myCleanTail(a);
	myCleanTail(b);
	myPopulation[a].totalDepth = ceil(log2(myPopulation[a].length + 1));
	myPopulation[b].totalDepth = ceil(log2(myPopulation[b].length + 1));
	//showFunction(a);
	//showFunction(b);
}

// 3. Mutation
void myMutation(int order) {
	int node_index = 0;
	string newO;
	string oldO;
	int num;
	int o;
	//std::cout << "3. Mutation "<<endl;
	// 3.0 choose a node for change
	while (node_index == 0 || myPopulation[order].function[node_index] == "" || myPopulation[order].function[node_index] == "T") {
		node_index = randomValue(myPopulation[order].length);
	}

	int decision = randomValue(9);
	int level = ceil(log2(node_index + 2));
	myPopulation[order].depth = level;
	expand(order);
	// 3.1.1 modify to an operator
	if (decision <= 6 && level < myPopulation[order].maxdepth - 1) {
		//std::cout << "3.1.1 modify [" << myPopulation[order].function[node_index] << "] to a ooperator at " << node_index;
		o = randomValue(6);
		newO = operators[o];
		oldO = myPopulation[order].function[node_index];
		//std::cout << " as " << operators[o] << endl;

		if (oldO == "+" || oldO == "-" ||
			oldO == "*" || oldO == "/") {
			if (o < 4) {
				// to + - * /
				myPopulation[order].function[node_index] = newO;

			} else {
				// to sine cosine
				int r = 2 * node_index + 2;
				myPopulation[order].function[node_index] = newO;
				deleteTree(order, r);
				myPopulation[order].function[r] = "T";

			}
		} else if (oldO == "s" || oldO == "c") {
			if (o >= 4) { // to sine, cosine
				myPopulation[order].function[node_index] = newO;
			}if (o < 4) { // to + - * /
				myPopulation[order].function[node_index] = newO;
				myPopulation[order].function[node_index * 2 + 2] = "";
				functionRandom(order, node_index * 2 + 2);
			}
		} else {
			// old is value/x
			myPopulation[order].function[node_index] = newO;
			functionRandom(order, node_index * 2 + 1);
			if (o < 4) {
				functionRandom(order, node_index * 2 + 2);
			}if (o >= 4) {
				myPopulation[order].function[node_index * 2 + 2] = "T";
			}

		}

	}
	// 3.1.2 modify to a value/x
	else if (decision <= 8 || level >= myPopulation[order].maxdepth - 1) {
		//std::cout << "3.1.2 modify {" << myPopulation[order].function[node_index] << "} to a value at " << node_index ;
		num = randomValue(2);

		deleteTree(order, node_index);

		if (num == 0) {
			// to an x
			myPopulation[order].function[node_index] = "x";
			//std::cout << " as x" << endl;
		} else {
			// to a value
			myPopulation[order].function[node_index] = to_string(randomNum());
			//std::cout << " as "<< myPopulation[order].function[node_index] << endl;
		}

	}
	myCleanTail(order);

}
