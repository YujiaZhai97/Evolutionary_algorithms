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


using namespace std;

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


int main() {
	string filename = "SR_div_1000.txt";
	fileReader(filename);
	ofstream output;
	int count;


	double err;
	double val;
	vector<string> BestFunction;
	double Bestresult = DBL_MAX;
	vector<double> error_record;
	vector<double> valid_record;
	//cout << original[0].x << " " << original[0].y<< endl;
	//cout << operators[0] << endl;
	for (int run = 2; run < 4; run++) { //1000
		for (int ite = 0; ite < 400; ite++) {
			count = 0;
			std::cout << "run = " << run << " ite = " << ite << endl;
			while (count < 500) { // 500
				//std::cout << "============================start a new function=========================" << endl;
				myInit();

				// Generat a random function
				// encoding into an vector of string
				length = functionRandom(0) + 1;
				myFunction.resize(length);

				// decoding it to a function
				// input x value and get the value through this function

				auto trainError = std::async(errGet);
				auto valError = std::async(valGet);
				err = trainError.get();
				error_record.push_back(err);
				//std::cout << "the total error is " << err << endl;

				//DecodeValid();

				val = valError.get();
				valid_record.push_back(val);
				//std::cout << "the validation error" << val << endl;

				// update the best result

				if (err < Bestresult) {
					Bestresult = err;
					BestFunction = myFunction;
				}

				//std::cout << endl;
				count++;

			}

			output.open("SR_RS_error_" + to_string(run) + ".txt", fstream::app);
			for (double i : error_record) {
				output << i << endl;
			}
			output.close();
			error_record.clear();

			output.open("SR_RS_validation_" + to_string(run) + ".txt", fstream::app);
			for (double i : valid_record) {
				output << i << endl;
			}
			output.close();
			valid_record.clear();

			output.open("SR_RS_Function_" + to_string(run) + ".txt");
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

int functionRandom(int index) {
	srand(time(NULL));
	int o;
	int v;
	depth++;
	int end = 0;
	int t;
	//std::cout << "temp depth is " << depth << endl;
	if (depth > totalDepth) {
		totalDepth = depth;
		if (depth < (MAXDEPTH - 1)) {
			// add two more depth
			for (int i = 0; i < (pow(2, depth)); i++) {
				myFunction.push_back("");
			}
		} else {
			for (int i = 0; i < pow(2, depth); i++) {
				myFunction.push_back("");
			}
		}
	}
	// if this node is filled with element
	if (!myFunction[index].empty()) {
		depth--;
		end = index;
		return end;
	}

	if ((randomChoose() < 5 || depth == 1) && depth < MAXDEPTH) {
		// add an operator
		o = randomO();

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


