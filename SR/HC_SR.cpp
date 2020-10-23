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
//using namespace concurrency;
//using namespace concurrency::fast_math;
// operatorset: plus, minus, multiple, divide, sin, cos
const char operators[] = { '+','-','*','/', 's','c' };
// terminal set: value, x, T (for sin and cos)
const string terminals[] = { "val", "x", "T" };
#define MAXDEPTH 7
#define TRAIN_SIZE 700
#define VALID_SIZE 300





// global variables
double x_o[1000];
double y_o[1000];
double x_train[TRAIN_SIZE];
double y_train[TRAIN_SIZE];
double y_t[TRAIN_SIZE];
double x_valid[VALID_SIZE];
double y_valid[VALID_SIZE];
double y_v[VALID_SIZE];


//double x_valid[200];
//double y_valid[200];

vector<string> myFunction;
int depth;
int totalDepth;
int length;
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

// choose an operator (0, 1, 2, 3, 4, 5)
int randomO() {

	vector<int> foo;
	for (int i = 0; i < 6; i++) {
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
// choose terminals (0,1)
int randomT() {

	vector<int> foo;
	for (int i = 0; i < 4; i++) {
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
// choose operator or terminals (0,1)
int randomChoose() {

	vector<int> foo;
	for (int i = 0; i < 8; i++) {
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
int functionRandom(int index);
int functionEncoder(int o, int index);
int terminalEncoder(int v, int  index);
double errGet();
double valGet();
double DecodeTrain(double data, int id);
double DecodeValid(double data, int id);
void myClean();
int myMutation(int length);
void showFunction();

int main() {
	string filename = "SR_div_1000.txt";
	fileReader(filename);
	ofstream output;
	int count;
	int count_finish;
	int cur_index;
	int ite_round = 120; // 1000
	int count_round = 500; // 500
	int flg = 0;
	int Bestlength = 0;
	double err;
	double val;
	double improve;
	vector<string> BestFunction;
	double Bestresult = DBL_MAX;
	vector<double> error_record;
	vector<double> valid_record;
	//cout << original[0].x << " " << original[0].y<< endl;
	//cout << operators[0] << endl;
	for (int run = 3; run < 4; run++) {
		std::cout << "============================start a new function " << run << "=========================" << endl;
		myInit();
		count_finish = 0;
		flg = 0;
		length = functionRandom(0);
		myClean();
		std::cout << "first time random generate, length: " << length << endl << endl;
		showFunction();

		myFunction.resize(length);
		auto trainError = std::async(errGet);
		auto valError = std::async(valGet);
		err = trainError.get();

		val = valError.get();

		Bestresult = err;
		BestFunction = myFunction;
		Bestlength = length;
		for (int ite = 0; ite < ite_round; ite++) { //1000
			count = 0;

			if (flg == 1) {
				break;
			}
			error_record.push_back(Bestresult);
			valid_record.push_back(val);
			std::cout << endl << "******************** run = " << run << " ite = " << ite << endl;
			while (count < count_round) { // 500
				std::cout << "$$$$$$$$$$ " << count << endl;
				myFunction = BestFunction;
				length = Bestlength;
				// Generat a random function
				// encoding into an vector of string
				length = myMutation(length);
				/*showFunction();*/
				myClean();
				showFunction();
				std::cout << endl << endl;

				// decoding it to a function
				// input x value and get the value through this function

				auto trainError = std::async(errGet);
				auto valError = std::async(valGet);
				err = trainError.get();
				error_record.push_back(err);
				val = valError.get();
				valid_record.push_back(val);

				cur_index = count + 1;
				improve = abs(error_record[cur_index] - error_record[cur_index - 1]);
				if (err > Bestresult || improve < 0.0001 || count == 0) {
					count_finish++;
				} else {
					count_finish = 0;
				}
				if (err < Bestresult) {
					Bestresult = err;
					BestFunction = myFunction;
				}
				if (count_finish > 600) {
					flg = 1;
					break;
				}
				count++;

			}

			output.open("SR_HC_error_" + to_string(run) + ".txt", fstream::app);
			for (double i : error_record) {
				output << i << endl;
			}
			output.close();
			error_record.clear();

			output.open("SR_HC_validation_" + to_string(run) + ".txt", fstream::app);
			for (double i : valid_record) {
				output << i << endl;
			}
			output.close();
			valid_record.clear();

			output.open("SR_HC_Function_" + to_string(run) + ".txt");
			for (string i : BestFunction) {
				output << i << ",";
			}
			output.close();
		}
		// record the error, validation error for each time
	}

	// record the temperary fittest function


	return 0;
}


void showFunction() {
	int d = 1;
	std::cout << "++++++++Start Function++++++++ " << length << endl;
	for (int i = 0; i < length; i++) {
		if (myFunction[i] == "") {
			std::cout << "N" << " ";
		} else {
			std::cout << myFunction[i] << " ";
		}


		if (i == (pow(2, d) - 2)) {
			std::cout << endl;
			d++;
		}
	}
	std::cout << endl;
	std::cout << "++++++++End Function++++++++" << endl;
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
	depth = 0;
	totalDepth = 0;
	length = 0;
	myFunction.resize(3);
	myFunction.clear();

	std::fill(y_t, y_t + TRAIN_SIZE, 0);
	std::fill(y_v, y_v + VALID_SIZE, 0);
	/*for (int i = 0; i < TRAIN_SIZE; i++) {
		y_t[i] = 0;
	}
	for (int i = 0; i < VALID_SIZE; i++) {
		y_v[i] = 0;
	}*/

}

int functionEncoder(int o, int index) {

	myFunction[index] = operators[o];

	if (o == 4 || o == 5) {
		// add a 'T' to take this place
		for (int i = 0; i < (pow(2, depth + 1)); i++) {
			myFunction.push_back("");
		}
		myFunction[index * 2 + 2] = terminals[2];
		return index * 2 + 2;
	}
	return index;
}

int terminalEncoder(int v, int  index) {
	// add a number [0, 10], or x
	if (v == 0) {
		// add a x
		myFunction[index] = terminals[1];
	} else {
		myFunction[index] = to_string(randomNum());
	}
	return index;
}

void expand() {
	if (depth > totalDepth) {
		totalDepth = depth;
		if (depth < (MAXDEPTH - 1)) {
			// add two more depth
			for (int i = length; i < (pow(2, depth) - 1); i++) {
				myFunction.push_back("");
				length++;
			}
		} else {
			for (int i = length; i < (pow(2, depth) - 1); i++) {
				myFunction.push_back("");
				length++;
			}
		}
		return;
	}
	if (totalDepth == depth && length < (pow(2, totalDepth) - 1)) {
		for (int i = length; i < pow(2, totalDepth) - 1; i++) {
			myFunction.push_back("");
			length++;
		}
		return;
	}
}

int functionRandom(int index) {
	//std::cout << "index is" << index << endl;
	srand(time(NULL));
	int o;
	int v;
	depth++;
	int end = length;
	int t;
	//std::cout << "temp depth is " << depth << endl;
	expand();
	// if this node is filled with element
	if (myFunction[index] == "T") {
		depth--;
		end = index + 1;
		return end;
	}

	if ((randomChoose() < 5 || depth == 1) && depth < MAXDEPTH) {
		// add an operator
		o = randomO();
		//std::cout << "??? it is an operator" << operators[o];
		// deal with the self level
		t = functionEncoder(o, index);
		if (t > end) end = t;
		// go to its left child
		t = functionRandom(index * 2 + 1);
		if (t > end) end = t;
		// go to its right child
		t = functionRandom(index * 2 + 2);
		if (t > end) end = t;
		depth--;
		return end;
	} else {
		// add a value or an 'x'
		//std::cout << "??? it is a value or x"<<endl;
		v = randomT();
		t = terminalEncoder(v, index);
		if (t > end) end = t;
		depth--;
		return end;

	}

}

double errGet() {
	double e_train[TRAIN_SIZE];
	double err = 0;
	for (int i = 0; i < TRAIN_SIZE; i++) {
		y_t[i] = DecodeTrain(x_train[i], 0);
		e_train[i] = abs(y_train[i] - y_t[i]);
		err += e_train[i];
		//std::cout << y_t[i] << " ";
	}
	err = abs(err) / TRAIN_SIZE;
	return err;
}

double valGet() {
	double e_valid[VALID_SIZE];
	double val = 0;
	double e;
	for (int i = 0; i < VALID_SIZE; i++) {
		y_v[i] = DecodeTrain(x_valid[i], 0);
		e = abs(y_valid[i] - y_v[i]) / 700;
		e_valid[i] = pow(e, 2);
		val += e_valid[i];
	}
	val = sqrt(e);
	return val;
}

double DecodeTrain(double data, int id) {
	// input is the x_train, myFunction

	double tmp1;
	double tmp2;


	string k = myFunction[id];
	//std::cout << "this is the "<< k << " at position i= "<<id<< endl;
	if (k == "+") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp2 = DecodeTrain(data, id * 2 + 2);

		return tmp1 + tmp2;
	} else if (k == "-") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp2 = DecodeTrain(data, id * 2 + 2);

		return tmp1 - tmp2;
	} else if (k == "*") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp2 = DecodeTrain(data, id * 2 + 2);

		return tmp1 * tmp2;
	} else if (k == "/") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp2 = DecodeTrain(data, id * 2 + 2);
		if (tmp2 != 0) {
			tmp1 = tmp1 / tmp2;
		} else {
			tmp1 = 0.0;
		}

		return tmp1;
	} else if (k == "s") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp1 = sin(tmp1);
		return tmp1;
	} else if (k == "c") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp1 = cos(tmp1);
		return tmp1;
	} else if (k == "x") {
		return data;
	} else {// k is a number
		tmp1 = stof(k);
		return tmp1;
	}

	return data;
}

