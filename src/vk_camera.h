#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vulkan/vulkan.h>

enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 7.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 10.0f;

class Camera
{
	public:
		Camera(glm::vec3 position = glm::vec3(0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

		void processKeyboard(Camera_Movement direction, float deltaTime);

		void ProcessMouseMovement(float xoffset, float yoffset, VkBool32 constrainPitch = VK_FALSE);

		void ProcessMouseScroll(float yoffset);

		glm::mat4 GetViewMatrix() const { return glm::lookAt(Position, Position + Front, Up); }
		glm::vec3 GetFront() const { return Front; }
		glm::vec3 GetUp() const { return WorldUp; }
		glm::vec3 GetPosition() const { return Position; }
		float GetZoom() const { return Zoom; }

		float Yaw;
		float Pitch;

	private:
		void updateCameraVectors();

		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;

		float MovementSpeed;
		float MouseSensitivity;
		float Zoom;
};
