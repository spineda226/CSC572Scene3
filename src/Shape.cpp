#include "Shape.h"
#include <iostream>
#include <assert.h>

#include "GLSL.h"
#include "Program.h"

using namespace std;

Shape::Shape() :
	eleBufID(0),
	posBufID(0),
	norBufID(0),
	texBufID(0), 
	vaoID(0)
{
	min = glm::vec3(0);
	max = glm::vec3(0);
}

Shape::~Shape()
{
}

/* copy the data from the shape to this object */
void Shape::createShape(tinyobj::shape_t & shape)
{
	posBuf = shape.mesh.positions;
	norBuf = shape.mesh.normals;
	texBuf = shape.mesh.texcoords;
	eleBuf = shape.mesh.indices;
}

void Shape::allocNorms()
{
	for (size_t v = 0; v < posBuf.size(); ++v) // set all normals to 0
	{
		norBuf.push_back(0); 
	}

	/* Calculate and populate norBuf */
	int faces = eleBuf.size()/3;
	int vertices = posBuf.size()/3;
	
	int index;
	glm::vec3 triangle[3];
	glm::vec3 normal;
	for (int f = 0; f < faces; ++f) // For each face, compute normal per face
	{
		for (int i = 0; i < 3; ++i) // Gather vertices for a face
		{
			index = eleBuf[3*f+i];
			triangle[i] = glm::vec3(posBuf[3*index], posBuf[3*index+1], posBuf[3*index+2]);
		}
		normal = normalize(cross((triangle[1]-triangle[0]), (triangle[2]-triangle[0])));
		for (int i = 0; i < 3; ++i) // add normal to 3 adjacent vertices
		{
			index = eleBuf[3*f+i];
			norBuf[3*index] += normal.x;
			norBuf[3*index+1] += normal.y;
			norBuf[3*index+2] += normal.z;
		}
	}
	for (int v = 0; v < vertices; ++v) // normalize each vertex normal
	{
		normal = normalize(glm::vec3(norBuf[3*v], norBuf[3*v+1], norBuf[3*v+2]));
		norBuf[3*v] = normal.x;
		norBuf[3*v+1] = normal.y;
		norBuf[3*v+2] = normal.z;
	}

	glGenBuffers(1, &norBufID);
	glBindBuffer(GL_ARRAY_BUFFER, norBufID);
	glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
}

void Shape::measure() 
{
	float minX, minY, minZ;
	float maxX, maxY, maxZ;

	minX = minY = minZ = 1.1754E+38F;
	maxX = maxY = maxZ = -1.1754E+38F;

   //Go through all vertices to determine min and max of each dimension
   for (size_t v = 0; v < posBuf.size() / 3; v++) {
		if(posBuf[3*v+0] < minX) minX = posBuf[3*v+0];
		if(posBuf[3*v+0] > maxX) maxX = posBuf[3*v+0];

		if(posBuf[3*v+1] < minY) minY = posBuf[3*v+1];
		if(posBuf[3*v+1] > maxY) maxY = posBuf[3*v+1];

		if(posBuf[3*v+2] < minZ) minZ = posBuf[3*v+2];
		if(posBuf[3*v+2] > maxZ) maxZ = posBuf[3*v+2];
	}

	min.x = minX;
	min.y = minY;
	min.z = minZ;
    max.x = maxX;
    max.y = maxY;
    max.z = maxZ;
}

