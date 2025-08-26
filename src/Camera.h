#ifndef CAMERA_CLASS_H
#define CAMERA_CLASS_H

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<glm/gtx/rotate_vector.hpp>
#include<glm/gtx/vector_angle.hpp>

#include "Shader.h"

class Camera
{
public:
	
	glm::vec3 Position;
	glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

	
	bool firstClick = true;

	
	int width;
	int height;

	
	bool m_cameraEnabled;

	
	float speed = 0.5f;
	float sensitivity = 100.0f;
	float farPlane = 100.0f;
	float FOV = 45.0f;

	
	Camera(int width, int height, glm::vec3 position);

	
	void Matrix(float FOVdeg, float nearPlane, float farPlane, Shader& shader, const char* uniform);
	
	void Inputs(GLFWwindow* window);
	glm::mat4 GetViewMatrix();
	glm::mat4 GetProjectionMatrix();
	glm::vec3 GetRay(GLFWwindow* window);
};
#endif