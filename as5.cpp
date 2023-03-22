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
#include <shaders.hpp>

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

// Default parameters
const float DEFAULT_ISO = 0.1f;
const float DEFAULT_MIN = -3.0f;
const float DEFAULT_MAX = 3.0f;
const float DEFAULT_STEP = 0.01f;

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

		// Generates the entire mesh (non-incremental)
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

		// Compares a value to the iso value based on the selected comparator
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

		// Adds vertices to the vertices list based on the given list of indices and current coordinates
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

		// Returns a pointer to the vertices list for populating buffers
		std::vector<float> *getVertexListPtr(){
			return &vertices;
		}
};

// Generates a list of normals for a list of vertices
std::vector<float> generateNormals(std::vector<float> *vertices){
	std::vector<float> normals;
	int size = vertices->size();
	if (size < 9) return normals;
	for (int i = 8; i < size; i += 9){
		// Putting the iterator at the end of a vertex means it won't break if, for some reason,
		// the last vertex in the list is incomplete.
		glm::vec3 v1(vertices[i - 8], vertices[i - 7], vertices[i - 6]);
		glm::vec3 v2(vertices[i - 5], vertices[i - 4], vertices[i - 3]);
		glm::vec3 v3(vertices[i - 2], vertices[i - 1], vertices[i]);

		glm::vec3 v12(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);	// Vector from v1 to v2
		glm::vec3 v13(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z);	// Vector from v1 to v3

		// CCW winding order means normal has correct direction with v12 X v13
		glm::vec3 normal = glm::normalize(glm::cross(v12, v13));

		// Add the normal to the list 3 times
		for (int j = 0; j < 3; j++){
			normals.emplace_back(normal.x);
			normals.emplace_back(normal.y);
			normals.emplace_back(normal.z);
		}
	}
	return normals;
}

int main(){
	std::vector<float> normals;

	// Todo: Command line args for step size, min, max, iso

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

	MarchingCubes cubes(f, DEFAULT_ISO, DEFAULT_MIN, DEFAULT_MAX, DEFAULT_STEP);

	// Set up the VAO and buffers
	GLuint vao, vertexVBO, normalVBO, programID;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// Vertex VBO
	glGenBuffers(1, &vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, cubes.getVertexListPtr(), GL_STATIC_DRAW);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(GL_FLOAT) * 3,
		(void*)0
	);
	// Normal VBO
	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, &normals, GL_STATIC_DRAW);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(GL_FLOAT) * 3,
		(void*)0
	);
	glBindVertexArray(0);

	// Shaders (Todo: Write the lighting shader)
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(vertexShaderID, 1, &vertexShader, NULL);
	glCompileShader(vertexShaderID);
	glShaderSource(fragmentShaderID, 1, &fragmentShader, NULL);
	glCompileShader(fragmentShaderID);
	programID = glCreateProgram();
	glAttachShader(programID, vertexShaderID);
	glAttachShader(programID, fragmentShaderID);
	glLinkProgram(programID);
	glDetachShader(programID, vertexShaderID);
	glDetachShader(programID, fragmentShaderID);
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);



	while (!glfwWindowShouldClose(window)){
		// Todo: Process input for camera movement
		// eyePos = ...
		// view = lookAt(eyePos, zero, up);
		// mvp = projection * view * model;

		// Generate more of the mesh if it's not done yet (also update vertex and normal buffers)
		if (!cubes.finished){
			cubes.generate();
		}
		else{
			// Todo: Save the mesh to a file
		}

		// Draw the mesh
		


		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	return 0;
}