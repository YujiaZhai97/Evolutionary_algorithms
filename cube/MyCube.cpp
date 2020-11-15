
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <math.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>


// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <myTry\main.cpp>
//#include "main.cpp"
#include "MyCube.h"


using namespace glm;

extern const double LONGEST_R;
extern const double SHORTEST_R;
extern double dtime;
extern const double LOrigin;
extern const double K0;
extern const double Kground;
extern const double damp;
extern const glm::dvec3 gravity;
extern const const double mu0;
extern const double mass0;
extern const double omega;

extern double relative[];
extern int conn[];
extern double bars[];
extern int P_bars[];
extern int bar_update[];
extern int bar_breath[];

//struct MyPoint {
//	double mass = mass0;
//	glm::dvec3 Pos;
//	glm::dvec3 Vel = { 0.0f, 0.0f, 0.0f };
//	glm::dvec3 Acc = { 0.0f, 0.0f, 0.0f };
//	glm::dvec3 Force = mass * gravity;
//	int index;
//	int connect[3];
//	int P_bar[3];
//};


// input the translation (x, y, z) and 
// the rotation in eular angle (roll, pitch ,yaw)
// MyCube::MyCube(glm::dvec3 cubeTranslate, glm::dvec3 cubeRotation)
MyCube::MyCube(glm::dvec3 cubeTranslate) {

	for (int i = 0; i < 8; i++) {
		// create a point
		MyPoint a;
		a.index = i;
		// initiate its position
		a.Pos[0] = relative[3 * i] * LOrigin + cubeTranslate[0];
		a.Pos[1] = relative[3 * i + 1] * LOrigin + cubeTranslate[1];
		a.Pos[2] = relative[3 * i + 2] * LOrigin + cubeTranslate[2];

		for (int j = 0; j < 7; j++) {
			// initiate connected point index
			a.connect[j] = conn[7 * i + j];
			// initiate connected bars index
			a.P_bar[j] = P_bars[7 * i + j];
		}
		cube.push_back(a);

	}
	for (int i = 0; i < 28; i++) {
		MySpring s;
		s.index = i;
		s.K = K0;
		s.L = bars[i];
		s.point_a = bar_update[2 * i];
		s.point_b = bar_update[2 * i + 1];
		springs.push_back(s);
	}
}

MyCube::MyCube(glm::dvec3 cubeTranslate, std::vector<glm::dvec3> vel) {

	for (int i = 0; i < 8; i++) {
		// create a point
		MyPoint a;
		a.index = i;
		// initiate its position
		a.Pos[0] = relative[3 * i] * LOrigin + cubeTranslate[0];
		a.Pos[1] = relative[3 * i + 1] * LOrigin + cubeTranslate[1];
		a.Pos[2] = relative[3 * i + 2] * LOrigin + cubeTranslate[2];

		a.Vel = vel[i];

		for (int j = 0; j < 7; j++) {
			// initiate connected point index
			a.connect[j] = conn[7 * i + j];
			// initiate connected bars index
			a.P_bar[j] = P_bars[7 * i + j];
		}
		cube.push_back(a);

	}
	for (int i = 0; i < 28; i++) {
		MySpring s;
		s.index = i;
		s.K = K0;
		s.L = bars[i];
		s.point_a = bar_update[2 * i];
		s.point_b = bar_update[2 * i + 1];
		springs.push_back(s);
	}
}

MyCube::MyCube(glm::dvec3 cubeTranslate, bool fix[]) {

	for (int i = 0; i < 8; i++) {
		// create a point
		MyPoint a;
		a.index = i;
		// initiate its position
		a.Pos[0] = relative[3 * i] * LOrigin + cubeTranslate[0];
		a.Pos[1] = relative[3 * i + 1] * LOrigin + cubeTranslate[1];
		a.Pos[2] = relative[3 * i + 2] * LOrigin + cubeTranslate[2];
		a.fixed = fix[i];
		for (int j = 0; j < 7; j++) {
			// initiate connected point index
			a.connect[j] = conn[7 * i + j];
			// initiate connected bars index
			a.P_bar[j] = P_bars[7 * i + j];
		}
		cube.push_back(a);

	}
	for (int i = 0; i < 28; i++) {
		MySpring s;
		s.index = i;
		s.K = K0;
		s.L = bars[i];
		s.point_a = bar_update[2 * i];
		s.point_b = bar_update[2 * i + 1];
		springs.push_back(s);
	}
}

