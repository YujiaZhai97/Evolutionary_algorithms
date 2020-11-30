// Include standard headersxxxxxxxxxxxxxx
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
#include<string>
#include<sstream>

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
#include "MyCubes.h"
#include"geneticAlgorithm.h"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768
#define GRID_SIZE 40
#define GRID_EDGE 0.2

const double LONGEST_R = 2.0;
const double SHORTEST_R = 0.5;
double dtime = 0.0002;	// s0.0004
const double LOrigin = 0.1;		// m
const double K0 = 1000.0;		// k = F/dx (N/m) 500
const double Kground = 4000.0;
double damp = 0.9999;         // update v = 0.9*v
const glm::dvec3 gravity = { 0.0, 0.0, -9.8 }; // z -9.8 N/Kg
const double mu0 = 1;			// mu = Ff/Fn
const double mass0 = 0.1;		// 0.1kg
const double sqrt2 = sqrt(2);
const double sqrt3 = sqrt(3);
const double a0 = 0.1;
const double b0 = 0.02;
const double b1 = -0.1;
const double c0 = 0;
const int robot_num = 30;
double omega = 1.5;

// parameters for evolve
double K[3] = { K0,K0,K0 };
double b[4] = { b0,b0,b0,b0 };
double c[8];
double weight_x = 1;
double weight_y = 1;
double weight_z = 1;


const int cube_count = 3;
std::vector<double> diversities;

int shade_order[] = {
	0,1,2
};

GLdouble gridVertices[(GRID_SIZE + 1) * (GRID_SIZE + 1) * 3];
GLdouble gridColor1[(GRID_SIZE + 1) * (GRID_SIZE + 1) * 3];
GLdouble gridColor2[(GRID_SIZE + 1) * (GRID_SIZE + 1) * 3];
GLuint gridindices1[GRID_SIZE* GRID_SIZE*3];
GLuint gridindices2[GRID_SIZE * GRID_SIZE * 3];

GLdouble myCube_vertex[cube_count * 24 * robot_num]; // cube_count * 8 points * 3 (x, y, z)
GLdouble myCube_color[cube_count * 24 * robot_num]; // cube_count * 8 points * 3 bit
GLuint myCubeindices[cube_count * 36 * robot_num];   // cube_count * 6 faces * 2 triangles * 3 indices
GLdouble myEdge_color[cube_count * 24 * robot_num];      // cube_count * 8 points * 3 bit
GLuint myEdge_indices[cube_count * 24 * robot_num]; // cube_count * 12 * 2

GLdouble myShade_vertex[cube_count * 24 * robot_num];
GLdouble myShade_color[cube_count * 24 * robot_num];
GLuint myShadeindices[cube_count * 36 * robot_num];

const GLdouble coordVertices[] = {
	0, 0, 0,
	0.15, 0, 0,

	0, 0, 0,
	0, 0.15, 0,

	0, 0, 0,
	0, 0, 0.15
};

const GLdouble CoordColor[]{
	1.0, 0, 0,
	1.0, 0, 0,

	0, 1.0, 0,
	0, 1.0, 0,

	0, 0, 1.0,
	0, 0, 1.0
};

GLdouble structure_base[] = {
	0, 0, 0.05,		    // 0
	0.1, 0, 0.05,			// 1
	0, 0.1, 0.05			// 3
	//0, 1, 1.5,			// 4
	//-1, 1, 1.5,			// 5
	//1, 1, 1.5,			// 6
	//-1, -1,0.5,			// 7
	//1, -1, 0.5,			// 8
	//-1, 1, 0.5,			// 9
	//1, 1, 0.5			// 10
};

GLdouble structure[3*cube_count];



std::vector<MyCubes> robotGroup;

void gridVertGen();
void drawCoord();
int stepCheck(MyCubes cubek);


