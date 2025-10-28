#pragma once
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(float width, float height);

    void SetPosition(const glm::vec2& pos);
    void Move(const glm::vec2& delta);
    const glm::mat4& GetView() const;
    const glm::mat4& GetProjection() const;

    void UpdateView();

private:
    glm::vec3 position;
    glm::mat4 view;
    glm::mat4 projection;

    float width;
    float height;
};