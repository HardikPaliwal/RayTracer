#pragma once

class Ray {
    public:
    glm::vec3 origin;
    glm::vec3 direction;
    ~Ray();
    Ray(glm::vec3 origin, glm::vec3 direction);
};
