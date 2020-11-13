// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <chrono>
#include <thread>
#include<fstream>
#include<cstring>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include "MyCube.h"

#define SCREEN_WIDTH 1000
#define SCREEN_HEIGHT 750
#define GRID_SIZE 10
#define GRID_EDGE 1.0

const double LONGEST_R = 2.3;
const double SHORTEST_R = 0.3;
double dtime = 0.0001;	// s
const double LOrigin = 1;		// m
const double K0 = 700.0;		// k = F/dx (N/m) 500
const double Kground = 3000.0;
const double damp = 1;         // update v = 0.9*v
const glm::dvec3 gravity = {0.0, 0.0, -9.8}; // z -9.8 N/Kg
const double mu0 = 0.1;			// mu = Ff/Fn
const double mass0 = 0.1;		// 0.1kg
const double sqrt2 = sqrt(2);
const double sqrt3 = sqrt(3);
const double a0 = 1.0;
const double b0 = 0.3;
const double b1 = -0.1;
const double c0 = 0;
const double omega = 3;

double relative[] = {
		0.5, 0.5, -0.5,					// 0  x, y, z locally
		-0.5, 0.5, -0.5,				// 1
		-0.5, -0.5, -0.5,				// 2
		0.5, -0.5, -0.5,				// 3

		0.5, 0.5, 0.5,					// 4
		-0.5, 0.5, 0.5,					// 5
		-0.5, -0.5, 0.5,				// 6
		0.5, -0.5, 0.5					// 7
};

// each point connects with other 7 points
int conn[] = {
	1, 3, 4, 2, 7, 5, 6,	// 0 x, y, z || xy, yz, zx || xyz locally
	0, 2, 5, 3, 6, 4, 7, 	// 1
	3, 1, 6, 0, 5, 7, 4,	// 2
	2, 0, 7, 1, 4, 6, 5,	// 3
	5, 7, 0, 6, 3, 1, 2, 	// 4
	4, 6, 1, 7, 2, 0, 3,	// 5
	7, 5, 2, 4, 1, 3, 0,	// 6
	6, 4, 3, 5, 0, 2, 1		// 7
};

// each point connected with 7 bars
int P_bars[] = {
	0, 3, 4, 12, 19, 23, 24,		// 0  x, y, z, xy, yz, xz, xyz locally
	0, 1, 5, 13, 17, 22, 25,		// 1
	2, 1, 6, 12, 16, 20, 26,		// 2
	2, 3, 7, 13, 18, 21, 27,		// 3
	8, 11, 4,14, 18, 22, 26,		// 4
	8, 9, 5, 15, 16, 23, 27,		// 5
	10, 9, 6, 14, 17, 21,24,		// 6
	10, 11, 7, 15, 19, 20, 25		// 7
};

// the length of each bar
double bars[] = {
	LOrigin, LOrigin, LOrigin, LOrigin, // bottom horizontal 0, 1, 2, 3
	LOrigin, LOrigin, LOrigin, LOrigin, // mid vertical		 4, 5, 6, 7
	LOrigin, LOrigin, LOrigin, LOrigin, // top horizontal	 8, 9, 10, 11

	sqrt2 * LOrigin, sqrt2 * LOrigin, sqrt2 * LOrigin, sqrt2* LOrigin, // xy-plane botto-top	12, 13, 14, 15
	sqrt2 * LOrigin, sqrt2 * LOrigin, sqrt2 * LOrigin, sqrt2* LOrigin, // yz-plane back-front	16, 17, 18, 19
	sqrt2 * LOrigin, sqrt2 * LOrigin, sqrt2 * LOrigin, sqrt2* LOrigin, // xz-plane left-right	20, 21, 22, 23

	sqrt3 * LOrigin, sqrt3 * LOrigin, sqrt3 * LOrigin, sqrt3 * LOrigin //4 corner cross		24, 25, 26, 27
};

// each bar connects with 2 end points
int bar_update[] = {
	0, 1,	// 0
	1, 2,	// 1
	2, 3,	// 2
	3, 0,	// 3
	4, 0,	// 4
	5, 1,	// 5
	6, 2,	// 6
	7, 3,	// 7
	4, 5,	// 8
	5, 6,	// 9
	6, 7,	// 10
	7, 4,	// 11
	0, 2,   // 12
	1, 3,	// 13
	4, 6,	// 14
	5, 7,	// 15
	2, 5, 	// 16
	1, 6, 	// 17
	3, 4,	// 18
	0, 7,	// 19
	2, 7,	// 20
	3, 6, 	// 21
	1, 4,	// 22
	0, 5,	// 23
	0, 6,	// 24
	1, 7,	// 25
	2, 4,	// 26
	3, 5	// 27

};

