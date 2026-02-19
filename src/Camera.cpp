#include "Camera.h"
#include <glad/glad.h>
#include "Editor.h"
#include "InputManager.h"

Camera::Camera(int width, int height, glm::vec3 position) : m_cameraEnabled(true)
{
	Camera::width = width;
	Camera::height = height;
	Position = position;
}

void Camera::Matrix(float FOVdeg, float nearPlane, float farPlane, Shader& shader, const char* uniform)
{
	
	glm::mat4 view = glm::mat4(1.0f);
	glm::mat4 projection = glm::mat4(1.0f);

	
	view = glm::lookAt(Position, Position + Orientation, Up);
	
	projection = glm::perspective(glm::radians(FOVdeg), (float)width / height, nearPlane, farPlane);

	
	glUniformMatrix4fv(glGetUniformLocation(shader.ID, uniform), 1, GL_FALSE, glm::value_ptr(projection * view));
}

void Camera::Inputs(GLFWwindow* window)
{


	if (Editor::isEditMode) {
		
		if (InputManager::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_2)) {
			if (!m_cameraEnabled) {
				InputManager::SetCursorMode(GLFW_CURSOR_DISABLED);
				m_cameraEnabled = true;
				firstClick = true;
			}
		}
		else {
			if (m_cameraEnabled) {
				InputManager::SetCursorMode(GLFW_CURSOR_NORMAL);
				m_cameraEnabled = false;
			}
		}
	}
	else {
		if (!m_cameraEnabled) {
			InputManager::SetCursorMode(GLFW_CURSOR_DISABLED);
			m_cameraEnabled = true;
			firstClick = true;
		}
	}

	if (!m_cameraEnabled) {
		return;
	}

	
	if (InputManager::IsKeyPressed(GLFW_KEY_W))
	{
		Position += speed * Orientation;
	}
	if (InputManager::IsKeyPressed(GLFW_KEY_A))
	{
		Position += speed * -glm::normalize(glm::cross(Orientation, Up));
	}
	if (InputManager::IsKeyPressed(GLFW_KEY_S))
	{
		Position += speed * -Orientation;
	}
	if (InputManager::IsKeyPressed(GLFW_KEY_D))
	{
		Position += speed * glm::normalize(glm::cross(Orientation, Up));
	}
	if (InputManager::IsKeyPressed(GLFW_KEY_SPACE))
	{
		Position += speed * Up;
	}
	if (InputManager::IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		Position += speed * -Up;
	}
	if (InputManager::IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
	{
		speed = 0.4f;
	}
	else 
	{
		speed = 0.1f;
	}


	
	if (m_cameraEnabled)
	{
		
		if (firstClick)
		{
			InputManager::SetCursorMode(GLFW_CURSOR_DISABLED); 
            
            
            
            
			glfwSetCursorPos(window, (width / 2), (height / 2));
			firstClick = false;
		}

		
		double mouseX = InputManager::GetMouseX();
		double mouseY = InputManager::GetMouseY();
		

		
		
		float rotX = sensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = sensitivity * (float)(mouseX - (width / 2)) / width;

		
		yaw   += rotY;
		pitch += -rotX;

		
		if (pitch > 89.0f)  pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;

		
		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		Orientation = glm::normalize(direction);

		
		glfwSetCursorPos(window, (width / 2), (height / 2));
	}
}

glm::mat4 Camera::GetViewMatrix()
{
	return glm::lookAt(Position, Position + Orientation, Up);
}

glm::mat4 Camera::GetProjectionMatrix()
{
	return glm::perspective(glm::radians(FOV), (float)width / height, nearPlane, farPlane);
}

glm::vec3 Camera::GetRay(GLFWwindow* window)
{
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	float x = (2.0f * mouseX) / width - 1.0f;
	float y = 1.0f - (2.0f * mouseY) / height;
	float z = 1.0f;
	glm::vec3 ray_nds = glm::vec3(x, y, z);

	glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

	glm::vec4 ray_eye = inverse(GetProjectionMatrix()) * ray_clip;
	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

	glm::vec3 ray_wor = (inverse(GetViewMatrix()) * ray_eye);
	ray_wor = normalize(ray_wor);

	return ray_wor;
}