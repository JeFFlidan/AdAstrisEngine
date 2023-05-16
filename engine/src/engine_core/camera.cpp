#include <engine_core/camera.h>

Camera::Camera(glm::vec3 position, 
				glm::vec3 up, 
				float yaw, 
				float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), Zoom(ZOOM), MouseSensitivity(SENSITIVITY)
{
	Position = position;
	WorldUp = up;
	Yaw = yaw;
	Pitch = pitch;
	update_camera_vectors();
}

void Camera::process_keyboard(Camera_Movement direction, float deltaTime)
{
	float velocity = deltaTime * MovementSpeed;
	if (direction == FORWARD)
		Position += velocity * Front;
	if (direction == BACKWARD)
		Position -= velocity * Front;
	if (direction == LEFT)
		Position -= Right * velocity;
	if (direction == RIGHT)
		Position += Right * velocity;
}

void Camera::process_mouse_movement(float xoffset, float yoffset, VkBool32 constrainPitch)
{
	Yaw += xoffset * MouseSensitivity;
	Pitch -= yoffset * MouseSensitivity;

	if (constrainPitch)
	{
		if (Pitch > 89.0f)
			Pitch = 89.0f;
		if (Pitch < -89.0f)
			Pitch = -89.0f;
	}

	update_camera_vectors();
}

void Camera::process_mouse_scroll(float yoffset)
{
	Zoom -= yoffset;
	if (Zoom > 45.0f)
		Zoom = 45.0f;
	if (Zoom < 1.0f)
		Zoom = 1.0f;
}

void Camera::update_camera_vectors()
{
	glm::vec3 direction;
	direction.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	direction.y = sin(glm::radians(Pitch));
	direction.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
	Front = glm::normalize(direction);
	Right = glm::normalize(glm::cross(Front, WorldUp));
	Up = glm::normalize(glm::cross(Right, Front));
}