int bar_breath[] = {
	0, 2, 8, 10,		// x directin
	1, 3, 9, 11,		// y direction
	4, 5, 7, 6			// z direction
};

double a[] = {
	a0, a0, a0, a0,
	a0, a0, a0, a0,
	a0, a0, a0, a0
};

double b[] = {
	b0, b0, b0, b0,
	b1, b1, b1, b1,
	b0, b0, b0, b0
};

double c[] = {
	c0, c0, c0, c0,
	c0, c0, c0, c0,
	c0, c0, c0, c0
};

std::vector<glm::dvec3> spin_v;

bool fix[] = {
	0, 0, 0, 0, 1, 0, 0, 0
};

GLdouble gridVertices[(GRID_SIZE + 1) * 2 * 2 * 3];

GLdouble gridColor[(GRID_SIZE + 1) * 2 * 2 * 3];

const GLdouble coordVertices[] = {
	0, 0, 0,
	1.5, 0, 0,

	0, 0, 0,
	0, 1.5, 0,

	0, 0, 0,
	0, 0, 1.5
};

const GLdouble CoordColor[]{
	0.7, 0, 0,
	0.7, 0, 0,

	0, 0.7, 0,
	0, 0.7, 0,

	0, 0, 0.7,
	0, 0, 0.7
};



GLdouble myCube_vertex[24];

GLdouble myCube_color[] = {
	0.5, 0.1, 0.05,
	0.5, 0.1, 0.05,
	0.5, 0.1, 0.05,
	0.5, 0.1, 0.05,
	   	   
	0.5, 0.1, 0.05,
	0.5, 0.1, 0.05,
	0.5, 0.1, 0.05,
	0.5, 0.1, 0.05
};

GLdouble myPoints_color[] = {
	0.2, 0.7, 0.5,
	0.2, 0.7, 0.5,
	0.2, 0.7, 0.5,
	0.2, 0.7, 0.5,
	0.2, 0.7, 0.5,
	0.2, 0.7, 0.5,
	0.2, 0.7, 0.5,
	0.2, 0.7, 0.5
};

GLuint myCubeindices[] = { 
	0, 1,	// 0
	1, 2,	// 1
	2, 3,	// 2
	3, 0,	// 3
	4, 0,	// 4
	5, 1,	// 5
	6, 2,	// 6
	7, 3,	// 7
	4, 5,	// 8
	5, 6,	// 9
	6, 7,	// 10
	7, 4,	// 11
	0, 2,   // 12
	1, 3,	// 13
	4, 6,	// 14
	5, 7,	// 15
	2, 5, 	// 16
	1, 6, 	// 17
	3, 4,	// 18
	0, 7,	// 19
	2, 7,	// 20
	3, 6, 	// 21
	1, 4,	// 22
	0, 5,	// 23
	0, 6,	// 24
	1, 7,	// 25
	2, 4,	// 26
	3, 5	// 27
};