int main(void) {
	
	double timeori;
	double timepre;
	double timecur;
	double time_sim = 0;
	int loops = 100;
	int num_point;
	std::ofstream output;
	double diverse;

#pragma region Initiate robots
	double length = 0.7 * 9;
	double width =  (ceil(robot_num/10));
	for (int i = 0; i < robot_num; i++) {
		randomInit();
		glm::dvec3 t = { (-length / 2 + (i % 10) * 0.7),(0.7 * (int(i / 10) - width/2)),0.01 };
		MyCubes myCubek(structure_base, i, t);
		robotGroup.push_back(myCubek);
	}

	std::vector<glm::dvec3> extForce;
	glm::dvec3 e = { 0,0,0 };
	num_point = robotGroup[0].points.size();
	for (int i = 0; i < num_point; i++) {
		extForce.push_back(e);
	}
//	
#pragma endregion
//
//
	for (int run = 0; run < 1; run++) {
		
		
		for (int round = 0; round < 60; round++) {

			for (int ite = 0; ite < 20; ite++) {
#pragma region Simulation
				std::cout << "======= run " << run << " round " << round << " ite " << ite<<" ==========\n";
				
				double span = 5; // 2*M_PI*4/omega
				int count = 0;
				while (time_sim <= span) {

					for (int i = 0; i < robot_num; i++) {
						robotGroup[i].Update(extForce);
						robotGroup[i].Breath(time_sim);
						
					}

					if (count >= 5000) {
						std::cout << time_sim << "\n";
						count = 0;
					}
					count++;
					time_sim += dtime;
					
				}
				//std::cout << "end time_sim " << time_sim << "\n";
				//std::cout << "after simulation, all lengths: \n";
				for (int i = 0; i < robot_num; i++) {
					robotGroup[i].distance = robotGroup[i].pathLength();
				//	std::cout << robotGroup[i].distance << " ";
				}
				time_sim = 0;
				//std::cout << "\n";


#pragma endregion

#pragma region Evolve
				if (ite != 0 && round != 0) {
					select();
				}

				crossOver();

				for (int i = 1; i < robot_num; i++) {
					mutation(i);
				}


#pragma endregion
				diverse = diversity();
				diversities.push_back(diverse);
				
				// record this generation to a txt file
				// including the 'distance' of top 3 individuals
				output.open("bestCubeRecord_" + std::to_string(run) +".txt", std::fstream::app);
				for (int i = 0; i < 1; i++) {
					output << robotGroup[i].distance << " " << robotGroup[i].EK << " " << robotGroup[i].EP << " " 
						<< robotGroup[i].startPos[0] << " " << robotGroup[i].startPos[1] << " " << robotGroup[i].startPos[2] << " "
						<< robotGroup[i].endPos[0] << " " << robotGroup[i].endPos[1] << " " << robotGroup[i].endPos[2] <<" "
						<< robotGroup[i].MyK[0] << " " << robotGroup[i].MyK[1] << " " << robotGroup[i].MyK[2]<<" " 
						<< robotGroup[i].Myb[0] << " " << robotGroup[i].Myb[1] << " " << robotGroup[i].Myb[2] << " " << robotGroup[i].Myb[3] << " "
						<< robotGroup[i].Myc[0] << " " << robotGroup[i].Myc[1] << " " << robotGroup[i].Myc[2] << " " << robotGroup[i].Myc[3] << " " 
						<< robotGroup[i].Myc[4] << " " << robotGroup[i].Myc[5] << " " << robotGroup[i].Myc[6] << " " << robotGroup[i].Myc[7] << " " 
						<<robotGroup[i].w_x<<" " << robotGroup[i].w_y << " " << robotGroup[i].w_z << "\n";
				}
				output.close();

			}
		
			output.open("diverse_" + std::to_string(run) +".txt", std::fstream::app);
			for (double d : diversities) {
				output << d << "\n";
			}
			output.close();
			diversities.clear();
		
		}
	
		std::cout << "the longest distance for the best robot to go:" << robotGroup[0].distance << "\n";
		std::cout << "K: " << robotGroup[0].MyK[0] << " " << robotGroup[0].MyK[1] << " " << robotGroup[0].MyK[2] << "\n";
		std::cout << "b: " << robotGroup[0].Myb[0] << " " << robotGroup[0].Myb[1] << " " << robotGroup[0].Myb[2] << "\n";
		std::cout << "c: " << "\n";
		for (int i = 0; i < 8; i++) {
			std::cout << robotGroup[0].Myc[i] << " ";
		}
		std::cout << "\n";
		
		for (int i = 0; i < robot_num; i++) {
			glm::dvec3 t = { (-length / 2 + (i % 10) * 0.4 + 0.3),(0.5 * (int(i / 10) - width/2)),0.05 };
			robotGroup[i].restart(structure_base,t);
		}
	}


	for (int i = 0; i < robot_num; i++) {
		robotGroup[i].toBuffer(i);
	}


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
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Assignment 3-Phase B-Bouncing ", NULL, NULL);


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
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	// VAO vertex array object
	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);

	// Camera matrix
	glm::mat4 View = glm::lookAt(
		glm::vec3(4, 1, 2), // Camera is at (4,3,3), in World Space
		glm::vec3(0, 0, 0), // and looks at the origin
		glm::vec3(0, 0, 1)  // Head is up (set to 0,-1,0 to look upside-down)
	);
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP = Projection * View * Model; // Remember, matrix multiplication is the other way around
#pragma endregion

	gridVertGen();
	
