#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <functional>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <TriTable.hpp>

// Corner definitions
#define BOTTOM_BACK_LEFT	1
#define BOTTOM_BACK_RIGHT	2
#define BOTTOM_FRONT_RIGHT	4
#define BOTTOM_FRONT_LEFT	8
#define TOP_BACK_LEFT		16
#define TOP_BACK_RIGHT		32
#define TOP_FRONT_LEFT		64
#define TOP_FRONT_RIGHT		128

// Function that generates the surface
float f(float x, float y, float z){
	return y - (sin(x) * cos(z));
}

GLFWwindow* window;

// Changes the operation of the marching cubes function.
// Full: Generates the whole mesh in one go (slow)
// Incremental: Generates a single "slice" along one axis each time generate() is called
enum CubesMode{
	Full,
	Incremental_X,
	Incremental_Y,
	Incremental_Z
};

enum CompareOperation{
	Less,
	LessEqual,
	Greater,
	GreaterEqual
};

// Handles marching cubes mesh generation
class MarchingCubes{
		CubesMode generationMode = Full;
		CompareOperation comparator = GreaterEqual;
		std::function<float(float, float, float)> generationFunction;
		float isoValue = 0;
		float minCoord = 0;
		float maxCoord = 1;
		float stepSize = 0.01;
		std::vector<float> vertices;

		void generateFull(){
			float bbl, bbr, bfr, bfl, tbl, tbr, tfr, tfl;
			int index;
			int* verts;

			for (float x = minCoord; x <= maxCoord; x += stepSize){
				for (float y = minCoord; y <= maxCoord; y += stepSize){
					for (float z = minCoord; z <= maxCoord; z += stepSize){
						// Test the 8 points
						bbl = generationFunction(x, y, z);
						bbr = generationFunction(x + stepSize, y, z);
						bfr = generationFunction(x + stepSize, y, z + stepSize);
						bfl = generationFunction(x, y, z + stepSize);
						tbl = generationFunction(x, y + stepSize, z);
						tbr = generationFunction(x + stepSize, y + stepSize, z);
						tfr = generationFunction(x + stepSize, y + stepSize, z + stepSize);
						tfl = generationFunction(x, y + stepSize, z + stepSize);

						index = 0;
						if (test(bbl)) index |= BOTTOM_BACK_LEFT;
						if (test(bbr)) index |= BOTTOM_BACK_RIGHT;
						if (test(bfr)) index |= BOTTOM_FRONT_RIGHT;
						if (test(bfl)) index |= BOTTOM_FRONT_LEFT;
						if (test(tbl)) index |= TOP_BACK_LEFT;
						if (test(tbr)) index |= TOP_BACK_RIGHT;
						if (test(tfr)) index |= TOP_FRONT_RIGHT;
						if (test(tfl)) index |= TOP_FRONT_LEFT;

						verts = marching_cubes_lut[index];
						add_triangles(verts, x, y, z);
					}
				}
			}
			finished = true;
		}

		bool test(float a){
			switch (comparator){
				case Less:
					return a < isoValue;
				case LessEqual:
					return a <= isoValue;
				case Greater:
					return a > isoValue;
				case GreaterEqual:
					return a >= isoValue;
				default:
					return false;
			}
		}

		void add_triangles(int* verts, float x, float y, float z){
			// First triangle
			if (verts[0] >= 0){
				// First vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[0]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[0]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[0]][2]);

				// Second vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[1]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[1]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[1]][2]);

				// Third vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[2]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[2]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[2]][2]);
			}

			// Second triangle
			if (verts[3] >= 0){
				// First vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[3]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[3]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[3]][2]);

				// Second vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[4]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[4]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[4]][2]);

				// Third vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[5]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[5]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[5]][2]);
			}

			// Third triangle
			if (verts[6] >= 0){
				// First vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[6]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[6]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[6]][2]);

				// Second vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[7]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[7]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[7]][2]);

				// Third vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[8]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[8]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[8]][2]);
			}

			// Fourth triangle
			if (verts[9] >= 0){
				// First vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[9]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[9]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[9]][2]);

				// Second vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[10]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[10]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[10]][2]);

				// Third vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[11]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[11]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[11]][2]);
			}

			// Fifth triangle
			if (verts[12] >= 0){
				// First vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[12]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[12]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[12]][2]);

				// Second vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[13]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[13]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[13]][2]);

				// Third vertex
				vertices.emplace_back(x + stepSize * vertTable[verts[14]][0]);
				vertices.emplace_back(y + stepSize * vertTable[verts[14]][1]);
				vertices.emplace_back(z + stepSize * vertTable[verts[14]][2]);
			}
		}
	public:
		bool finished = false;	// Becomes true when the mesh is finished generating (for incremental modes)
		MarchingCubes(std::function<float(float, float, float)> f, float isoval, float min, float max, float step, CubesMode mode = Full, CompareOperation comp = GreaterEqual){
			generationFunction = f;
			isoValue = isoval;
			minCoord = min;
			maxCoord = max;
			stepSize = step;
			generationMode = mode;
			comparator = comp;
		}

		void generate(){
			switch (generationMode){
				case Full:
					generateFull();
					break;
				case Incremental_X:
				case Incremental_Y:
				case Incremental_Z:
					break;
			}
		}

		std::vector<float> *getVertices(){
			return &vertices;
		}
};

int main(){
	// Command line args for step size, min, max, iso

	// Initialize window
	if (!glfwInit()){
		printf("Failed to initialize GLFW\n");
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	window = glfwCreateWindow(1000, 1000, "Assignment 5", NULL, NULL);
	if (window == NULL){
		printf("Failed to open window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK){
		printf("Failed to initialize GLEW\n");
		glfwTerminate();
		return -1;
	}

	glClearColor(0, 0, 0, 1);

	// Set up initial MVP matrix
	glm::mat4 mvp;
	glm::vec3 eyePos(5, 5, 5);
	glm::vec3 zero(0, 0, 0);
	glm::vec3 up(0, 1, 0);
	glm::mat4 view = glm::lookAt(eyePos, zero, up);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.001f, 1000.0f);
	glm::mat4 model = glm::mat4(1.0f);
	mvp = projection * view * model;

	

	while (!glfwWindowShouldClose(window)){
		// Process input for camera movement
		// eyePos = ...
		// view = lookAt(eyePos, zero, up);
		// mvp = projection * view * model;


		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	return 0;
}