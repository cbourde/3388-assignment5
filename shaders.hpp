char const *vertexShader = "\
#version 330 core\n\
// Input vertex data, different for all executions of this shader.\n\
layout(location = 0) in vec3 vertexPosition;\n\
layout(location = 1) in vec3 normal;\n\
// Output data ; will be interpolated for each fragment.\n\
out vec2 vertex_normal;\n\
// Values that stay constant for the whole mesh.\n\
uniform mat4 MVP;\n\
void main(){ \n\
	// Output position of the vertex, in clip space : MVP * position\n\
	gl_Position =  MVP * vec4(vertexPosition,1);\n\
	// The color will be interpolated to produce the color of each fragment\n\
	vertex_normal = normal;\n\
}\n\0";

char const *fragmentShader = "\
#version 330 core\n\
in vec3 vertex_normal; \n\
out vec4 color; \n\
uniform sampler2D tex;\n\
void main() {\n\
	color = vec4(vertex_normal, 1.0f);\n\
}\n\0";