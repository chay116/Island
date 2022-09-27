//
//  Camera.hpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#ifndef Camera_h
#define Camera_h

#include "WorldRenderer_shared.h"
#include <simd/simd.h>

namespace CHA {
class Camera
{
private:
    void setDirection();

    // Internally generated camera uniforms used/defined by the renderer
    CameraUniforms m_uniforms;

    // A boolean value that denotes if the intenral uniforms structure needs rebuilding
    bool m_uniformsDirty;
    float m_viewAngle;
    float m_width;
    float m_nearPlane;
    float m_farPlane;
    float m_aspectRatio;
    float m_cameraPitch;
    float m_cameraYaw;
    vector_float3 m_direction;
    vector_float3 m_position;
    vector_float3 m_up;
    vector_float3 m_right;
    
public:
    Camera(vector_float3 position,
             float yaw,
             float pitch,
             float width,
             float height,
             float nearPlane,
             float farPlane);

    // Updates internal uniforms from the various properties
    void updateUniforms();

    // For all the setter functions, we update the instance variables and set the dirty flag
    void setNearPlane(float newNearPlane)           { m_nearPlane    = newNearPlane;                m_uniformsDirty = true; }
    void setFarPlane(float newFarPlane)             { m_farPlane     = newFarPlane;                 m_uniformsDirty = true; }
    void setAspectRatio(float newAspectRatio)       { m_aspectRatio  = newAspectRatio;              m_uniformsDirty = true; }
    void setViewAngle(float newAngle )              { m_viewAngle    = newAngle;                    m_uniformsDirty = true; }
    void setWidth(float newWidth)                   { m_width        = newWidth;                    m_uniformsDirty = true; }
    void setPosition(vector_float3 newPosition)     { m_position     = newPosition;                 m_uniformsDirty = true; }
    void setPitch(float newPitch)                   { m_cameraPitch  = newPitch;                    m_uniformsDirty = true; }
    void setYaw(float newYaw)                       { m_cameraYaw    = newYaw;                      m_uniformsDirty = true; }
    
    // Getters for posing properties
    float getViewAngle()                        { return m_viewAngle; };
    float getWidth()                            { return m_width; };
    float getNearPlane()                        { return m_nearPlane; };
    float getFarPlane()                         { return m_farPlane; };
    float getAspectRatio()                      { return m_aspectRatio; };
    vector_float3 getPosition()                 { return m_position; };
    vector_float3 getDirection()                { return m_direction; };
    bool isPerspective()                        { return m_viewAngle != 0.0f; };
    bool isParallel()                           { return m_viewAngle == 0.0f; };
    
    matrix_float4x4 viewMatrix()                { return m_uniforms.viewMatrix; };
    matrix_float4x4 projectionMatrix()          { return m_uniforms.projectionMatrix; };
    matrix_float4x4 viewProjectionMatrix()      { return m_uniforms.viewProjectionMatrix; };
    matrix_float4x4 invViewProjectionMatrix()   { return m_uniforms.invViewProjectionMatrix; };
    matrix_float4x4 invProjectionMatrix()       { return m_uniforms.invProjectionMatrix; };
    matrix_float4x4 invViewMatrix()             { return m_uniforms.invViewMatrix; };

    CameraUniforms getUniforms();
    
    // All other directions are derived from the -up and m_direction instance variables
    vector_float3 getLeft()        { return -m_right;       };
    vector_float3 getRight()       { return m_right;        };
    vector_float3 getUp()          { return m_up;           };
    vector_float3 getDown()        { return -m_up;          };
    vector_float3 getForward()     { return m_direction;    };
    vector_float3 getBackward()    { return -m_direction;   };
    float         getPitch()       { return m_cameraPitch;  };
    float         getYaw()         { return m_cameraYaw;    };
};
}

#endif /* Camera_h */
