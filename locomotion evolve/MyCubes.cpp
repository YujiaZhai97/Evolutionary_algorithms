#include "MyCubes.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <math.h>

#include <algorithm>

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
extern const double sqrt2;
extern const double sqrt3;
extern const double a0;
extern const double b0;
extern const double c0;
extern double b[];
extern double c[];
extern double omega;
extern const int robot_num;

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
extern GLdouble myEdge_color[];      // cube_count * 12 * 2 * 3 bit
extern GLuint myEdge_indices[]; // cube_count * 12 * 2

extern GLdouble myShade_vertex[];
extern GLdouble myShade_color[];
extern GLuint myShadeindices[];

#pragma region parameters for cube reference
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

int existCubes[] = {
	0,0,    // xy_plane bottom, top
	0,0,	// yz_plane back, front
	0,0		// xz_plane left, right
};

int nearByCube[] = {
	0, 3, 5,   // 0 point
	0, 2, 5,   // 1
	0, 2, 4,   // 2
	0, 3, 4,   // 3
	1, 3, 5,   // 4
	1, 2, 5,   // 5
	1, 2, 4,   // 6
	1, 3, 4	   // 7
};

int overlapDirection[] = {
	0,0,-1,      // according to the index order in existCubes
	0,0,1,
	-1,0,0,
	1,0,0,
	0,-1,0,
	0,1,0
};

int overlapxz[] = {
	3, 2, 1, 0,
	7, 6, 5, 4
};

int overlapxy[] = {
	4, 5, 6, 7,
	0, 1, 2, 3
};

int overlapyz[] = {
	1, 0, 3, 2,
	5, 4, 7, 6
};

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

int cube_triangle[] = {
	0,3,2,
	0,2,1,
	4,6,7,
	4,5,6,

	1,6,5,
	1,2,6,
	4,3,0,
	4,7,3,
	
	2,7,6,
	2,3,7,
	1,5,4,
	1,4,0


};