void MyCube::CubeUpdate(std::vector<glm::dvec3> extForce) {

	//std::cout << "start update\n";
	//std::cout << "cube has " << sizeof(cube) << " elements\n";
	int k = sizeof(extForce);
	glm::dvec3 Fspring;
	glm::dvec3 Fground = {0,0,0};
	glm::dvec3 Ffriction = {0,0,0};
	glm::dvec3 L;
	glm::dvec3 directionf;
	
	//glm::dvec3 constrain = { 0,0,0 };
	//std::cout << "finish init variable\n";

	for (int i = 0; i < 28; i++) {
		// update springs
		springs[i].L = myDistance(springs[i].point_a, springs[i].point_b, i);
		if (springs[i].L >= LONGEST_R * bars[i] || springs[i].L <= SHORTEST_R * bars[i]) {
			springs[i].F = 0;
		} else {
			springs[i].F = (springs[i].L - bars[i])* K0;
		}
	}
	
	//std::cout << "finish update the lenth of every edge\n";
	for (int i = 0; i < 8; i++) {
		Fspring = mySpringForce(i);

		// update forces
		if (cube[i].Vel[0] == 0 && cube[i].Vel[1] == 0) {
			directionf = glm::dvec3(0, 0, 0);
		} else {
			directionf = glm::normalize(glm::dvec3(-cube[i].Vel[0], -cube[i].Vel[1], 0));
		}

		if (cube[i].Pos[2] <= 0) {
			// Ffriction and Fground exist
			Ffriction = directionf * cube[i].Force[2] * mu0;
			Fground = glm::dvec3(0, 0, -1 * cube[i].Pos[2] * Kground);
		}
		//std::cout << "update spring force\n";

		cube[i].Force = extForce[i] + Ffriction + Fground + Fspring + gravity * cube[i].mass;


		if (cube[i].fixed == 0) {
			cube[i].Pos += dtime * cube[i].Vel;
			cube[i].Vel = (cube[i].Vel + dtime * cube[i].Acc) * damp;
			cube[i].Acc = cube[i].Force / cube[i].mass;
		}
	}

}

double MyCube::myDistance(int a, int b, int bar) {
	//std::cout << "in the function myDistance\n";
	double l = glm::distance(cube[a].Pos, cube[b].Pos);
	return l;
}

glm::dvec3 MyCube::mySpringForce(int cubeindex) {
	//std::cout << "in the function mySpringForce\n";
	glm::dvec3 Force = { 0.0,0.0,0.0 };
	glm::dvec3 direction;
	int end;
	int bar;
	double f;
	//std::cout << "%%%%%%%%%%%%%%%%%this point is " << cubeindex << "\n";
	for (int i = 0; i < 7; i++) {
		end = cube[cubeindex].connect[i];
		bar = cube[cubeindex].P_bar[i];
		direction = glm::normalize(cube[end].Pos - cube[cubeindex].Pos);  
		f = springs[bar].F;
		Force += direction * f;
	}
	//std::cout <<"Force of springs"<< Force[0] << ", " << Force[1] << ", " << Force[2] << "\n";
	return Force;
}

glm::dvec3 MyCube::checkConstrain(int point) {

	glm::dvec3 check = { 0,0,0 };

	int x_bar;
	int y_bar;
	int z_bar;
	int start;
	glm::dvec3 dir;
	x_bar = P_bars[point * 3];
	y_bar = P_bars[point * 3 + 1];
	z_bar = P_bars[point * 3 + 2];

	if (conn[point * 3] < point) {
		start = conn[point * 3];
		dir = cube[point].Pos - cube[start].Pos;
		
		if (bars[x_bar] >= LONGEST_R * LOrigin && glm::dot(cube[start].Vel, dir)>0) {
			// when this edge is longest and velocity is going to elongate it
			check[0] = 1;
		} else if (bars[x_bar] <= SHORTEST_R * LOrigin && glm::dot(cube[start].Vel, dir) < 0) {
			// when this edge is shortest and velocity is going to shorten it
			check[0] = -1;
		}
	}

	if (conn[point * 3 + 1] < point) {
		start = conn[point * 3 + 1];
		dir = cube[point].Pos - cube[start].Pos;
		if (bars[y_bar] >= LONGEST_R * LOrigin && glm::dot(cube[start].Vel, dir) > 0) {
			check[1] = 1;
		} else if (bars[y_bar] <= SHORTEST_R * LOrigin && glm::dot(cube[start].Vel, dir) < 0) {
			check[1] = -1;
		}
	}

	if (conn[point * 3 + 2] < point) {
		start = conn[point * 3 + 2];
		dir = cube[point].Pos - cube[start].Pos;
		if (bars[z_bar] >= LONGEST_R * LOrigin && glm::dot(cube[start].Vel, dir) > 0) {
			check[2] = 1;
		} else if (bars[z_bar] <= SHORTEST_R * LOrigin && glm::dot(cube[start].Vel, dir) < 0) {
			check[2] = -1;
		}
	}
	
	return check;
}

