#pragma once
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <thread>
#include<fstream>
#include<cstring>
#include<vector>
#include <cmath>
#include<math.h>
#include<limits>
#include<time.h>
#include<ctime>
#include<set>
#include<iterator>
#include<random>
#include<thread>
#include<future>
#include <math.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include "MyCubes.h"


extern std::vector<MyCubes> robotGroup;
extern GLdouble structure_base[];


void swaprobot(int a, int b);

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

int randomValueInt(int b) {

	std::vector<int> foo;
	for (int i = 0; i < b; i++) {
		foo.push_back(i);
	}

	std::vector<int> num;
	std::set<int> used;
	while (num.size() < 1) {
		int r = *select_randomly(foo.begin(), foo.end());
		if (used.count(r) == 0) {
			num.push_back(r);
			used.insert(r);
		}
	}
	return num[0];
}

std::vector<int> randomValuesInt(int largest, int size) {

	std::vector<int> foo;
	for (int i = 0; i < largest; i++) {
		foo.push_back(i);
	}

	std::vector<int> num;
	std::set<int> used;
	while (num.size() < size) {
		int r = *select_randomly(foo.begin(), foo.end());
		if (used.count(r) == 0) {
			num.push_back(r);
			used.insert(r);
		}
	}
	return num;
}

std::vector<double> randomGeneratorDouble(int largest, int size) {

	std::vector<double> foo;
	for (int i = 0; i < largest; i++) {
		foo.push_back(i);
	}

	std::vector<double> num;
	std::set<double> used;
	while (num.size() < size) {
		int r = *select_randomly(foo.begin(), foo.end());
		if (used.count(r) == 0) {
			num.push_back(r);
			used.insert(r);
		}
	}
	return num;
}

// parameters:
// 1. springs: (c parameter is deterimined according to distance separately)
//	1.1 hard spring (bone); K_hard, b_hard, c_hard						k (600, 900)
//			
//	1.2 relative hard (hard muscle): K_muscle_hard, b_muscle, c_muscle  k (400, 600)
//
//	1.3 soft (soft muscle): K_soft, b_soft, c_soft						k (300, 400)

// the range of b is (0.5, 3)
// the choice of c is [0, 1/6, 1/4, 1/3]*M_PI + 
//					  [0, 1/2, 1, 3/2]*M_PI


//	law 1:, four part: x > 0, y > 0
//						x > 0, y < 0
//						x < 0, y > 0
//						x < 0, y < 0
//			when x=0 or y=0, it takes the mean value of b in 2 near by parts
//			when x=0, and y=0, it takes the mean value of b in 4 parts
//	law 2: parameter: w_x*x + w_y*y + w_z*z devide springs into 3 level and each level
//			in which, w_x+w_y+w_z=1
// these 2 laws are evolved.

// All above parameters are get by random firstly, within a range.
// Then, K and b follow law 2
// c follows law 1 and 2

// so totally 3 k, 3 b, 12 c, w_x, w_y, w_z   ==> 3 + 3 + 12 + 3 = 21 parameters

// all 3 functions are for evolve 21 parameters above
void randomInit() {
	std::vector<double> K_group = randomGeneratorDouble(100, 3);
	K[0] = K_group[0]*5 + 600; // 100-350
	K[1] = K_group[1]*5 + 600; // 100-300
	K[2] = K_group[2]*5 + 600; // 50-250

	std::vector<double> b_gen = randomGeneratorDouble(30, 4);  // 0 - 0.03
	for (int i = 0; i < 4; i++) {
		b[i] = (b_gen[i])/1000;
		//b[i] = 0.1;
	}

	std::vector<double> c_gen = randomGeneratorDouble(12, 8); //[0,2pi]
	for (int i = 0; i < 8; i++) {
		c[i] = (c_gen[i] - 6) / 12 * M_PI;
	}

	weight_x = randomValueInt(5);
	weight_y = randomValueInt(5);
	weight_z = randomValueInt(5);
}


// change parameters with small possibility
void mutation(int idx) {
	if (robotGroup[idx].fail == true) return;
	int doMutation = randomValueInt(6);
	std::vector<double> mutationChances;
	// 1/3 chance to mutate
	if (doMutation >= 2) return;

	mutationChances = randomGeneratorDouble(10, 3);
	if (mutationChances[0] == 1) {
		robotGroup[idx].MyK[0] = randomValueInt(40) * 4 + 100;
	}
	if (mutationChances[1] == 1) {
		robotGroup[idx].MyK[1] = randomValueInt(40) * 2 + 50;
	}
	if (mutationChances[2] == 1) {
		robotGroup[idx].MyK[2] = randomValueInt(40) + 50;
	}


	std::vector<double> b_gen = randomGeneratorDouble(25, 3);
	mutationChances = randomGeneratorDouble(10,3);
	for (int i = 0; i < 3; i++) {
		
		if (mutationChances[i] <=2) {
			robotGroup[idx].Myb[i] = b_gen[i] / 100;
		}
	}

	std::vector<double> c_gen = randomGeneratorDouble(12, 12); //[0,2pi]
	mutationChances = randomGeneratorDouble(24, 12);
	for (int i = 0; i < 12; i++) {

		if (mutationChances[i] <= 2) {
			robotGroup[idx].Myc[i] = (c_gen[i] - 6) / 12 * M_PI;
		}
	}

}

