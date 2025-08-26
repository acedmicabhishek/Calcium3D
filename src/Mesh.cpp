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
	// Attribute locations expected by shaders: 0-pos, 1-color, 2-tex, 3-normal
	vao.LinkAttrib(VBO, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
	vao.LinkAttrib(VBO, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));
	vao.LinkAttrib(VBO, 2, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, texUV));
	vao.LinkAttrib(VBO, 3, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
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
		textures[i].texUnit(shader, (type + num).c_str(), textures[i].unit);
		textures[i].Bind();
	}
	// Take care of the camera Matrix
	glUniform3f(glGetUniformLocation(shader.ID, "camPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	glUniform3f(glGetUniformLocation(shader.ID, "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
	camera.Matrix(camera.FOV, camera.nearPlane, camera.farPlane, shader, "camMatrix");

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, position);
	model *= glm::mat4_cast(rotation);
	model = glm::scale(model, scale);

	glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
	
	// Calculate texture tiling factor based on scale
	glm::vec3 tilingFactor = glm::vec3(scale.x, scale.y, scale.z);
	glUniform3f(glGetUniformLocation(shader.ID, "tilingFactor"), tilingFactor.x, tilingFactor.y, tilingFactor.z);

	// Draw the actual mesh
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	vao.Unbind();
}

bool Mesh::Intersect(const glm::vec3& ray_origin, const glm::vec3& ray_direction, const glm::mat4& modelMatrix, float& intersection_distance)
{
	// Transform ray to object space
	glm::mat4 invModelMatrix = glm::inverse(modelMatrix);
	glm::vec3 localRayOrigin = glm::vec3(invModelMatrix * glm::vec4(ray_origin, 1.0f));
	glm::vec3 localRayDirection = glm::vec3(invModelMatrix * glm::vec4(ray_direction, 0.0f));
	localRayDirection = glm::normalize(localRayDirection);

	// Perform AABB intersection in object space
	float tmin = 0.0;
	float tmax = std::numeric_limits<float>::max();

	for (int i = 0; i < 3; ++i) {
		if (abs(localRayDirection[i]) < 1e-6) {
			if (localRayOrigin[i] < minAABB[i] || localRayOrigin[i] > maxAABB[i]) {
				return false;
			}
		}
		else {
			float ood = 1.0f / localRayDirection[i];
			float t1 = (minAABB[i] - localRayOrigin[i]) * ood;
			float t2 = (maxAABB[i] - localRayOrigin[i]) * ood;

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

	// Transform intersection distance back to world space
	glm::vec3 intersectionPoint = localRayOrigin + localRayDirection * tmin;
	glm::vec3 worldIntersectionPoint = glm::vec3(modelMatrix * glm::vec4(intersectionPoint, 1.0f));
	intersection_distance = glm::length(worldIntersectionPoint - ray_origin);
	
	return true;
}