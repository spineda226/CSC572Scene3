#version 330 core 
in vec3 fragNor;
in vec3 eye;
in vec3 L;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;
uniform vec3 sunCol;
uniform vec3 lightCol;

out vec4 color;

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(L);

	float d = max(0, dot(normal, light));

	vec3 H = (normalize(eye) + light)/2.0;
	float s = pow(max(0, dot(normal, normalize(H))), shine);

	vec3 Ncolor = MatDif*d*lightCol + MatSpec*s*lightCol + MatAmb*lightCol;
	color = vec4(Ncolor, 1.0);
}
