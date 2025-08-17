#ifndef MESH_CLASS_H
#define MESH_CLASS_H

#include<string>
#include<vector>

#include"VBO.h"
#include"VAO.h"
#include"EBO.h"
#include"Camera.h"
#include"Texture.h"

class Mesh
{
public:
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;
	// Store VAO in public so it can be used in the Draw function
	VAO vao;
	glm::vec3 minAABB;
	glm::vec3 maxAABB;

	// Initializes the mesh
	Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures);

	// Draws the mesh
	void Draw(Shader& shader, Camera& camera);
	bool Intersect(const glm::vec3& ray_origin, const glm::vec3& ray_direction, float& intersection_distance);
};
#endif