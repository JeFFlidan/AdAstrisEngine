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

		void process_keyboard(Camera_Movement direction, float deltaTime);

		void process_mouse_movement(float xoffset, float yoffset, VkBool32 constrainPitch = VK_FALSE);

		void process_mouse_scroll(float yoffset);

		glm::mat4 get_view_matrix() const { return glm::lookAt(Position, Position + Front, Up); }
		glm::mat4 get_projection_matrix(bool bReverse = false) const
		{
			if (bReverse)
			{
				glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.0f / 900.0f, 5000.0f, 0.001f);
				projection[1][1] *= -1;
				return projection;
			}
			else
			{
				glm::mat4 projection = glm::perspective(glm::radians(70.f), 1700.0f / 900.0f, 0.001f, 5000.0f);
				projection[1][1] *= -1;
				return projection;
			}
		}
		
		glm::vec3 get_front() const { return Front; }
		glm::vec3 get_up() const { return WorldUp; }
		glm::vec3 get_position() const { return Position; }
		float get_zoom() const { return Zoom; }

		float Yaw;
		float Pitch;

	private:
		void update_camera_vectors();

		glm::vec3 Position;
		glm::vec3 Front;
		glm::vec3 Up;
		glm::vec3 Right;
		glm::vec3 WorldUp;

		float MovementSpeed;
		float MouseSensitivity;
		float Zoom;
};