#pragma region VBOs
	// VBO vertex buffer object
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gridVertices), gridVertices, GL_STATIC_DRAW);

	GLuint colorbuffer;
	glGenBuffers(1, &colorbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gridColor1), gridColor1, GL_STATIC_DRAW);

	GLuint colorbuffer2;
	glGenBuffers(1, &colorbuffer2);
	glBindBuffer(GL_ARRAY_BUFFER, colorbuffer2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(gridColor2), gridColor2, GL_STATIC_DRAW);

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
	glBufferData(GL_ARRAY_BUFFER, sizeof(myCube_color), myCube_color, GL_STATIC_DRAW);

	GLuint colorbufferEdge;
	glGenBuffers(1, &colorbufferEdge);
	glBindBuffer(GL_ARRAY_BUFFER, colorbufferEdge);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myEdge_color), myEdge_color, GL_STATIC_DRAW);

	GLuint vertexbufferShade;
	glGenBuffers(1, &vertexbufferShade);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufferShade);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myShade_vertex), myShade_vertex, GL_DYNAMIC_DRAW);

	GLuint colorbufferShade;
	glGenBuffers(1, &colorbufferShade);
	glBindBuffer(GL_ARRAY_BUFFER, colorbufferShade);
	glBufferData(GL_ARRAY_BUFFER, sizeof(myShade_color), myShade_color, GL_STATIC_DRAW);
#pragma endregion

#pragma region EBOs
	// EBO Element buffer -- myCubeindices
	GLuint EBOCube;
	glGenBuffers(1, &EBOCube);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOCube);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(myCubeindices), myCubeindices, GL_STATIC_DRAW);

	GLuint EBOEdge;
	glGenBuffers(1, &EBOEdge);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOEdge);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(myEdge_indices), myEdge_indices, GL_STATIC_DRAW);

	GLuint EBOG1;
	glGenBuffers(1, &EBOG1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOG1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridindices1), gridindices1, GL_STATIC_DRAW);

	GLuint EBOG2;
	glGenBuffers(1, &EBOG2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOG2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gridindices2), gridindices2, GL_STATIC_DRAW);


#pragma endregion

	
	timepre = glfwGetTime();
	timeori = timepre;
	int count = 0;
	time_sim = 0;
	/// START THE LOOP!!!!!!
	do {
		std::cout << "============================round " << count <<" time:"<<time_sim<< " ========================\n";
		
		//std::cout << "\n";
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
#pragma region Grid Color1
		
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_DOUBLE,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0           // array buffer offset
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
			(void*)0                     // array buffer offset
		);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOG1);
		glDrawElements(GL_TRIANGLES, 3 * GRID_SIZE*GRID_SIZE, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * 0));

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_DOUBLE,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0           // array buffer offset
		);
		// 2nd attribute buffer : colors
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer2);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			3,                                // size
			GL_DOUBLE,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                     // array buffer offset
		);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOG2);
		glDrawElements(GL_TRIANGLES, 3 * GRID_SIZE * GRID_SIZE, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * 0));

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