int edge_lines[] = {
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
#pragma endregion


MyCubes::MyCubes(GLdouble build[], int robot, glm::dvec3 trans) {
	int n = cube_count;
	int direction;
	int cor_cube;
	int cor_point;
	MyEle Cube2;
	int point2;
	int vertex_count = 0;
	double dist;
	glm::dvec3 weights;
	
	// 0. Record parameters for this robot
	for (int i = 0; i < 3; i++) {
		MyK[i] = K[i];
	}

	for (int i = 0; i < 4; i++) {
		Myb[i] = b[i];
	}

	for (int i = 0; i < 8; i++) {
		Myc[i] = c[i];
	}
	w_x = weight_x + 1;
	w_y = weight_y + 1;
	w_z = weight_z + 1;
	weights = {w_x, w_y, w_z};
	weights = glm::normalize(weights);

	double c_x = 0;
	double c_y = 0;
	double c_z = 0;
	for (int i = 0; i < cube_count; i++) {
		c_x += build[3 * i];
		c_y += build[3 * i + 1];
		c_z += build[3 * i + 2];
	}
	bodyCenter = { (c_x / cube_count + trans[0]) , (c_y / cube_count + trans[1]), (c_z / cube_count + trans[2]) };
	startPos = bodyCenter;
	
	// 1. each position of cube center
	for (int i = 0; i < n; i++) {
		MyEle tmpE;
		//std::cout << "==========cube Initial #" << i << " ===========\n";
		tmpE.index = i;
		tmpE.pos = { build[3 * i] + trans[0], build[3 * i + 1] + trans[1], build[3 * i + 2] + trans[2] };
		dist = glm::distance(bodyCenter, tmpE.pos);
		//std::cout << "distance of element i to (0,0,0) is " << dist << "\n";
		if ( dist<=0.01) {
			tmpE.color = { 0.4, 0.4, 0.9 };   // Blue
		} else if (dist > 0.01 && dist <0.03) {
			tmpE.color = { 0.1, 0.9, 0.2 };   // Green
		} else if (dist >= 0.03 && dist < 0.05) {
			tmpE.color = { 0.4, 0.4, 0.2 };
		} else {
			tmpE.color = { 0.9, 0.4, 0.4 };
		}
	
		
		checkExist(i, build);
		// update 8 points contained in this cube
		for (int j = 0; j < 8; j++) {
			// search for previous cubes
			// when previous cube overlap with this cube and 
			// uses the point we are dealing with
			// add the exist point index
			//std::cout << "++++++point process #" << j << "+++++++\n";
			if (existCubes[nearByCube[3 * j]] != -1) {
				// overlap in xy_plane
				//std::cout << "overlap in xy_plane\n";
				cor_cube = existCubes[nearByCube[3 * j]];  // the cube index it overlap
				cor_point = overlapxy[j];				   // the point index it overlap
				Cube2 = elements[cor_cube];
				point2 = Cube2.pointId[cor_point];        // point index in vector points

				tmpE.pointId[j] = point2;

			} 
			else if (existCubes[nearByCube[3 * j + 1]] != -1) {
				// overlap in yz_plane
				//std::cout << "overlap in yz_plane\n";
				cor_cube = existCubes[nearByCube[3 * j + 1]];  // the cube index it overlap
				cor_point = overlapyz[j];				   // the point index it overlap
				Cube2 = elements[cor_cube];
				point2 = Cube2.pointId[cor_point];        // point index in vector points
				tmpE.pointId[j] = point2;
			} 
			else if (existCubes[nearByCube[3 * j + 2]] != -1) {
				// overlap in xz_plane
				cor_cube = existCubes[nearByCube[3 * j + 2]];  // the cube index it overlap
				cor_point = overlapxz[j];				   // the point index it overlap
				Cube2 = elements[cor_cube];
				point2 = Cube2.pointId[cor_point];        // point index in vector points

				tmpE.pointId[j] = point2;
			} 
			else {
				// this point is new
				// we initiate this point
				
				MyPoint tmpP;
				tmpP.mass = mass0;
				tmpP.index = vertex_count;
				tmpP.Pos[0] = trans[0] + build[3 * i] + relative[3 * j] * LOrigin;
				tmpP.Pos[1] = trans[1] + build[3 * i + 1] + relative[3 * j + 1] * LOrigin;
				tmpP.Pos[2] = trans[2] + build[3 * i + 2] + relative[3 * j + 2] * LOrigin;
				tmpP.Acc = glm::dvec3{ 0,0,0 };
				tmpP.Force = gravity * mass0;
				tmpP.startPos = tmpP.Pos;
				points.push_back(tmpP);
				tmpE.pointId[j] = vertex_count;
				vertex_count++;

			}
		}
		elements.push_back(tmpE);
	}

	// 2. initiate all springs according to cube 
	//std::cout << "2. initiate all springs according to cube\n";
	std::vector<springGroup> grp;
	int spring_count = 0;
	int edge_count = 0;
	int face_count = 0;
	int internal_count = 0;
	for (int i = 0; i < n; i++) {
		//std::cout << "===========process cube " << i << "==========\n";
		int aa; // index of a point in points
		int bb; // index of b point in points
		springGroup tmpab;
		edge_count = 0;
		face_count = 0;
		internal_count = 0;
		// 2.1 traverse every point in this cube
		for (int j = 0; j < 8; j++) {
			aa = elements[i].pointId[j];
			// 2.1.1 traverse every connected point in this cube, according the conn[] array
			for (int k = 0; k < 7; k++) {
		//		// ignore the done pair because alwayse the point with smaller cube_index find the larger one
		//		// this can filter half of iteration from 7*8=56 ->28
				
				bb = elements[i].pointId[conn[7 * j + k]];
				if (aa < bb) {
					tmpab.a = aa;
					tmpab.b = bb;
				}
				else {
					tmpab.a = bb;
					tmpab.b = aa;
				}

				if (findGroup(grp, tmpab)) {
					// exist this spring!
					// update Mypoint.P_bar[j]
					// update MyEle.edgeSpring[], or faceSpring, or internalSpring
					int spring_idx = findIdx(tmpab, springs);
					if (points[aa].P_bar.size() != 0) {
						for (int pb = 0; pb < points[aa].P_bar.size(); pb++) {
							if (points[aa].P_bar[pb] == spring_idx) {
								break;
							}if (pb == points[aa].P_bar.size() - 1 && points[aa].P_bar[pb] != spring_idx) {
								points[aa].P_bar.push_back(spring_idx);
							}
						}
					} else if (points[aa].P_bar.size() == 0) {
						points[aa].P_bar.push_back(spring_idx);
					}
					
					
					if (k < 3) {
						elements[i].edgeSpring[edge_count] = spring_idx;
						edge_count++;
					} else if (k >= 3 && k < 6) {
						elements[i].faceSpring[face_count] = spring_idx;
						face_count++;
					} else {
						elements[i].intenalSpring[internal_count] = spring_idx;
						internal_count++;
					}
					
				}
				else {
					// add a new spring
					MySpring tmpSpring;
					glm::dvec3 edgeCenter = weights * (points[tmpab.a].Pos + points[tmpab.b].Pos) * 0.5;
					dist = glm::distance(bodyCenter, edgeCenter);
					
					//int prt;

					if (edgeCenter[0] >= 0) {
						if (edgeCenter[1] >= 0) {
							tmpSpring.b = b[0];
							if (edgeCenter[1] >= 0) {
								tmpSpring.c = c[0];
							} else {
								tmpSpring.c = c[1];
							}
						} else if (edgeCenter[1] < 0) {
							tmpSpring.b = b[1];
							if (edgeCenter[1] >= 0) {
								tmpSpring.c = c[2];
							} else {
								tmpSpring.c = c[3];
							}
						}
					} else if (edgeCenter[0] < 0) {
						if (edgeCenter[1] >= 0) {
							tmpSpring.b = b[2];
							if (edgeCenter[1] >= 0) {
								tmpSpring.c = c[4];
							} else {
								tmpSpring.c = c[5];
							}
						} else if (edgeCenter[1] < 0) {
							tmpSpring.b = b[3];
							if (edgeCenter[1] >= 0) {
								tmpSpring.c = c[6];
							} else {
								tmpSpring.c = c[7];
							}
						}
					}

					tmpSpring.index = spring_count;
					if (dist < 0.06) {
						tmpSpring.K = K[0];
					} else if(dist > 0.1){
						tmpSpring.K = K[1];
					} else {
						tmpSpring.K = K[2];
					}
					
					tmpSpring.L = glm::distance(points[tmpab.a].Pos, points[tmpab.b].Pos);
					tmpSpring.L0 = tmpSpring.L;
					tmpSpring.a = tmpSpring.L;
					tmpSpring.group = tmpab;
					// std::cout << "new spring!!" << spring_count << "\n";
					// std::cout << dist << "\n";

					points[aa].P_bar.push_back(spring_count);
					if (k < 3) {
						// update the 
						elements[i].edgeSpring[edge_count] = spring_count;
						edge_count++;
					} else if (k >= 3 && k < 5) {
						elements[i].faceSpring[face_count] = spring_count;
						face_count++;
					} else {
						elements[i].intenalSpring[internal_count] = spring_count;
						internal_count++;
					}
					grp.push_back(tmpab);
					springs.push_back(tmpSpring);
					spring_count++;
				}
				///std::cout << elements[i].index << " ->"<<elements[i+1].index << "\n";
			}
			//std::cout << "current element index is " << elements[i].index << ", next index is " << elements[i+1].index << "\n";
		}
	}
	// put vertex of every cube into the buffer
	// the overlap vertex are considered as different points
	n = elements.size();
	//std::cout << "put all data into buffer, sizi of elements is: " << n << "\n";
	int point_idx;
	int idx;
#pragma omp parallel for
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < 8; j++) {
			point_idx = elements[i].pointId[j];
			idx = n * 24 * robot + 24 * i + 3 * j;
			myCube_vertex[idx + 0] = points[point_idx].Pos[0];
			myCube_vertex[idx + 1] = points[point_idx].Pos[1];
			myCube_vertex[idx + 2] = points[point_idx].Pos[2];

			myShade_vertex[idx + 0] = points[point_idx].Pos[0];
			myShade_vertex[idx + 1] = points[point_idx].Pos[1];
			myShade_vertex[idx + 2] = 0;

			myCube_color[idx + 0] = elements[i].color[0];
			myCube_color[idx + 1] = elements[i].color[1];
			myCube_color[idx + 2] = elements[i].color[2];

			myShade_color[idx + 0] = 0.3;
			myShade_color[idx + 1] = 0.3;
			myShade_color[idx + 2] = 0.3;

			myEdge_color[idx + 0] = 0.1;
			myEdge_color[idx + 1] = 0.1;
			myEdge_color[idx + 2] = 0.1;
		}
	}
	// element buffer recording the connected points 
	// myCubeindices
	int ele_idx;
	int edge_idx;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < 36; j++) {
			ele_idx = cube_triangle[j];
			myCubeindices[n * 36 * robot +36 * i + j] =  8 * (i+n*robot) + cube_triangle[j];
			myShadeindices[n * 36 * robot + 36 * i + j] = 8 * (i + n * robot) + cube_triangle[j];
		}
		for (int j = 0; j < 24; j++) {
			if (j == 2 || j == 3 || j == 4 || j == 5 || j == 12 || j == 13) {
				myEdge_indices[n * 24 * robot + 24 * i + j] = 0;
				continue;
			}
			myEdge_indices[n * 24 * robot + 24 * i + j] = 8 * (i + n * robot) + edge_lines[j];
		}
	}
	
}


