#include "Camera.h"

Camera::Camera(float width, float height)
    : width(width), height(height),
    position(0.0f, 0.0f, 3.0f),
    target(0.0f, 0.0f, 0.0f),
    up(0.0f, 1.0f, 0.0f)
{
    projection = glm::ortho(0.0f, width, 0.0f, height, -1.0f, 1.0f);
    UpdateView();
}

const glm::mat4& Camera::GetProjection() const {
    return projection;
}

const glm::mat4& Camera::GetView() const {
    return view;
}

void Camera::SetPosition(const glm::vec2& pos) {
    position = glm::vec3(pos, position.z);
    UpdateView();
}

void Camera::Move(const glm::vec2& delta) {
    position += glm::vec3(delta, 0.0f);
    UpdateView();
}

void Camera::UpdateView()
{
    // Egyszerû 2D-s kamera, amely a position alapján tolja el a világot
    view = glm::translate(glm::mat4(1.0f), glm::vec3(-position.x, -position.y, 0.0f));
}
