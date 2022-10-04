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

void Camera::setDirection()
{
    m_direction =  normalize(matrix3x3_XRotation(radians_from_degrees(m_cameraPitch)) *
                             matrix3x3_YRotation(radians_from_degrees(m_cameraYaw)) *
                             (vector_float3){0.f, 0.f, 1.f});
    m_right = matrix3x3_YRotation(radians_from_degrees(m_cameraYaw - 90)) *
            (vector_float3){0.f, 0.f, 1.f};
    m_up = normalize(cross(m_right, m_direction));
};

 
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
    m_uniformsDirty  = false;
    m_cameraPitch    = pitch;
    m_cameraYaw      = yaw;
    
    setDirection();
}

void Camera::updateUniforms()
{
    setDirection();
    
    
    m_uniforms.viewMatrix = sInvMatrixLookat(m_position, m_position + m_direction, m_up);
    m_uniforms.projectionMatrix = matrix_perspective_right_hand(m_viewAngle, m_aspectRatio, m_nearPlane, m_farPlane);
    m_uniforms.viewProjectionMatrix              = m_uniforms.projectionMatrix * m_uniforms.viewMatrix;
    m_uniforms.invProjectionMatrix               = simd_inverse(m_uniforms.projectionMatrix);
    m_uniforms.invOrientationProjectionMatrix    = simd_inverse(m_uniforms.projectionMatrix *  sInvMatrixLookat((float3){0,0,0}, m_direction, m_up));
    m_uniforms.invViewProjectionMatrix           = simd_inverse(m_uniforms.viewProjectionMatrix);
    m_uniforms.invViewMatrix                     = simd_inverse(m_uniforms.viewMatrix);

    float4x4 transp_vpm = simd::transpose(m_uniforms.viewProjectionMatrix);
    m_uniforms.frustumPlanes[0] = sPlaneNormalize(transp_vpm.columns[3] +
                                                  transp_vpm.columns[0]); // left plane eq
    m_uniforms.frustumPlanes[1] = sPlaneNormalize(transp_vpm.columns[3] -
                                                  transp_vpm.columns[0]); // right plane eq
    m_uniforms.frustumPlanes[2] = sPlaneNormalize(transp_vpm.columns[3] +
                                                  transp_vpm.columns[1]); // up plane eq
    m_uniforms.frustumPlanes[3] = sPlaneNormalize(transp_vpm.columns[3] -
                                                  transp_vpm.columns[1]); // down plane eq
    m_uniforms.frustumPlanes[4] = sPlaneNormalize(transp_vpm.columns[3] +
                                                  transp_vpm.columns[2]);    // near plane eq
    m_uniforms.frustumPlanes[5] = sPlaneNormalize(transp_vpm.columns[3] -
                                                  transp_vpm.columns[2]);    // far
    
    m_uniformsDirty = false;
}

CameraUniforms Camera::getUniforms()
{
    if (m_uniformsDirty) this->updateUniforms();
    return m_uniforms;
}
}