// put all updated position, color to buffer
// myCube_vertex[];
// myCube_color[];
// myPoints_color[];

// not finished
void MyCubes::toBuffer(int robot) {
	 // put vertex of every cube into the buffer
	 // the overlap vertex are considered as different points
	
	int n = elements.size();
	//std::cout << "put all data into buffer, sizi of elements is: " << n << "\n";
	int point_idx;
	int idx;
#pragma omp parallel for
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < 8; j++) {
			point_idx = elements[i].pointId[j];
			idx = cube_count * 24 * robot + 24 * i + 3 * j;
			myCube_vertex[idx + 0] = points[point_idx].Pos[0];
			myCube_vertex[idx + 1] = points[point_idx].Pos[1];
			if (points[point_idx].Pos[2] <= 0) {
				myCube_vertex[idx + 2] = 0;
			} else {
				myCube_vertex[idx + 2] = points[point_idx].Pos[2];
			}

			myShade_vertex[idx + 0] = points[point_idx].Pos[0];
			myShade_vertex[idx + 1] = points[point_idx].Pos[1];
			myShade_vertex[idx + 2] = 0;

		}
	}

	
}

// modify the flag existCubes[],
// if exist nearby cube, record the index of this cube
// otherwise, -1
void MyCubes::checkExist(int index, GLdouble build[]) {
	GLdouble x = build[3 * index];
	GLdouble y = build[3 * index + 1];
	GLdouble z = build[3 * index + 2];
	GLdouble xx;
	GLdouble yy;
	GLdouble zz;
	
	for (int i = 0; i < 6; i++) {
		existCubes[i] = -1;
	}

	for (int i = index - 1; i >= 0; i--) {
		xx = build[3 * i];
		yy = build[3 * i + 1];
		zz = build[3 * i + 2];
		if (x == xx && y == yy && z - 0.1 == zz) {
			existCubes[0] = i;		// xy_plane bottom
			continue;
		}

		if (x == xx && y == yy && z + 0.1 == zz) {
			existCubes[1] = i;		// xy_plane top
			continue;
		}

		if (x - 0.1 == xx && y == yy && z == zz) {
			existCubes[2] = i;		// yz_plane back
			continue;
		}

		if (x + 0.1 == xx && y == yy && z == zz) {
			existCubes[3] = i;		// yz_plane front
			continue;
		}

		if (x == xx && y - 0.1 == yy && z == zz) {
			existCubes[4] = i;		// xz_plane left
			continue;
		}

		if (x == xx && y + 0.1 == yy && z == zz) {
			existCubes[5] = i;		// xz_plane right
			continue;
		}
	}
}