double DecodeValid(double data, int id) {
	// input is the x_train, myFunction

	double tmp1;
	double tmp2;


	string k = myFunction[id];
	//std::cout << "this is the "<< k << " at position i= "<<id<< endl;
	if (k == "+") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp2 = DecodeTrain(data, id * 2 + 2);

		return tmp1 + tmp2;
	} else if (k == "-") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp2 = DecodeTrain(data, id * 2 + 2);

		return tmp1 - tmp2;
	} else if (k == "*") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp2 = DecodeTrain(data, id * 2 + 2);

		return tmp1 * tmp2;
	} else if (k == "/") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp2 = DecodeTrain(data, id * 2 + 2);
		if (tmp1 != 0) {
			tmp1 = tmp1 / tmp2;
		} else if (tmp1 == 0 && tmp2 > 0) {
			tmp1 = 100;
		} else if (tmp1 == 0 && tmp2 < 0) {
			tmp1 = -100;
		} else if (tmp1 == 0 && tmp2 == 0) {
			tmp1 = 0.0;
		}
		return tmp1;
	} else if (k == "s") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp1 = sin(tmp1);
		return tmp1;
	} else if (k == "c") {
		tmp1 = DecodeTrain(data, id * 2 + 1);
		tmp1 = cos(tmp1);
		return tmp1;
	} else if (k == "x") {
		return data;
	} else {// k is a number
		tmp1 = stof(k);
		return tmp1;
	}

	return data;
}