void gridVertGen();
void drawGrid();
void drawCoord();
void tobuffer(MyCube cubek);
int stepCheck(MyCube cubek);
void Spinz(double v);
void Spinx(double v);
void Spiny(double v);
int main(void) {
	double timeori;
	double timepre;
	double timecur;
	double time_sim = 0;
	int loops = 10;
	std::ofstream output;
		#pragma region Init Window
	// Initialise GLFW
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Assignment 3A-Breathing Cube", NULL, NULL);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	//glEnable(GL_CULL_FACE);// delete the back side parts
	//glCullFace(GL_BACK);

	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark background
	glClearColor(0.7f, 0.7f, 0.7f, 0.0f);

	// VAO vertex array object
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(60.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Or, for an ortho camera :
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(6, -3, 2), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 0, 1)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
		#pragma endregion

	gridVertGen();
	//Spinz(0.7);
	//Spinx(1);
	Spiny(1);
	//MyCube myCubek(glm::dvec3(0,0,0.5));
	MyCube myCubek(glm::dvec3(0, 0, 2), spin_v);
	//MyCube myCubek(glm::dvec3(0, 0, 2), fix);
	
	tobuffer(myCubek);
	std::vector<glm::dvec3> extForce;
	glm::dvec3 e = { 0,0,0 };
	for (int i = 0; i < 12; i++) {
		extForce.push_back(e);
	}
		#pragma region VBOs
	// VBO vertex buffer object
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertices), gridVertices, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gridColor), gridColor, GL_STATIC_DRAW);

	GLuint vertexbufferCoord;
	glGenBuffers(1, &vertexbufferCoord);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCoord);
	glBufferData(GL_ARRAY_BUFFER, sizeof(coordVertices), coordVertices, GL_STATIC_DRAW);

	GLuint colorbufferCoord;
	glGenBuffers(1, &colorbufferCoord);
	glBindBuffer(GL_ARRAY_BUFFER, colorbufferCoord);
	glBufferData(GL_ARRAY_BUFFER, sizeof(CoordColor), CoordColor, GL_STATIC_DRAW);

	GLuint vertexbufferCube;
	glGenBuffers(1, &vertexbufferCube);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myCube_vertex), myCube_vertex, GL_DYNAMIC_DRAW);

	GLuint colorbufferCube;
	glGenBuffers(1, &colorbufferCube);
	glBindBuffer(GL_ARRAY_BUFFER, colorbufferCube);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myCube_color), myCube_color, GL_DYNAMIC_DRAW);

	GLuint colorbufferPoint;
	glGenBuffers(1, &colorbufferPoint);
	glBindBuffer(GL_ARRAY_BUFFER, colorbufferPoint);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myPoints_color), myPoints_color, GL_DYNAMIC_DRAW);
		#pragma endregion

	// EBO Element buffer -- myCubeindices
	GLuint EBOCube;
	glGenBuffers(1, &EBOCube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOCube);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(myCubeindices), myCubeindices, GL_DYNAMIC_DRAW);
	
	timepre = glfwGetTime();
	timeori = timepre;
	int count = 0;
	output.open("BreathCube.txt", std::fstream::app);
	output << K0 << "," << mass0 << "," << 9.8 << "," << a0 << "," << b0 << "," << c0 << "\n";
	/*output.open("BouncingCube.txt", std::fstream::app);
	output << K0 << "," << mass0 << "," << 9.8 << "\n";*/
	do {
		std::cout << "============================round " << count << " ========================\n";
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			#pragma region Grid
		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_DOUBLE,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_DOUBLE,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		for (int i = 0; i < (GRID_SIZE + 1) * 2; i++) {
			glDrawArrays(GL_LINES, 2 * i, 2);
		}

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
			#pragma endregion

			#pragma region Coordinate
		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCoord);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_DOUBLE,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);
		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbufferCoord);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_DOUBLE,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
		glLineWidth(1.0f);
		glDrawArrays(GL_LINES, 0, 2);
		glDrawArrays(GL_LINES, 2, 2);
		glDrawArrays(GL_LINES, 4, 2);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
			#pragma endregion

			#pragma region Cube
		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_DOUBLE,           // type
			GL_FALSE,           // normalized?
			0,                  // stride 3*sizeof(float)
			(void*)0            // array buffer offset
		);
		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbufferCube);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_DOUBLE,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
		// 3rd element buffer: indices
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOCube);
		glDrawElements(GL_LINES, 56, GL_UNSIGNED_INT, 0);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
			#pragma endregion

			#pragma region Points
		
		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_DOUBLE,           // type
			GL_FALSE,           // normalized?
			0,                  // stride 3*sizeof(float)
			(void*)0            // array buffer offset
		);
		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbufferPoint);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_DOUBLE,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);
		//glEnable(GL_ALPHA_TEST);
		//glAlphaFunc(GL_NOTEQUAL, 0);
		//glEnable(GL_POINT_SMOOTH);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		glPointSize(2.0f);
		glDrawArrays(GL_POINTS, 0, 8);
		//glDisable(GL_BLEND);
		//glDisable(GL_POINT_SMOOTH);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		
			#pragma endregion
		//std::cout << "gonna go to update now \n";
		
		loops = stepCheck(myCubek);
		for (int tt = 0; tt < loops; tt++) {
			myCubek.CubeUpdate(extForce);
			//myCubek.CubeBreath(time_sim, a, b, c);
		}
		
		/*std::cout << "+++++++ " << 1 << "th point +++++++++++++\n";
		std::cout << "new position is " << myCubek.cube[1].Pos[0] << ", " << myCubek.cube[1].Pos[1] << ", " << myCubek.cube[1].Pos[2] << " \n";
		std::cout << "new velocity is " << myCubek.cube[1].Vel[0] << ", " << myCubek.cube[1].Vel[1] << ", " << myCubek.cube[1].Vel[2] << " \n";
		std::cout << "new acceleration is " << myCubek.cube[1].Acc[0] << ", " << myCubek.cube[1].Acc[1] << ", " << myCubek.cube[1].Acc[2] << " \n";
		
		std::cout << "Force is " << myCubek.cube[1].Force[0] << ", " << myCubek.cube[1].Force[1] << ", " << myCubek.cube[1].Force[2] << "\n";

		std::cout << "+++++++ " << 0 << "th point +++++++++++++\n";
		std::cout << "new position is " << myCubek.cube[0].Pos[0] << ", " << myCubek.cube[0].Pos[1] << ", " << myCubek.cube[0].Pos[2] << " \n";
		std::cout << "new velocity is " << myCubek.cube[0].Vel[0] << ", " << myCubek.cube[0].Vel[1] << ", " << myCubek.cube[0].Vel[2] << " \n";
		std::cout << "new acceleration is " << myCubek.cube[0].Acc[0] << ", " << myCubek.cube[0].Acc[1] << ", " << myCubek.cube[0].Acc[2] << " \n";

		std::cout << "Force is " << myCubek.cube[0].Force[0] << ", " << myCubek.cube[0].Force[1] << ", " << myCubek.cube[0].Force[2] << "\n";

		std::cout << "+++++++ " << 3 << "th point +++++++++++++\n";
		std::cout << "new position is " << myCubek.cube[3].Pos[0] << ", " << myCubek.cube[3].Pos[1] << ", " << myCubek.cube[3].Pos[2] << " \n";
		std::cout << "new velocity is " << myCubek.cube[3].Vel[0] << ", " << myCubek.cube[3].Vel[1] << ", " << myCubek.cube[3].Vel[2] << " \n";
		std::cout << "new acceleration is " << myCubek.cube[3].Acc[0] << ", " << myCubek.cube[3].Acc[1] << ", " << myCubek.cube[3].Acc[2] << " \n";

		std::cout << "Force is " << myCubek.cube[3].Force[0] << ", " << myCubek.cube[3].Force[1] << ", " << myCubek.cube[3].Force[2] << "\n";*/
		time_sim += loops * dtime;
		timecur = glfwGetTime();
		//std::cout << "current simulation time is " << time_sim << "\n";
		//std::cout << "current time is " << timecur - timeori << " \n";
		//std::cout << "time span of this loop is " << timecur - timepre << "\n";
		timepre = timecur;
		// potential energy part:
		//		z value of every point:
		for (int i = 0; i < 8; i++) { // 8
			output << myCubek.cube[i].Pos[2] << ",";
		}
		//		each length of spring:
		for (int i = 0; i < 28; i++) { //28
			output << myCubek.springs[i].L << ",";
		}
		//kinetic energy part:
		// velocities:
		for (int i = 0; i < 8; i++) { //24
			output << myCubek.cube[i].Vel[0] << "," << myCubek.cube[i].Vel[1] << "," << myCubek.cube[i].Vel[2] << ",";
		}
		// simulation time and loops (28 springs per loop)
		output << time_sim << "," << loops << ","<< timecur - timeori<<"\n" ;
		tobuffer(myCubek);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
		glBufferData(GL_ARRAY_BUFFER, sizeof(myCube_vertex), myCube_vertex, GL_DYNAMIC_DRAW);
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		
		count++;
		
		//std::this_thread::sleep_for(std::chrono::nanoseconds(100000000));
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteBuffers(1, &vertexbufferCoord);
	glDeleteBuffers(1, &colorbufferCoord);
	glDeleteBuffers(1, &vertexbufferCube);
	glDeleteBuffers(1, &colorbufferCube);
	glDeleteBuffers(1, &EBOCube);
	glDeleteBuffers(1, &colorbufferPoint);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

