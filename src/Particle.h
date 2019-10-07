//
// sueda
// November, 2014/ wood 16
//

#pragma once

#ifndef LAB471_PARTICLE_H_INCLUDED
#define LAB471_PARTICLE_H_INCLUDED

#include <vector>
#include <memory>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

class MatrixStack;
class Program;
class Texture;


class Particle
{

public:
	void update(float t, float h, const glm::vec3 &g, const bool *keyToggles);
	glm::vec3 position = glm::vec3(0.f); // position
	glm::vec3 velocity = glm::vec3(0.f); // velocity
	glm::vec3 forces = glm::vec3(0.f);
	float density = 0;
	float pressure = 0;
	//const vec3 &getPosition() const { return x; };
	//const vec3 &getVelocity() const { return v; };
};
/*
// Sort particles by their z values in camera space
class ParticleSorter
{

public:

	bool operator() (const std::shared_ptr<Particle> p0, const std::shared_ptr<Particle> p1) const
	{
		// Particle positions in world space
		const vec3 &x0 = p0->getPosition();
		const vec3 &x1 = p1->getPosition();

		// Particle positions in camera space
		vec4 x0w = C * glm::vec4(x0.x, x0.y, x0.z, 1.0f);
		vec4 x1w = C * glm::vec4(x1.x, x1.y, x1.z, 1.0f);
		return x0w.z < x1w.z;
	}

	// current camera matrix
	mat4 C;

};
*/
#endif // LAB471_PARTICLE_H_INCLUDED