int MyCubes::findIdx(springGroup ab, std::vector<MySpring> springset) {
	
	int pos = 0;
	for (MySpring i : springset) {
		if ((i.group.a == ab.a && i.group.b == ab.b) || (i.group.a == ab.b && i.group.b == ab.a)) {
			return pos;
		}
		pos++;
	}

}

bool MyCubes::findGroup(std::vector<springGroup> grp, springGroup tmpab) {
	int n = grp.size();
	bool find = false;
	for (int i = 0; i < n; i++) {
		if ( (grp[i].a == tmpab.a && grp[i].b == tmpab.b) || (grp[i].a == tmpab.b && grp[i].b == tmpab.a)) {
			
			return true;
		}
	}
	return false;
}

void MyCubes::Update(std::vector<glm::dvec3> extForce) {
	if (fail == true) {
		return;
	}
	int num_point = points.size();
	int num_spring = springs.size();
	int num_ele = elements.size();

	glm::dvec3 Fspring = { 0,0,0 };
	glm::dvec3 Fground = { 0,0,0 };
	glm::dvec3 Ffriction = { 0,0,0 };
	glm::dvec3 L;
	glm::dvec3 directionf;

	glm::dvec3 pnta;
	glm::dvec3 pntb;

#pragma omp parallel for
	for (int i = 0; i < num_spring; i++) {
		// update springs'
		
		pnta = points[springs[i].group.a].Pos;
		pntb = points[springs[i].group.b].Pos;
		springs[i].L = glm::distance(pnta, pntb);
		if (springs[i].L > 8 * LONGEST_R * springs[i].a) {
			endPos = startPos;
			fail = true;
			return;
		}
		springs[i].F = (springs[i].L - springs[i].L0) * springs[i].K;
		
	}

	

#pragma omp parallel for
	for (int i = 0; i < num_point; i++) {
		Fspring = mySpringForce(i);  
		Ffriction = glm::dvec3{ 0,0,0 };
		Fground = glm::dvec3{ 0,0,0 };
		// update forces

		if (points[i].Pos[2] <= 0) {
			if (points[i].Vel[0] == 0 && points[i].Vel[1] == 0) {

				if (points[i].Force[0] == 0 && points[i].Force[1] == 0) {
					directionf = glm::dvec3(0, 0, 0);
				} else {
					directionf = glm::normalize(glm::dvec3(-points[i].Force[0], -points[i].Force[1], 0));
				}
				
			} else {
				directionf = glm::normalize(glm::dvec3(-points[i].Vel[0], -points[i].Vel[1], 0));
			}
			Ffriction = directionf * points[i].Force[2] * mu0;
			Fground = glm::dvec3(0, 0, -1 * points[i].Pos[2] * Kground);
		}

		points[i].Force = extForce[i] + Ffriction + Fground + Fspring + gravity * points[i].mass;
		//damp = 1.0;
		if (points[i].fixed == 0) {
			/*if (glm::length(Fspring) >= 0.001 && glm::dot(Fspring, points[i].Vel)<=0) {
				damp = pow(0.6, dtime / 0.1);
			}*/
			points[i].Acc = points[i].Force / points[i].mass;
			points[i].Vel = (points[i].Vel + dtime * (points[i].Acc - gravity)) * damp + dtime * gravity;
			points[i].Pos += dtime * points[i].Vel;
		}
	}
	
	allEnergy();
}

