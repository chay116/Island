//
//  Camera.cpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#include "Camera.h"
#include "MathUtilities.h"
#include "Assert.h"

using namespace simd;

namespace CHA {
// Helper function that is called after up is updated; will adjust forward/direction to stay orthogonal
// when creating a more defined basis, do not set axis independently, but use rotate() or setBasis() functions to update all at once

void Camera::setDirection()
{
    m_direction =  normalize(matrix3x3_XRotation(radians_from_degrees(m_cameraPitch)) *
                             matrix3x3_YRotation(radians_from_degrees(m_cameraYaw)) *
                             (vector_float3){0.f, 0.f, 1.f});
    m_right = matrix3x3_YRotation(radians_from_degrees(m_cameraYaw - 90)) *
            (vector_float3){0.f, 0.f, 1.f};
    m_up = normalize(cross(m_right, m_direction));
};

// Helper function that is called after forward is updated; will adjust up to stay orthogonal
// when creating a more defined basis, do not set axis independently, but use rotate() or setBasis() functions to update all at once

 
Camera::Camera(vector_float3 position,
         float yaw,
         float pitch,
         float width,
         float height,
         float nearPlane,
         float farPlane)
{
    m_position       = position;
    m_width          = width;
    m_viewAngle      = 3.14159265f / 3.0f;
    m_aspectRatio    = width / height;
    m_nearPlane      = nearPlane;
    m_farPlane       = farPlane;
    m_uniformsDirty  = true;
    m_cameraPitch    = pitch;
    m_cameraYaw      = yaw;
    
    setDirection();
}

// Updates internal uniforms to reflect new direction, up and position properties of the object
void Camera::updateUniforms()
{
    // Generate the view matrix from a matrix lookat
    setDirection();
    
//    printf("Yaw,Pitch: %f %f\n", m_cameraYaw, m_cameraPitch);
//    printf("dirction : %f %f %f\n", m_direction[0], m_direction[1], m_direction[2]);
//    printf("Right    : %f %f %f\n", m_right[0], m_right[1], m_right[2]);
//    printf("position : %f %f %f\n\n", m_position[0], m_position[1], m_position[2]);
    
    m_uniforms.viewMatrix = sInvMatrixLookat(m_position, m_position + m_direction, m_up);
    m_uniforms.projectionMatrix = matrix_perspective_right_hand(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane);

    // Derived matrices
    m_uniforms.viewProjectionMatrix              = m_uniforms.projectionMatrix * m_uniforms.viewMatrix;
    m_uniforms.invProjectionMatrix               = simd_inverse(m_uniforms.projectionMatrix);
    m_uniforms.invOrientationProjectionMatrix    = simd_inverse(m_uniforms.projectionMatrix *  sInvMatrixLookat((float3){0,0,0}, m_direction, m_up));
    m_uniforms.invViewProjectionMatrix           = simd_inverse(m_uniforms.viewProjectionMatrix);
    m_uniforms.invViewMatrix                     = simd_inverse(m_uniforms.viewMatrix);
        // Uniforms are updated and no longer dirty
    m_uniformsDirty = false;
}


// For the uniforms getter, we first check the dirty flag and re-calculate the values if needed
CameraUniforms Camera::getUniforms()
{
    if (m_uniformsDirty) this->updateUniforms();
    return m_uniforms;
}
}