void myClean() {
	//std::cout << "clean!!" << endl;
	int parent_i;
	if (myFunction[0] == "") {
		length = 0;
		return;
	}

	for (int i = 1; i < length; i++) {

		if (i % 2 == 1) {
			// odd number
			parent_i = (i - 1) / 2; // parent
			if (myFunction[parent_i] == "" && myFunction[i] != "") {
				myFunction[i] = "";
			}
		} else {
			//even number
			parent_i = (i - 2) / 2;
			if (myFunction[parent_i] == "" && myFunction[i] != "") {
				myFunction[i] = "";
			}
		}
	}
	while (myFunction[length - 1] == "") {
		length--;
	}
}

int deleteTree(int index) {

	int tmpl = length;
	//std::cout << "temporary length is:" << tmpl << endl;
	depth = ceil(log2(index + 2));
	//std::cout << "modified node depth " << depth << endl;
	if (myFunction[index] == "+" || myFunction[index] == "-" || myFunction[index] == "*" || myFunction[index] == "/") {
		tmpl = deleteTree(index * 2 + 1);

		tmpl = deleteTree(index * 2 + 2);
		myFunction[index] = "";

	} else if (myFunction[index] == "s" || myFunction[index] == "c") {
		myFunction[2 * index + 2] = "";
		tmpl = deleteTree(index * 2 + 1);
		myFunction[index] = "";
	} else {
		// if it is a number
		myFunction[index] = "0";
	}

	if (tmpl < length) {
		length = tmpl;
	}
	if (length != 0 && index == length - 1) {
		while (myFunction[length - 1] == "") {
			length--;
		}
	}
	myClean();

	totalDepth = ceil(log2(length + 1));
	return length;
}




