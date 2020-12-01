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

using namespace glm;

extern const double LONGEST_R;
extern const double SHORTEST_R;
extern double dtime;
extern const double LOrigin;
extern const double K0;
extern const double Kground;
extern double damp;
extern const glm::dvec3 gravity;
extern const const double mu0;
extern const double mass0;
extern double omega;
extern double b[];
extern double c[];
extern const int robot_num;
extern double relative[];
extern int conn[];
extern double bars[];
extern int P_bars[];
extern int bar_update[];
//extern int bar_breath[];

extern double K[];
extern double b[];
extern double c[];
extern double weight_x;
extern double weight_y;
extern double weight_z;
extern int encode[];
extern const int cube_count;


extern GLdouble myCube_vertex[];  // cube_count * 8 points * 3 (x, y, z)
extern GLdouble myCube_color[];	  // cube_count * 8 points * 3 bit
extern GLuint myCubeindices[];    // cube_count * 6 faces * 2 triangles * 3 indices

extern GLdouble myShade_vertex[];
extern GLdouble myShade_color[];
extern GLuint myShadeindices[];

class MyCubes {

public:
	struct springGroup {
		// a < b
		int a;
		int b; 
	};

	struct MyPoint {
		int index;
		double mass = mass0;
		glm::dvec3 startPos;
		glm::dvec3 Pos;
		glm::dvec3 Vel = { 0, 0, 0 };
		glm::dvec3 Acc = { 0, 0, 0 };
		glm::dvec3 Force = mass * gravity;
		
		int connect[7];
		std::vector<int> P_bar;
		bool fixed = 0;
	};

	struct MySpring {
		springGroup group;
		int index;
		double K;
		double L;
		double L0;
		double F = 0;
		double a;	// L = a + b * sin(wt + c)
		double b;
		double c;
				
		
	};

	struct MyEle {
		int index;
		int pointId[8];
		int faceSpring[12];
		int edgeSpring[12];
		int intenalSpring[4];
		glm::dvec3 pos;
		glm::dvec3 color;
	};

	
	std::vector<MyPoint> points;
	std::vector<MySpring> springs;
	std::vector<MyEle> elements;
	
	
	glm::dvec3 startPos;
	glm::dvec3 endPos;
	glm::dvec3 bodyCenter;
	
	bool fail = false;
	double w_x;
	double w_y;
	double w_z;
	double MyK[3];
	double Myb[4];
	double Myc[8];
	double distance = 0;
	double EK = 0;
	double EP = 0;
	double E = 0;

	MyCubes(GLdouble structure[], int robot, glm::dvec3 trans); // constructor
	
	// update the cube according to phisical law
	void Update(std::vector<glm::dvec3> extForce);

	// update the L0 = a + b * sin(wt+c)
	void Breath(double time);

	// get length the robot goes from start to now
	double pathLength();

	// get the geometric center of the cube
	void getCenter();

	// set Pos, Vel, Acc, Force to the start state
	void restart(GLdouble build[], glm::dvec3 trans);

	// calculate kinetic Energy
	void kineticE();

	// calculate potential energy
	void potentialE();

	// calculate all energy
	void allEnergy();



	void toBuffer(int robot);
	void checkExist(int index, GLdouble build[]);
	int findIdx(springGroup ab, std::vector<MySpring> springset);
	bool findGroup(std::vector<springGroup> grp, springGroup tmpab);
	glm::dvec3 mySpringForce(int cubeindex);
	

};