// use 2 robot to generate a new one
void crossOver() {
	
	int start = 0.6 * robot_num;


	for (int i = 0; i < 0.6 * robot_num; i++) {
		if (robotGroup[i].fail == true) {
			start = i;
			break;
		}
	}
	// if the number of good robots are too small,
	// generate some random individuals to increase diversity
	if (start <= 0.4 * robot_num) {
		for (int i = start; i < 0.6 * robot_num; i++) {
			randomInit();
			double length = 5 * (robot_num - 1);
			glm::dvec3 t = { -length / 2 + i * 5,0,0 };
			MyCubes myCubek(structure_base, i, t);
			robotGroup[i] = myCubek;
			start++;
		}
	}
	// here we only need to exchange k, b, c, weight
	std::vector<int> crossoverChance;
	std::vector<int> twoRobots;
	int cchance;
	
	twoRobots = randomValuesInt(int(start), 2);
	
	cchance = randomValueInt(10);
	
	//printf("finish parameter create");
	for (int rbt = start; rbt < robot_num; rbt += 2) {
		//printf("this is robot %d",rbt);
		
		robotGroup[rbt] = robotGroup[twoRobots[0]];
		robotGroup[rbt+1] = robotGroup[twoRobots[1]];
		crossoverChance = randomValuesInt(15, 2);
		
		// crossover 3 k values
		if (crossoverChance[0] < 3) {
			//printf("exchange Ks");
			for (int st = crossoverChance[0]; st < 3; st++) {
				robotGroup[rbt].MyK[st] = robotGroup[twoRobots[1]].MyK[st];
				robotGroup[rbt+1].MyK[st] = robotGroup[twoRobots[0]].MyK[st];
			}
		}
		// crossover 3 b values
		if (crossoverChance[1] < 3) {
			//printf("exchange bs");
			for (int st = crossoverChance[0]; st < 3; st++) {
				robotGroup[rbt].Myb[st] = robotGroup[twoRobots[1]].Myb[st];
				robotGroup[rbt + 1].Myb[st] = robotGroup[twoRobots[0]].Myb[st];
			}
		}

		// crossover 12 c values
		if (cchance < 6) {
			//printf("exchange cs");
			crossoverChance = randomValuesInt(13, 2);
			int left;
			int right;
			if (crossoverChance[0] <= crossoverChance[1]) {
				left = crossoverChance[0];
				right = crossoverChance[1];
			}
			else {
				left = crossoverChance[1];
				right = crossoverChance[0];
			}
			if (left == 12) {
				continue;
			}
			for (int st = left; st < right; st++) {
				//printf("start exchange!!");
				robotGroup[rbt].Myc[st] = robotGroup[twoRobots[1]].Myc[st];
				robotGroup[rbt + 1].Myc[st] = robotGroup[twoRobots[0]].Myc[st];
			}
		}

	}
	
	
	

}

//select the 60% of individuals with top speed
void select() {
	// compare the ith robot's pathlength with all behind it
	// if the later one has larger pathlength, swap them
	double maxlength;
	int toppart = 0.6 * robotGroup.size();
	int k = toppart;
	for (int i = 0; i < toppart; i++) {
		if (k == robotGroup.size()-1)break;
		if (robotGroup[i].fail == true) {
			swaprobot(i, k);
			k++;
		}
		maxlength = robotGroup[i].distance;
		for (int j = i + 1; j < robotGroup.size(); j++) {
			if (robotGroup[j].fail == true) continue;
			if (robotGroup[i].distance < robotGroup[j].distance) {
				swaprobot(i, j);
				maxlength = robotGroup[i].distance;
			}
		}
	}
}

// change the position of two robot in robotGroup vector
void swaprobot(int a, int b) {
	glm::dvec3 translate = { 0,0,0 };
	MyCubes tmpcube(structure_base, 0, translate);

	tmpcube = robotGroup[b];
	robotGroup[b] = robotGroup[a];
	robotGroup[a] = tmpcube;
	
}

// calculate the diversity of the whole group
double diversity() {
	// define the diversity:
	// because the k, b, c values are changed and evolved,
	// the diversity is based on these values.
	
	// suppose that k b c have equal important to the diversity
	// however, the size of them are different, 
	// we do the scale for them
	// 1. For k:
	//		[(ka0 - kb0)/250 + (ka1 - kb1)/200 + (ka2 - kb20)/200] / 3
	// 2. for b:
	//		[(ba0 - bb0) + (ba1 - bb1) + (ba2 - bb2)] / 3 / 0.5
	// 3. for c:
	//		[sumof(cai-cbi)] / 12 / 2 / M_PI

	double diversity_k = 0;
	double diversity_b = 0;
	double diversity_c = 0;
	double diversity = 0;

	int n = robotGroup.size();
	for (int i = 0; i < n; i++) {
	
		for (int j = i + i; j < n; j++) {
			diversity_k += abs((robotGroup[i].MyK[0] - robotGroup[j].MyK[0])) / 250 + 
						   abs((robotGroup[i].MyK[1] - robotGroup[j].MyK[1])) / 200 + 
						   abs((robotGroup[i].MyK[2] - robotGroup[j].MyK[2])) / 200;

			diversity_b += abs((robotGroup[i].Myb[0] - robotGroup[j].Myb[0])) +
						   abs((robotGroup[i].Myb[1] - robotGroup[j].Myb[1])) +
						   abs((robotGroup[i].Myb[2] - robotGroup[j].Myb[2])) + 
						   abs((robotGroup[i].Myb[3] - robotGroup[j].Myb[3]));

			for (int k = 0; k < 8; k++) {
				diversity_c += abs(robotGroup[i].Myc[k] - robotGroup[j].Myc[k]);
			}
		}
	}
	diversity = (diversity_k / 3 + diversity_b / 4 / 0.5 + diversity_c / 8 / 2 / M_PI)*33;
	return diversity;
}