// we update Position, Velocity, Acceleration here
// and consider the limited length of edge
void MyCube::updateWithConstrian(glm::dvec3 constrain, int point) {

	glm::dvec3 a_d;
	glm::dvec3 b_d;
	glm::dvec3 c_d;

	if (constrain == glm::dvec3(0, 0, 0)) {
		// no constrain 
		cube[point].Pos += dtime * cube[point].Vel;
		cube[point].Vel = cube[point].Vel * damp + dtime * cube[point].Acc;
		cube[point].Acc = cube[point].Force / cube[point].mass;
		return;
	} 
	else if (glm::length(constrain) == 1) {
		// 1 edge reach constrain
		// if the dirction of position change is towoards LOrigin, we keep it!
		// or we should constrain the Pos, Vel, Acc
		int start;
		if (constrain[0] != 0) {
			start = conn[3 * point];

		} else if (constrain[1] != 0) {
			start = conn[3 * point + 1];

		} else { //constrain[2] != 0
			start = conn[3 * point + 2];

		}
		a_d = cube[point].Pos - cube[start].Pos;   // a vector from start to point
		b_d = getNormal(a_d, cube[point].Vel); // constrain the velocity -> position
		cube[point].Pos += b_d * dtime;

		b_d = getNormal(a_d, cube[point].Acc); // constrain the acceleration -> velocity
		cube[point].Vel += b_d * dtime;

		cube[point].Acc = cube[point].Force / cube[point].mass; // Acceleration can keep the edge-orientation part
		return;
	} 
	else if (glm::length(constrain) == 2) {
		// 2 edges reaches constrain
		//(constrain[0] == -1 && (constrain[1] == -1 || constrain[2] == -1)) || (constrain[1] == -1 && constrain[2] == -1)
		int start_a;
		int start_b;
		if (constrain[0] == 0) {
			start_a = conn[3 * point + 1];
			start_b = conn[3 * point + 2];

		} else if (constrain[1] == 0) {
			start_a = conn[3 * point + 0];
			start_b = conn[3 * point + 2];
		} else if (constrain[2] == 0) {
			start_a = conn[3 * point + 0];
			start_b = conn[3 * point + 1];
		}

		a_d = cube[point].Pos - cube[start_a].Pos;
		b_d = cube[point].Pos - cube[start_b].Pos;

		c_d = getNormal(a_d, b_d, cube[point].Vel);
		cube[point].Pos += c_d * dtime;

		c_d = getNormal(a_d, b_d, cube[point].Acc);
		cube[point].Pos += c_d * dtime;

		cube[point].Acc = cube[point].Force / cube[point].mass; // Acceleration can keep the edge-orientation part
		return;
	} 
	else {
		// 3 edges reaches constrain
		double L = SHORTEST_R * LOrigin;
		double Lc;
		double dL;
		int start;
		glm::dvec3 newpoint = cube[point].Pos;
		// update the position
		for (int i = 0; i < 3; i++) {
			
			start = conn[3 * point + i];
			a_d = newpoint - cube[start].Pos;
			if (constrain[i] == 1) {
				L = LONGEST_R * LOrigin;
				Lc = glm::length(a_d);
				if (Lc > L) {
					dL = Lc - L;
					newpoint = newpoint - dL * glm::normalize(a_d);
				}
			}
		}
		cube[point].Pos = newpoint;

		// update the velocity
		a_d = cube[point].Pos - cube[conn[3 * point]].Pos;
		b_d = cube[point].Pos - cube[conn[3 * point+1]].Pos;
		c_d = cube[point].Pos - cube[conn[3 * point+2]].Pos;
		
		a_d = glm::dot(cube[conn[3 * point]].Vel, a_d) * glm::normalize(a_d) / glm::length(a_d);
		b_d = glm::dot(cube[conn[3 * point+1]].Vel, b_d) * glm::normalize(b_d) / glm::length(b_d);
		c_d = glm::dot(cube[conn[3 * point + 1]].Vel, c_d) * glm::normalize(c_d) / glm::length(c_d);
		cube[point].Acc = cube[point].Force / cube[point].mass; // Acceleration can keep the edge-orientation part
		return;
	}
}

