//
// Created by niek on 3/27/2025.
//

#include "camera.h"

#include <glm/gtc/quaternion.hpp>

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch):
Front(glm::vec3(0.0f, 0.0f, -1.0f)), Up(), Right(),
MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
    Position = position;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;

    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(const Camera_Movement direction, const float deltaTime) {
    const float velocity = MovementSpeed * deltaTime;
    if (direction == FORWARD)
        Position += Front * velocity;
    if (direction == BACKWARD)
        Position -= Front * velocity;
    if (direction == LEFT)
        Position -= Right * velocity;
    if (direction == RIGHT)
        Position += Right * velocity;
    if (direction == UP)
        Position -= WorldUp * velocity;
    if (direction == DOWN)
        Position += WorldUp * velocity;
}

void Camera::ProcessMouseMovement(float xOffset, float yOffset, const GLboolean constrainPitch) {
    xOffset *= MouseSensitivity;
    yOffset *= MouseSensitivity;

    Yaw += xOffset;
    Pitch += yOffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (constrainPitch) {
        if (Pitch > 89.0) {
            Pitch = 89.0;
        }
        if (Pitch < -89.0) {
            Pitch = -89.0;
        }
    }
    updateCameraVectors();
}

void Camera::ProcessMouseScroll(const float yOffset) {
    Zoom -= yOffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = static_cast<float>(cos(glm::radians(Yaw))) * static_cast<float>(cos(glm::radians(Pitch)));
    front.y = static_cast<float>(sin(glm::radians(Pitch)));
    front.z = static_cast<float>(sin(glm::radians(Yaw))) * static_cast<float>(cos(glm::radians(Pitch)));
    Front = normalize(front);

    // recalculate right and up vector
    Right = normalize(cross(Front, WorldUp));
    Up = normalize(cross(Right, Front));
}
