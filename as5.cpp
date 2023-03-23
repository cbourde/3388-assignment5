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

#include "TriTable.hpp"
#include "shaders.hpp"

// Corner definitions
#define BOTTOM_BACK_LEFT	1
#define BOTTOM_BACK_RIGHT	2
#define BOTTOM_FRONT_RIGHT	4
#define BOTTOM_FRONT_LEFT	8
#define TOP_BACK_LEFT		16
#define TOP_BACK_RIGHT		32
#define TOP_FRONT_RIGHT		64
#define TOP_FRONT_LEFT		128

// Function that generates the surface
float f(float x, float y, float z){
	return y - (sin(x) * cos(z));
}

float f2(float x, float y, float z){
	return x;
}

// Default parameters
const float DEFAULT_ISO = 0.05f;
const float DEFAULT_MIN = -3.0f;
const float DEFAULT_MAX = 3.0f;
const float DEFAULT_STEP = 0.02f;
const float ZOOM_SPEED = 2.0f;

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

// Different comparisons to use when testing if a point is inside.
// IDK if this is actually useful
enum CompareOperation{
	Less,
	LessEqual,
	Greater,
	GreaterEqual
};

// Handles marching cubes mesh generation
class MarchingCubes{
		CubesMode generationMode = Full;
		CompareOperation comparator = Less;
		std::function<float(float, float, float)> generationFunction;
		float isoValue = 0;
		float minCoord = 0;
		float maxCoord = 1;
		float stepSize = 0.1;
		float currentIteration = 0;
		std::vector<float> vertices;