// from vecter a and g vector
// get the norm vector b, c
// where b, c is vertical to a, and a + b + c = g
glm::dvec3 MyCube::getNormal(glm::dvec3 a, glm::dvec3 g) {
	glm::dvec3 b = { 1, 2, 0 };
	glm::dvec3 c = { 0, 0, 1 };

	if (a[2] == 0 && a[1] == 0) {
		// a = {1,0,0}
		b = glm::dvec3(0, 1, 0);
	} else if (a[2] == 0) {
		b[1] = -(a[0] * b[0]) / a[1];
	} else { // a[2] != 0
		b[2] = -(a[0] * b[0] + a[1] * b[1]) / a[2];
		if (a[0] == 0 && a[1] == 0) {
			// a = {0,0,1}
			// make c vertical to b
			double cx = -b[1] / b[0];
			c = glm::dvec3(cx, 1, 0);
		}else if (a[0] == 0) {
			c[1] = -a[2] * c[2] / a[1];
			c[0] = -(b[1] * c[1] + b[2] * c[2]) / b[1];
		} else if (a[1] == 0) {
			c[0] = -a[2] * c[2] / a[0];
			c[1] = -(b[0] * c[0] + b[2] * c[2]) / b[1];
		}
	}
	glm::mat3 vectors = { a, b, c };
	glm::dvec3 para;
	vectors = glm::inverse(vectors);
	para = vectors * g;
	return para[1] * b + para[2] * c;
}

// from vector a, b, and g vector
// get the norm vector c
// where c is vertical to a and b, and a + b + c = g
glm::dvec3 MyCube::getNormal(glm::dvec3 a, glm::dvec3 b, glm::dvec3 g) {
	
	glm::dvec3 c = glm::cross(a, b);
	glm::mat3 vectors = { a, b, c };
	glm::dvec3 para;
	vectors = glm::inverse(vectors);
	para = vectors * g;
	return para[2] * c;
	
}

void MyCube::CubeBreath(double t, double a[], double b[], double c[]) {
	//int bar_breath[] = {
	//0, 2, 8, 10,		// x directin
	//1, 3, 9, 11,		// y direction
	//4, 5, 7, 6			// z direction
	//};
	int edge;
	double l_goal;
	int point_a;
	int point_b;
	double l;
	double dist;
	double v;

	glm::dvec3 pos_a;
	glm::dvec3 pos_b;
	glm::dvec3 move_direction;

	for (int d = 0; d < 3; d++) {
		for (int e = 0; e < 4; e++) {
			edge = bar_breath[4 * d + e];
			l_goal = a[4 * d + e] + b[4 * d + e] * sin(t * omega + c[4 * d + e]);
			point_a = springs[edge].point_a;
			point_b = springs[edge].point_b;
			//std::cout << "point a is " << point_a << " and point b is" << point_b << "\n";
			v = omega * b[edge] * cos(omega * t + c[edge]);
			pos_a = cube[point_a].Pos;
			pos_b = cube[point_b].Pos;
			move_direction = glm::normalize(pos_a - pos_b);
			l = glm::distance(pos_a, pos_b);

			if (d == 2) {
				// if point.pos[2] == 0, it is on the ground, 
				// do not change z value
				if (pos_a[d] <= 0) {
					dist = l_goal - l;
					pos_b[d] -= move_direction[d] * dist;
					cube[point_b].Pos = pos_b;
					cube[point_b].Vel[d] = -move_direction[d] * v;
				} else if (pos_b[d] <= 0) {
					dist = l_goal - l;
					pos_a[d] += move_direction[d] * dist;
					cube[point_a].Pos = pos_a;
					cube[point_a].Vel[d] = move_direction[d] * v;
				}
				continue;
			}
			dist = (l_goal - l) / 2;
			//pos_a += move_direction * dist;
			//pos_b -= move_direction * dist;
			pos_a[d] += move_direction[d] * dist;
			pos_b[d] -= move_direction[d] * dist;

			cube[point_a].Pos = pos_a;
			cube[point_b].Pos = pos_b;
			cube[point_a].Vel[d] = move_direction[d] * v;
			cube[point_b].Vel[d] = - move_direction[d] * v;
		}
	}

	
	
	
}