void MyCubes::Breath(double time) {
	//int bar_breath[] = {
//0, 2, 8, 10,		// x directin
//1, 3, 9, 11,		// y direction
//4, 5, 7, 6			// z direction
//};
	//int edge;
	if (fail == true) return;

	double l_goal;
	int point_a;
	int point_b;
	double l;
	double dist;
	double v;
	int spring_size = springs.size();
	MySpring tmpSpring;
	glm::dvec3 pos_a;
	glm::dvec3 pos_b;
	glm::dvec3 move_direction;

	for (int i = 0; i < spring_size;i++) {

		tmpSpring = springs[i];
		point_a = springs[i].group.a;
		point_b = springs[i].group.b;
		pos_a = points[point_a].Pos;
		pos_b = points[point_b].Pos;
		l_goal = tmpSpring.a + tmpSpring.b * sin(time * omega + tmpSpring.c);
		springs[i].L0 = l_goal;

	}
	
}

glm::dvec3 MyCubes::mySpringForce(int pointidx) {
	//std::cout << "in the function mySpringForce\n";
	glm::dvec3 Force = { 0.0,0.0,0.0 };
	glm::dvec3 direction;
	int end;
	int spring;
	double f;
	int spring_size = points[pointidx].P_bar.size();
	//std::cout << "%%%%%%%%%%%%%%%%%this point is " << cubeindex << "\n";
//#pragma omp parallel for
	for (int i = 0; i < spring_size; i++) {
		//end = points[pointidx].connect[i];
		spring = points[pointidx].P_bar[i];
		if (springs[spring].group.a == pointidx) {
			end = springs[spring].group.b;
		} else {
			end = springs[spring].group.a;
		}
		direction = glm::normalize(points[end].Pos - points[pointidx].Pos);
		f = springs[spring].F;
		Force += direction * f;
	}
	return Force;
}