#pragma endregion

#pragma region Shade
		// 1st attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferShade);
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
		glBindBuffer(GL_ARRAY_BUFFER, colorbufferShade);
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
		glDrawElements(GL_TRIANGLES, 36*cube_count*robot_num, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * 0));


		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
#pragma endregion
		for (int rbt = 0; rbt < robot_num; rbt++) {
			for (int cb = 0; cb < cube_count; cb++) {

				int order = shade_order[cb];
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
				//glDrawElements(GL_TRIANGLES, 72, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * 36));
				glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * (36 * order + rbt * 36 * cube_count)));


				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);
#pragma endregion

#pragma region Edge

				//// 1st attribute buffer : vertices
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
				glBindBuffer(GL_ARRAY_BUFFER, colorbufferEdge);
				glVertexAttribPointer(
					1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
					3,                                // size
					GL_DOUBLE,                         // type
					GL_FALSE,                         // normalized?
					0,                                // stride
					(void*)0                          // array buffer offset
				);
				// 3rd element buffer: indices
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOEdge);

				glDrawElements(GL_LINES, 2 * 12, GL_UNSIGNED_INT, (GLvoid*)(sizeof(GLuint) * (24 * order+rbt*24*cube_count)));

				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);

#pragma endregion
			}
	}
		
		
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

		glDrawArrays(GL_LINES, 0, 2);
		glDrawArrays(GL_LINES, 2, 2);
		glDrawArrays(GL_LINES, 4, 2);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