void Shape::init()
{
	// Initialize the vertex array object
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	// Send the position array to the GPU
	glGenBuffers(1, &posBufID);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glBufferData(GL_ARRAY_BUFFER, posBuf.size()*sizeof(float), &posBuf[0], GL_STATIC_DRAW);
	
	// Send the normal array to the GPU
	if(norBuf.empty())
	{
		allocNorms();		
	}
	else
	{
		glGenBuffers(1, &norBufID);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glBufferData(GL_ARRAY_BUFFER, norBuf.size()*sizeof(float), &norBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the texture array to the GPU
	if(texBuf.empty()) {
		texBufID = 0;
	} else {
		glGenBuffers(1, &texBufID);
		glBindBuffer(GL_ARRAY_BUFFER, texBufID);
		glBufferData(GL_ARRAY_BUFFER, texBuf.size()*sizeof(float), &texBuf[0], GL_STATIC_DRAW);
	}
	
	// Send the element array to the GPU
	glGenBuffers(1, &eleBufID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size()*sizeof(unsigned int), &eleBuf[0], GL_STATIC_DRAW);
	
	// Unbind the arrays
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	
	assert(glGetError() == GL_NO_ERROR);
}

void Shape::draw(const shared_ptr<Program> prog) const
{
	int h_pos, h_nor, h_tex;
	h_pos = h_nor = h_tex = -1;

    glBindVertexArray(vaoID);
	// Bind position buffer
	h_pos = prog->getAttribute("vertPos");
	GLSL::enableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, posBufID);
	glVertexAttribPointer(h_pos, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	
	// Bind normal buffer
	h_nor = prog->getAttribute("vertNor");
	if(h_nor != -1 && norBufID != 0) {
		GLSL::enableVertexAttribArray(h_nor);
		glBindBuffer(GL_ARRAY_BUFFER, norBufID);
		glVertexAttribPointer(h_nor, 3, GL_FLOAT, GL_FALSE, 0, (const void *)0);
	}

	if (texBufID != 0) {	
		// Bind texcoords buffer
		h_tex = prog->getAttribute("vertTex");
		if(h_tex != -1 && texBufID != 0) {
			GLSL::enableVertexAttribArray(h_tex);
			glBindBuffer(GL_ARRAY_BUFFER, texBufID);
			glVertexAttribPointer(h_tex, 2, GL_FLOAT, GL_FALSE, 0, (const void *)0);
		}
	}
	
	// Bind element buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufID);
	
	// Draw
	glDrawElements(GL_TRIANGLES, (int)eleBuf.size(), GL_UNSIGNED_INT, (const void *)0);
	
	// Disable and unbind
	if(h_tex != -1) {
		GLSL::disableVertexAttribArray(h_tex);
	}
	if(h_nor != -1) {
		GLSL::disableVertexAttribArray(h_nor);
	}
	GLSL::disableVertexAttribArray(h_pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

// Load geometry of a multiple shape mesh
void loadMultipleShapeMesh(shared_ptr<vector<shared_ptr<Shape>>> shapes,
			  glm::vec3 *gMin, glm::vec3 *gMax, const string &meshName)
{
	shared_ptr<Shape> shape;
	vector<tinyobj::shape_t> TOshapes;
	vector<tinyobj::material_t> objMaterials;
	string errStr;
	bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, meshName.c_str());
	if (! rc)
	{
		cerr << errStr << endl;
	}
	else 
	{
	    int totalShapes = TOshapes.size();
		for (int i = 0; i < totalShapes; ++i)
		{
			shape = make_shared<Shape>();
			shape->createShape(TOshapes[i]);
			shape->measure();
			shape->init();
			shapes->push_back(shape);
			updateBounds(i, shape, gMin, gMax);
		}
	}	
}

// Updates global min and max coordinates for each shape
void updateBounds(int i, const shared_ptr<Shape> shape,
				  glm::vec3 *gMin, glm::vec3 *gMax)
{
	if (i == 0)
	{
		gMin->x = shape->min.x;
		gMin->y = shape->min.y;
		gMin->z = shape->min.z;
		gMax->x = shape->max.x;
		gMax->y = shape->max.y;
		gMax->z = shape->max.z;
	}
	if (shape->min.x < gMin->x)
		gMin->x = shape->min.x;
	if (shape->min.y < gMin->y)
		gMin->y = shape->min.y;
	if (shape->min.z < gMin->z)
		gMin->z = shape->min.z;
	if (shape->max.x > gMax->x)
		gMax->x = shape->max.x;
	if (shape->max.y > gMax->y)
		gMax->y = shape->max.y;
	if (shape->max.z > gMax->z)
		gMax->z = shape->max.z;
}