double MyCubes::pathLength() {
	//std::cout << "update length\n";
	getCenter();
	endPos = bodyCenter;
	double l;
	double dx = startPos[0] - endPos[0];
	double dy = startPos[1] - endPos[1];
	l = sqrt((pow(dx, 2) + pow(dy, 2)));
	distance = l;
	// std::cout << startPos[0] << " " << startPos[1] << " " << startPos[2] << " \n";
	// std::cout << endPos[0] << " " << endPos[1] << " " << endPos[2] << " \n";
	// std::cout << "distance: " << distance << "\n";
	return l;
}

void MyCubes::getCenter() {
	double c_x = 0;
	double c_y = 0;
	double c_z = 0;
	int n = points.size();
	for (int i = 0; i < n; i++) {
		c_x += points[i].Pos[0];
		c_y += points[i].Pos[1];
		c_z += points[i].Pos[2];
	}
	bodyCenter = { c_x / n, c_y / n, c_z / n };
}

void MyCubes::restart(GLdouble build[], glm::dvec3 trans) {

	glm::dvec3 move = trans - startPos;
	for (int i = 0; i < points.size(); i++) {
		
		points[i].Pos = points[i].startPos + move;
		points[i].Vel = glm::dvec3{ 0,0,0 };
		points[i].Acc = glm::dvec3{ 0,0,0 };
		points[i].Force = points[i].mass * gravity;
	}
	fail = false;
	EP = 0;
	EK = 0;
	E = 0;
	distance = 0;
}

void MyCubes::kineticE() {
	double Energy = 0;    // E = 1/2 * m * v^2
	for (int i = 0; i < points.size(); i++) {
		Energy += pow(points[i].Vel[0], 2);
		Energy += pow(points[i].Vel[1], 2);
		Energy += pow(points[i].Vel[2], 2);
	}
	Energy = Energy / 2 * points[0].mass;
	EK = Energy;
}

void MyCubes::potentialE() {
	double Energy = 0;    // E = mgh + 1/2*k*dx^2
	double h = 0;
	for (int i = 0; i < points.size(); i++) {
		h += points[i].Pos[2];
	}
	Energy = points[0].mass * 9.8 * h;

	double dx2 = 0;
	double Energy2 = 0;
	for (int i = 0; i < springs.size(); i++) {
		dx2 += pow((springs[i].L - springs[i].L0), 2)*springs[i].K;
		
		Energy2 += dx2 * springs[i].K / 2;
	}
	
	EP = Energy + Energy2;

}

void MyCubes::allEnergy() {
	kineticE();
	potentialE();

	E = EK + EP;
}