#pragma endregion

		//loops = stepCheck(myCubek);
		for (int tt = 0; tt < loops; tt++) {
			for (int rr = 0; rr < robot_num; rr++) {
				robotGroup[rr].Breath(time_sim);
				robotGroup[rr].Update(extForce);
				
				time_sim += dtime;
			}
		}
		std::cout << robotGroup[0].points[9].Pos[0] << " " << robotGroup[0].points[9].Pos[1] << " " << robotGroup[0].points[9].Pos[2] << "\n";
		std::cout << robotGroup[0].points[9].Vel[0] << " " << robotGroup[0].points[9].Vel[1] << " " << robotGroup[0].points[9].Vel[2] << "\n";
		std::cout << robotGroup[0].points[9].Acc[0] << " " << robotGroup[0].points[9].Acc[1] << " " << robotGroup[0].points[9].Acc[2] << "\n";
		std::cout << robotGroup[0].points[9].Force[0] << " " << robotGroup[0].points[9].Force[1] << " " << robotGroup[0].points[9].Force[2] << "\n";
		std::cout << robotGroup[0].EK <<" + "<< robotGroup[0].EP << " = " <<robotGroup[0].E << "\n";


		timecur = glfwGetTime();

		timepre = timecur;
		

		for (int i = 0; i < robot_num; i++) {
			robotGroup[i].toBuffer(i);
		}
		
		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferCube);
		glBufferData(GL_ARRAY_BUFFER, sizeof(myCube_vertex), myCube_vertex, GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, vertexbufferShade);
		glBufferData(GL_ARRAY_BUFFER, sizeof(myShade_vertex), myShade_vertex, GL_DYNAMIC_DRAW);
		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

		count++;

		//std::this_thread::sleep_for(std::chrono::nanoseconds(100000000));
	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteBuffers(1, &vertexbufferCoord);
	glDeleteBuffers(1, &colorbufferCoord);
	glDeleteBuffers(1, &vertexbufferCube);
	glDeleteBuffers(1, &colorbufferCube);
	glDeleteBuffers(1, &EBOCube);
	glDeleteBuffers(1, &EBOEdge);
	glDeleteBuffers(1, &colorbufferEdge);
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
		for (int j = 0; j < GRID_SIZE + 1; j++) {
			gridVertices[(GRID_SIZE + 1) * i * 3 + j * 3 + 0] = left + GRID_EDGE * j;
			gridVertices[(GRID_SIZE + 1) * i * 3 + j * 3 + 1] = top - GRID_EDGE * i;
			gridVertices[(GRID_SIZE + 1) * i * 3 + j * 3 + 2] = 0;
			

			gridColor1[(GRID_SIZE + 1) * i * 3 + j * 3 + 0] = 0.9;
			gridColor1[(GRID_SIZE + 1) * i * 3 + j * 3 + 1] = 0.9;
			gridColor1[(GRID_SIZE + 1) * i * 3 + j * 3 + 2] = 0.9;
					  
			gridColor2[(GRID_SIZE + 1) * i * 3 + j * 3 + 0] = 0.5;
			gridColor2[(GRID_SIZE + 1) * i * 3 + j * 3 + 1] = 0.5;
			gridColor2[(GRID_SIZE + 1) * i * 3 + j * 3 + 2] = 0.9;
		}		
	}

	for (int i = 0; i < GRID_SIZE; i++) {
		for (int j = 0; j < GRID_SIZE/2; j++) {
			if (i % 2 == 0) {
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 0] = (GRID_SIZE + 1) * i + 2 * j;
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 1] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 1;
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 2] = (GRID_SIZE + 1) * i + 2 * j +1;
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 3] = (GRID_SIZE + 1) * i + 2 * j +1;
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 4] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 1;
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 5] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 2;

				gridindices2[GRID_SIZE * 3 * i + j * 6 + 0] = (GRID_SIZE + 1) * i + 2 * j + 1;
				gridindices2[GRID_SIZE * 3 * i + j * 6 + 1] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 2;
				gridindices2[GRID_SIZE * 3 * i + j * 6 + 2] = (GRID_SIZE + 1) * i + 2 * j + 2;
				gridindices2[GRID_SIZE * 3 * i + j * 6 + 3] = (GRID_SIZE + 1) * i + 2 * j + 2;
				gridindices2[GRID_SIZE * 3 * i + j * 6 + 4] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 2;
				gridindices2[GRID_SIZE * 3 * i + j * 6 + 5] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 3;
			} else {
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 0] = (GRID_SIZE + 1) * i + 2 * j + 1;
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 1] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 2;
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 2] = (GRID_SIZE + 1) * i + 2 * j + 2;
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 3] = (GRID_SIZE + 1) * i + 2 * j + 2;
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 4] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 2;
				gridindices1[GRID_SIZE * 3 * i + j * 6 + 5] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 3;

				gridindices2[GRID_SIZE * 3 * i + j * 6 + 0] = (GRID_SIZE + 1) * i + 2 * j;
				gridindices2[GRID_SIZE * 3 * i + j * 6 + 1] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 1;
				gridindices2[GRID_SIZE * 3 * i + j * 6 + 2] = (GRID_SIZE + 1) * i + 2 * j + 1;
				gridindices2[GRID_SIZE * 3 * i + j * 6 + 3] = (GRID_SIZE + 1) * i + 2 * j + 1;
				gridindices2[GRID_SIZE * 3 * i + j * 6 + 4] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 1;
				gridindices2[GRID_SIZE * 3 * i + j * 6 + 5] = (GRID_SIZE + 1) * i + 2 * j + GRID_SIZE + 2;
			}

		}
	}
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

int stepCheck(MyCubes cubek) {
	int flag = 0;
	int loops;
	for (int i = 0; i < 7; i++) {
		if (glm::length(cubek.points[i].Acc) >= 10) {
			flag = 1;
			break;
		}
	}
	if (flag == 1) {
		dtime = 0.0001;
		loops = 100;
	}if (flag == 0) {
		dtime = 0.001;
		loops = 100;
	}
	return loops;
}

