#ifndef RENDERENGINE_THIRDPERSONCAMERA_H
#define RENDERENGINE_THIRDPERSONCAMERA_H

#include "ACamera.h"
#include <glm/ext.hpp>

class ThirdPersonCamera : public ACamera {
public:
    void moveCenter(float offsetX, float offsetY, float offsetZ);
    void move(float offsetR, float offsetPhi, float offsetTheta);

    void setCenter(float x, float y, float z);
    void setCenter(glm::vec3 center);

    void setR(float r);
    void setPhi(float phi);
    void setTheta(float theta);

    float getR() const;
    float getPhi() const;
    float getTheta() const;
private:
    void updateXYZ();

    constexpr static const float PI_HALF = glm::pi<float>() / 2.f;
    constexpr static const float TWO_PI = glm::pi<float>() * 2.f;

    float m_r = 0.f;
    float m_phi = 0.f;
    float m_theta = 0.f;

    glm::vec3 m_center = glm::vec3(0.f);
};

#endif