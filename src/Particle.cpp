//
// sueda - geometry edits Z. Wood
// 3/16
//

#include <iostream>
#include "Particle.h"
#include "GLSL.h"
#include "MatrixStack.h"
#include "Program.h"
#include "Texture.h"



void Particle::update(float t, float h, const vec3 &g, const bool *keyToggles)
{
	//gravity
	velocity.y = velocity.y - 0.0005;
	// very simple update
	position += h * velocity;
	//color.a = (tEnd - t) / lifespan;
}
