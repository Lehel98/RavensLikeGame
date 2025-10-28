#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Camera
{
public:
    Camera(float width, float height);

    const glm::mat4& GetProjection() const;
    const glm::mat4& GetView() const;

    void SetPosition(const glm::vec2& pos);
    void Move(const glm::vec2& delta);

private:
    glm::mat4 projection;
    glm::mat4 view;
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 target;
    float width, height;

    void UpdateView();
};