void gridVertGen() {
	GLdouble left = -GRID_SIZE / 2 * GRID_EDGE;
	GLdouble bottom = -GRID_SIZE / 2 * GRID_EDGE;
	GLdouble right = GRID_SIZE / 2 * GRID_EDGE;
	GLdouble top = GRID_SIZE / 2 * GRID_EDGE;

	// put the vertices for horizontal lines in
	for (int i = 0; i < GRID_SIZE + 1; i++) {
		gridVertices[6 * i] = left;
		gridVertices[6 * i + 1] = top - GRID_EDGE * i;
		gridVertices[6 * i + 2] = 0;
		gridVertices[6 * i + 3] = right;
		gridVertices[6 * i + 4] = top - GRID_EDGE * i;
		gridVertices[6 * i + 5] = 0;

		gridColor[6 * i] = 0.22;
		gridColor[6 * i + 1] = 0.22;
		gridColor[6 * i + 2] = 0.22;

		gridColor[6 * i + 3] = 0.22;
		gridColor[6 * i + 4] = 0.22;
		gridColor[6 * i + 5] = 0.22;
	}

	// put the vertices for vertical lines in
	int start = (GRID_SIZE + 1) * 2 * 3;
	for (int i = 0; i < GRID_SIZE + 1; i++) {
		gridVertices[start + 6 * i] = left + GRID_EDGE * i;
		gridVertices[start + 6 * i + 1] = top;
		gridVertices[start + 6 * i + 2] = 0;
		gridVertices[start + 6 * i + 3] = left + GRID_EDGE * i;
		gridVertices[start + 6 * i + 4] = bottom;
		gridVertices[start + 6 * i + 5] = 0;

		gridColor[start + 6 * i] = 0.22;
		gridColor[start + 6 * i + 1] = 0.22;
		gridColor[start + 6 * i + 2] = 0.22;
										 
		gridColor[start + 6 * i + 3] = 0.22;
		gridColor[start + 6 * i + 4] = 0.22;
		gridColor[start + 6 * i + 5] = 0.22;
	}

}