int myMutation(int length) {
	// choose a node
	int node_index = randomValue(length);
	string newOperator;
	int num;
	int o;
	int tmp;
	std::cout << "node_index is " << node_index << endl;
	std::cout << myFunction[node_index];
	while (node_index == 0 || myFunction[node_index] == "" || myFunction[node_index] == "T") {
		node_index = randomValue(length);
	}
	// decide modify this node or delete it
	int decision = randomValue(9);
	int level = ceil(log2(node_index + 2));
	// 1. modify to an operator
	if (decision <= 6 && level < MAXDEPTH - 1) {

		o = randomO();
		depth = ceil(log2(node_index + 2));
		if (depth == totalDepth) {
			totalDepth++;
		}
		std::cout << "1. modify [" << myFunction[node_index] << "] to an operater in:" << node_index << endl;
		std::cout << "depth here is: " << depth << " || total depth is " << totalDepth << endl;
		std::cout << "new operator is " << operators[o] << endl;
		// 1.1
		if ((myFunction[node_index] == "+" || myFunction[node_index] == "-" || myFunction[node_index] == "*" || myFunction[node_index] == "/")) {
			std::cout << "1.1 " << endl;
			if (operators[o] != 's' && operators[o] != 'c') {
				// change between + - * /
				myFunction[node_index] = operators[o];
				depth = ceil(log2(length + 1)) - 1;
				return length;
			} else {
				// new operator is sin, cos
				// turn the right subtree to "T"
				myFunction[node_index] = operators[o];
				int right_child = node_index * 2 + 2;
				length = deleteTree(right_child);
				if (right_child >= length) {
					for (int i = length; i <= right_child + 1; i++) {
						myFunction.push_back("");
					}
					length = right_child + 1;
				}
				myFunction[right_child] = "T";
				depth = ceil(log2(length + 1)) - 1;
				return length;
			}
		}
		// 1.2
		else if (myFunction[node_index] == "s" || myFunction[node_index] == "c") {
			std::cout << "1.2 " << endl;
			if (operators[o] == 's' || operators[o] == 'c') {
				// change between s c
				myFunction[node_index] = operators[o];
				depth = ceil(log2(length + 1)) - 1;
				myClean();
				return length;
			} else {
				// change to + - * / 

				myFunction[node_index] = operators[o];
				myFunction[node_index * 2 + 2] = "";

				depth = ceil(log2(node_index + 2));
				tmp = functionRandom(node_index * 2 + 2);
				if (tmp > length) length = tmp;
				myClean();
				depth = ceil(log2(length + 1));
				return length;
			}

		}
		// 1.3
		else {
			// number or x to operator
			std::cout << "1.3 " << endl;

			myFunction[node_index] = operators[o];
			/*std::cout << " tmp length when value to operator: " << length << endl;*/
			if (operators[o] == 's' || operators[o] == 'c') {
				std::cout << "1.3.1 " << endl;
				int right_child = node_index * 2 + 2;
				if (right_child >= length) {
					for (int i = length; i <= right_child; i++) {
						myFunction.push_back("");
					}
					length = right_child + 1;
				}
				expand();
				myFunction[right_child] = "T";
				std::cout << "add the T successfully" << endl;
				// go to the left child

				tmp = functionRandom(node_index * 2 + 1);
				if (tmp > length) length = tmp;
				myClean();
				depth = ceil(log2(length + 1)) - 1;
				return length;
			} else {
				std::cout << "1.3.2 " << endl;
				// + - * /
				int tmpl;
				expand();
				depth = ceil(log2(node_index + 2));
				totalDepth = ceil(log2(length + 1));
				tmpl = functionRandom(node_index * 2 + 1);
				tmpl = functionRandom(node_index * 2 + 2);
				if (tmpl > length) {
					length = tmpl + 1;
				}
				myClean();
				depth = ceil(log2(length + 1));
				return length;
			}

		}

	}
	// 2. modify to a value or an x
	else if (decision <= 8 || level >= MAXDEPTH - 1) {
		std::cout << "2. modify [" << myFunction[node_index] << "} to a value " << node_index << endl;
		num = randomValue(2);   // determine x or a number will be put in

		length = deleteTree(node_index);
		if (node_index >= length) {
			for (int i = length; i <= node_index; i++) {
				myFunction.push_back("");
			}
			length = node_index + 1;
		}

		if (num == 0) {
			// value
			myFunction[node_index] = to_string(randomNum());
			std::cout << "new value is " << myFunction[node_index] << endl;
		} else {
			// x
			myFunction[node_index] = "x";
			std::cout << "new value is \"x\"" << endl;
		}



	}
	// delete this subtree
	else {
		std::cout << "simply delete " << node_index << endl;
		length = deleteTree(node_index);
	}

	return length;
}