#include "Camera.h"

Camera::Camera(float width, float height)
    : width(width), height(height),
    position(0.0f, 0.0f, 0.0f)
{
    // Ortografikus kamera (2D)
    projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);
    UpdateView();
}

void Camera::SetPosition(const glm::vec2& pos)
{
    position.x = pos.x;
    position.y = pos.y;
    UpdateView();
}

void Camera::Move(const glm::vec2& delta)
{
    position.x += delta.x;
    position.y += delta.y;
    UpdateView();
}

void Camera::UpdateView()
{
    // A kamera pozíciójától függõen mozgatjuk a világot
    view = glm::translate(glm::mat4(1.0f), glm::vec3(-position.x, -position.y, 0.0f));
}

const glm::mat4& Camera::GetView() const { return view; }
const glm::mat4& Camera::GetProjection() const { return projection; }