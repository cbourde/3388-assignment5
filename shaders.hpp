char const *vertexShader = "\
#version 330 core\n\
// Input vertex data, different for all executions of this shader.\n\
layout(location = 0) in vec3 vertexPosition;\n\
layout(location = 1) in vec3 vertexNormal;\n\
// Output data ; will be interpolated for each fragment.\n\
out vec3 normal;\n\
out vec3 eye_direction;\n\
out vec3 light_direction;\n\
// Values that stay constant for the whole mesh.\n\
uniform mat4 MVP;\n\
uniform mat4 V;\n\
uniform vec3 lightDir;\n\
void main(){ \n\
	// Output position of the vertex, in clip space : MVP * position\n\
	gl_Position =  MVP * vec4(vertexPosition,1);\n\
	normal = mat3(V) * vertexNormal;\n\
	eye_direction = vec3(0, 0, 0) - (V * vec4(vertexPosition, 1)).xyz;\n\
	light_direction = mat3(V) * lightDir;\n\
}\n\0";

char const *fragmentShader = "\
#version 330 core\n\
in vec3 normal; \n\
in vec3 light_direction;\n\
in vec3 eye_direction;\n\
out vec4 color; \n\
uniform vec4 modelColor;\n\
void main() {\n\
	vec4 ambient = vec4(0.2, 0.2, 0.2, 1);\n\
	vec4 specular = vec4(1, 1, 1, 1);\n\
	float alpha = 64;\n\
	\n\
	vec3 n = normalize(normal);\n\
	vec3 l = normalize(light_direction);\n\
	vec3 e = normalize(eye_direction);\n\
	vec3 r = reflect(-l, n);\n\
	\n\
	float cosTheta = clamp(dot(n, l), 0, 1);\n\
	float cosAlpha = clamp(dot(e, r), 0, 1);\n\
	color = ambient + (modelColor * cosTheta) + (specular * pow(cosAlpha, alpha));\n\
}\n\0";