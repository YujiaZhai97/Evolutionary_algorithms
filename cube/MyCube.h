#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>


// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <myTry\main.cpp>
//#include "main.cpp"
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

class MyCube {

public:
	struct MyPoint {
		double mass = mass0;
		glm::dvec3 Pos;
		glm::dvec3 Vel = {0, 0, 0.2 };
		glm::dvec3 Acc = { 0, 0, 0 };
		glm::dvec3 Force = mass*gravity;
		//int cubeindex;
		int index;
		int connect[7];
		int P_bar[7];
		bool fixed = 0;
	};

	struct MySpring {
		double K;
		double L;
		//int cubeindex;
		double F;
		int index;
		int point_a;
		int point_b;
	};

	std::vector<MyPoint> cube;
	std::vector<MySpring> springs;
	//MyCube(glm::dvec3 central, glm::dvec3 Rot);
	MyCube(glm::dvec3 cubeTranslate); // constructor
	MyCube(glm::dvec3 cubeTranslate, std::vector<glm::dvec3> vel);
	MyCube(glm::dvec3 cubeTranslate, bool fix[]);

	void CubeUpdate(std::vector<glm::dvec3> extForce);
	void CubeBreath(double t, double a[], double b[], double c[]);

// private:
	double myDistance(int a, int b, int bar);
	glm::dvec3 mySpringForce(int cubeindex);
	// update the position of a point
	//  and set constrain for the length of the edge between two points
	glm::dvec3 checkConstrain(int point);
	// we update Position, Velocity, Acceleration here
	// and consider the limited length of edge
	void updateWithConstrian(glm::dvec3 check, int point);
	// from vecter a and g vector
	// get the norm vector b
	// where b is vertical to a, and a + b = g
	glm::dvec3 getNormal(glm::dvec3 a, glm::dvec3 g);
	// from vector a, b, and g vector
	// get the norm vector c
	// where c is vertical to a and b, and a + b + c = g
	glm::dvec3 getNormal(glm::dvec3 a, glm::dvec3 b, glm::dvec3 g);
};