		// Generates the entire mesh (non-incremental)
		void generateFull(){
			float bbl, bbr, bfr, bfl, tbl, tbr, tfr, tfl;
			int index;
			int* verts;

			for (float x = minCoord; x < maxCoord; x += stepSize){
				for (float y = minCoord; y < maxCoord; y += stepSize){
					for (float z = minCoord; z < maxCoord; z += stepSize){
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

		// Generates one slice of the mesh
		void generateIterative(){
			float bbl, bbr, bfr, bfl, tbl, tbr, tfr, tfl;
			int index;
			int* verts;
			
			for (float a = minCoord; a < maxCoord; a += stepSize){
				for (float b = minCoord; b < maxCoord; b += stepSize){
					switch (generationMode){
						case Incremental_X:
							// A is Y, B is Z
							// Test the 8 points
							bbl = generationFunction(currentIteration, a, b);
							bbr = generationFunction(currentIteration + stepSize, a, b);
							bfr = generationFunction(currentIteration + stepSize, a, b + stepSize);
							bfl = generationFunction(currentIteration, a, b + stepSize);
							tbl = generationFunction(currentIteration, a + stepSize, b);
							tbr = generationFunction(currentIteration + stepSize, a + stepSize, b);
							tfr = generationFunction(currentIteration + stepSize, a + stepSize, b + stepSize);
							tfl = generationFunction(currentIteration, a + stepSize, b + stepSize);

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
							add_triangles(verts, currentIteration, a, b);
							break;
						case Incremental_Y:
							// A is X, B is Z
							// Test the 8 points
							bbl = generationFunction(a, currentIteration, b);
							bbr = generationFunction(a + stepSize, currentIteration, b);
							bfr = generationFunction(a + stepSize, currentIteration, b + stepSize);
							bfl = generationFunction(a, currentIteration, b + stepSize);
							tbl = generationFunction(a, currentIteration + stepSize, b);
							tbr = generationFunction(a + stepSize, currentIteration + stepSize, b);
							tfr = generationFunction(a + stepSize, currentIteration + stepSize, b + stepSize);
							tfl = generationFunction(a, currentIteration + stepSize, b + stepSize);

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
							add_triangles(verts, a, currentIteration, b);
							break;
						case Incremental_Z:
							// A is X, B is Y
							// Test the 8 points
							bbl = generationFunction(a, b, currentIteration);
							bbr = generationFunction(a + stepSize, b, currentIteration);
							bfr = generationFunction(a + stepSize, b, currentIteration + stepSize);
							bfl = generationFunction(a, b, currentIteration + stepSize);
							tbl = generationFunction(a, b + stepSize, currentIteration);
							tbr = generationFunction(a + stepSize, b + stepSize, currentIteration);
							tfr = generationFunction(a + stepSize, b + stepSize, currentIteration + stepSize);
							tfl = generationFunction(a, b + stepSize, currentIteration + stepSize);

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
							add_triangles(verts, a, b, currentIteration);
							break;
							
					}
				}
			}

			currentIteration += stepSize;
			if (currentIteration > maxCoord){
				finished = true;
				std::cout << "Done generating!" << std::endl;
			} 
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
		MarchingCubes(std::function<float(float, float, float)> f, float isoval, float min, float max, float step, CubesMode mode = Full, CompareOperation comp = Less){
			generationFunction = f;
			isoValue = isoval;
			minCoord = min;
			maxCoord = max;
			stepSize = step;
			generationMode = mode;
			comparator = comp;
			currentIteration = minCoord;
		}

		void generate(){
			switch (generationMode){
				case Full:
					generateFull();
					break;
				case Incremental_X:
					generateIterative();
					break;
				case Incremental_Y:
				case Incremental_Z:
					generateIterative();
					break;
			}
		}

		// Returns the vertices list for populating buffers
		std::vector<float> getVertices(){
			return vertices;
		}
};

// Generates a list of normals for a list of vertices
std::vector<float> generateNormals(std::vector<float> vertices){
	std::vector<float> normals;
	int size = vertices.size();
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

class Axes {

	glm::vec3 origin;
	glm::vec3 extents;

	glm::vec3 xcol = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 ycol = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 zcol = glm::vec3(0.0f, 0.0f, 1.0f);

public:

	Axes(glm::vec3 orig, glm::vec3 ex) : origin(orig), extents(ex) {}

	void draw() {

		glMatrixMode( GL_MODELVIEW );
		glPushMatrix();


		glLineWidth(2.0f);
		glBegin(GL_LINES);
		glColor3f(xcol.x, xcol.y, xcol.z);
		glVertex3f(origin.x, origin.y, origin.z);
		glVertex3f(origin.x + extents.x, origin.y, origin.z);

		glVertex3f(origin.x + extents.x, origin.y, origin.z);
		glVertex3f(origin.x + extents.x, origin.y, origin.z+0.1);
		glVertex3f(origin.x + extents.x, origin.y, origin.z);
		glVertex3f(origin.x + extents.x, origin.y, origin.z-0.1);

		glColor3f(ycol.x, ycol.y, ycol.z);
		glVertex3f(origin.x, origin.y, origin.z);
		glVertex3f(origin.x, origin.y + extents.y, origin.z);

		glVertex3f(origin.x, origin.y + extents.y, origin.z);
		glVertex3f(origin.x, origin.y + extents.y, origin.z+0.1);
		glVertex3f(origin.x, origin.y + extents.y, origin.z);
		glVertex3f(origin.x, origin.y + extents.y, origin.z-0.1);
		
		glColor3f(zcol.x, zcol.y, zcol.z);
		glVertex3f(origin.x, origin.y, origin.z);
		glVertex3f(origin.x, origin.y, origin.z + extents.z);
		
		glVertex3f(origin.x, origin.y, origin.z + extents.z);
		glVertex3f(origin.x+0.1, origin.y, origin.z + extents.z);

		glVertex3f(origin.x, origin.y, origin.z + extents.z);
		glVertex3f(origin.x-0.1, origin.y, origin.z + extents.z);
		glEnd();


		//glPopMatrix();
	}

};

void draw_box(float min, float max){
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINE_STRIP);
		glVertex3f(min, min, min);
		glVertex3f(max, min, min);
		glVertex3f(max, max, min);
		glVertex3f(min, max, min);
		glVertex3f(min, min, min);
		glVertex3f(min, min, max);
		glVertex3f(max, min, max);
		glVertex3f(max, max, max);
		glVertex3f(min, max, max);
		glVertex3f(min, min, max);
	glEnd();
	glBegin(GL_LINES);
		glVertex3f(min, max, min);
		glVertex3f(min, max, max);
		glVertex3f(max, max, min);
		glVertex3f(max, max, max);
		glVertex3f(max, min, min);
		glVertex3f(max, min, max);
	glEnd();
}

int main(){
	std::vector<float> normals;

	// Todo: Command line args for step size, min, max, iso
	float step = DEFAULT_STEP;
	float min = DEFAULT_MIN;
	float max = DEFAULT_MAX;
	float isoval = DEFAULT_ISO;

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
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// Set up initial MVP matrix
	glm::mat4 mvp;
	glm::vec3 eyePos(5, 5, 5);
	glm::vec3 zero(0, 0, 0);
	glm::vec3 up(0, 1, 0);
	glm::mat4 view = glm::lookAt(eyePos, zero, up);
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.001f, 1000.0f);
	glm::mat4 model = glm::mat4(1.0f);
	mvp = projection * view * model;

	MarchingCubes cubes(f, isoval, min, max, step, Incremental_Z);
	Axes ax(glm::vec3(min), glm::vec3(max - min));


	// Set up the VAO and buffers
	GLuint vao, vertexVBO, normalVBO, programID;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	// Vertex VBO
	glGenBuffers(1, &vertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, &cubes.getVertices()[0], GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0
	);
	// Normal VBO
	glGenBuffers(1, &normalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, &normals, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
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

	glClear(GL_COLOR_BUFFER_BIT);
	glfwSwapBuffers(window);

	// Variables for input
	double mouseX = 0;
	double mouseY = 0;
	double prevMouseX = 0;
	double prevMouseY = 0;

	// Camera position spherical coords
	float r = 5.0f;
	float theta = 45.0f;
	float phi = 45.0f;

	double prevTime = glfwGetTime();

	while (!glfwWindowShouldClose(window)){
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Get delta time
		double currentTime = glfwGetTime();
		float deltaTime = currentTime - prevTime;
		prevTime = currentTime;


		// Mouse dragging
		glfwGetCursorPos(window, &mouseX, &mouseY);
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
			// Move the camera
			phi = glm::clamp(phi - (mouseY - prevMouseY), 0.0001, 179.9999);	// If phi is exactly 0 or 180 weird things happen
			theta += (mouseX - prevMouseX);
		}
		// Zoom with arrow keys
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
			// Zoom in (clamp to almost zero)
			r = glm::clamp(r - ZOOM_SPEED * deltaTime, 0.1f, 1000.0f);
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
			// Zoom out (no limit)
			r += ZOOM_SPEED * deltaTime;
		}

		eyePos = {r * cos(glm::radians(theta)) * sin(glm::radians(phi)), r * cos(glm::radians(phi)), r * sin(glm::radians(theta)) * sin(glm::radians(phi))};
		view = glm::lookAt(eyePos, zero, up);
		mvp = projection * view * model;

		// Generate more of the mesh if it's not done yet (also update vertex and normal buffers)
		if (!cubes.finished){
			cubes.generate();
			std::vector<float> vertices = cubes.getVertices();
			normals = generateNormals(vertices);

			// Update buffers
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
			glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GL_FLOAT), &normals[0], GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GL_FLOAT), &vertices[0], GL_DYNAMIC_DRAW);
			glBindVertexArray(0);
		}
		else{
			// Todo: Save the mesh to a file
		}

		// Draw the axes and box
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixf(glm::value_ptr(projection));
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadMatrixf(glm::value_ptr(view));
		ax.draw();
		draw_box(min, max);

		// Draw the mesh
		glUseProgram(programID);		
		GLuint matrixID = glGetUniformLocation(programID, "MVP");
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvp[0][0]);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, normals.size());
		glBindVertexArray(0);
		glUseProgram(0);

		prevMouseX = mouseX;
		prevMouseY = mouseY;

		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	return 0;
}