#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout (location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;

out vec3 fragNor;
out vec3 L;
out vec3 eye;

void main()
{
	vertTex.xy; // ignore texture coordinates for meshes where I use materials
	gl_Position = P * V * M * vertPos;
	fragNor = (M * vec4(vertNor, 0.0)).xyz;
	eye = (-1*V*M*vertPos).xyz;
	L = lightPos - (M * vertPos).xyz;
}