void drawGrid() {

	glPushAttrib(GL_LINE_BIT);
	//glLineWidth(0.3); 

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, gridVertices);
	glColorPointer(3, GL_DOUBLE, 0, gridColor);
	for (int i = 0; i < (GRID_SIZE + 1) * 2; i++) {
		glDrawArrays(GL_LINES, 2 * i, 2);
	}
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopAttrib();


}

void drawCoord() {
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_LINE_STIPPLE);
	glPushAttrib(GL_LINE_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_DOUBLE, 0, coordVertices);
	//glColorPointer(3, GL_DOUBLE, 0, color);
	glLineWidth(3);
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, 2, 2);
	glDrawArrays(GL_LINES, 4, 2);


	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
	glPopAttrib();
	glDisable(GL_LINE_STIPPLE);
	glDisable(GL_LINE_SMOOTH);

}

void tobuffer(MyCube cubek) {
	//std::cout << "in the to buffer\n";
	for (int i = 0; i < 8; i++) {
		//std::cout << "point " << i << ": ";
		for (int j = 0; j < 3; j++) {
			myCube_vertex[i * 3 + j] = cubek.cube[i].Pos[j];
		}
	}
	
}

int stepCheck(MyCube cubek) {
	int flag = 0;
	int loops;
	for (int i = 0; i < 7; i++) {
		if (glm::length(cubek.cube[i].Acc) >= 30) {
			flag = 1;
			break;
		}
	}
	if (flag == 1) {
		dtime = 0.000001;
		loops = 400;
	}if (flag == 0) {
		dtime = 0.00001;
		loops = 100;
	}
	return loops;
}

void Spinz(double v) {
	//spin_v
	glm::dvec3 v0 = { -1,1,0 };
	glm::dvec3 v1 = { -1,-1,0 };
	glm::dvec3 v2 = { 1,-1,0 };
	glm::dvec3 v3 = { 1,1,0 };

	for (int i = 0; i < 2; i++) {
		spin_v.push_back(v0 * v);
		spin_v.push_back(v1 * v);
		spin_v.push_back(v2 * v);
		spin_v.push_back(v3 * v);
	}
}

void Spinx(double v) {
	//spin_v
	glm::dvec3 v0 = { 0,1,1 };
	glm::dvec3 v1 = { 0,-1,1 };
	glm::dvec3 v2 = { 0,-1,-1 };
	glm::dvec3 v3 = { 0,1,-1 };


	spin_v.push_back(v0 * v);
	spin_v.push_back(v0 * v);
	spin_v.push_back(v3 * v);
	spin_v.push_back(v3 * v);
	spin_v.push_back(v1 * v);
	spin_v.push_back(v1 * v);
	spin_v.push_back(v2 * v);
	spin_v.push_back(v2 * v);

}

void Spiny(double v) {
	//spin_v
	glm::dvec3 v0 = {-1,0,1 };
	glm::dvec3 v1 = { 1,0,1 };
	glm::dvec3 v2 = { 1,0,-1 };
	glm::dvec3 v3 = { -1,0,-1 };

	spin_v.push_back(v3 * v);
	spin_v.push_back(v0 * v);
	spin_v.push_back(v0 * v);
	spin_v.push_back(v3 * v);
	spin_v.push_back(v2 * v);
	spin_v.push_back(v1 * v);
	spin_v.push_back(v1 * v);
	spin_v.push_back(v2 * v);
	

}