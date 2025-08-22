#include "Mesh.h"
#include <limits>

Mesh::Mesh(std::vector <Vertex>& vertices, std::vector <GLuint>& indices, std::vector <Texture>& textures)
{
	Mesh::vertices = vertices;
	Mesh::indices = indices;
	Mesh::textures = textures;

	minAABB = glm::vec3(std::numeric_limits<float>::max());
	maxAABB = glm::vec3(std::numeric_limits<float>::lowest());

	for (const auto& vertex : vertices) {
		minAABB.x = std::min(minAABB.x, vertex.position.x);
		minAABB.y = std::min(minAABB.y, vertex.position.y);
		minAABB.z = std::min(minAABB.z, vertex.position.z);

		maxAABB.x = std::max(maxAABB.x, vertex.position.x);
		maxAABB.y = std::max(maxAABB.y, vertex.position.y);
		maxAABB.z = std::max(maxAABB.z, vertex.position.z);
	}

	vao.Bind();
	VBO VBO(vertices);
	EBO EBO(indices);
	vao.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);
	vao.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));
	vao.LinkAttrib(VBO, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	vao.LinkAttrib(VBO, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));
	vao.Unbind();
	VBO.Unbind();
	EBO.Unbind();
}


void Mesh::Draw(Shader& shader, Camera& camera, glm::vec3 position, glm::quat rotation, glm::vec3 scale)
{
	shader.use();
	vao.Bind();
	unsigned int numDiffuse = 0;
	unsigned int numSpecular = 0;

	for (unsigned int i = 0; i < textures.size(); i++)
	{
		std::string num;
		std::string type = textures[i].type;
		if (type == "diffuse")
		{
			num = std::to_string(numDiffuse++);
		}
		else if (type == "specular")
		{
			num = std::to_string(numSpecular++);
		}
		textures[i].texUnit(shader, (type + num).c_str(), i);
		textures[i].Bind();
	}
	// Take care of the camera Matrix
	glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	camera.Matrix(45.0f, 0.1f, 100.0f, shader, "camMatrix");

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model *= glm::mat4_cast(rotation);
	model = glm::scale(model, scale);

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));

	// Draw the actual mesh
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	vao.Unbind();
}

bool Mesh::Intersect(const glm::vec3& ray_origin, const glm::vec3& ray_direction, float& intersection_distance)
{
	float tmin = 0.0;
	float tmax = std::numeric_limits<float>::max();

	for (int i = 0; i < 3; ++i) {
		if (abs(ray_direction[i]) < 1e-6) {
			if (ray_origin[i] < minAABB[i] || ray_origin[i] > maxAABB[i]) {
				return false;
			}
		}
		else {
			float ood = 1.0f / ray_direction[i];
			float t1 = (minAABB[i] - ray_origin[i]) * ood;
			float t2 = (maxAABB[i] - ray_origin[i]) * ood;

			if (t1 > t2) {
				std::swap(t1, t2);
			}

			tmin = std::max(tmin, t1);
			tmax = std::min(tmax, t2);

			if (tmin > tmax) {
				return false;
			}
		}
	}

	intersection_distance = tmin;
	return true;
}