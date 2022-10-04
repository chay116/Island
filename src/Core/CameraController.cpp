//
//  CameraController.cpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/27.
//

#import "WorldRenderer.h"
#include "CameraController.h"


// Internal structure of a keypoint.

namespace CHA {

CameraController::CameraController(Camera *camera) : m_pCamera(camera)
{
    m_keypoints[0].position     = camera->getPosition();
    m_keypoints[0].cameraPitch  = camera->getPitch();
    m_keypoints[0].cameraYaw    = camera->getYaw();
}

void CameraController::setCamera()
{
    m_pCamera->setPosition(m_keypoints[idx_now].position);
    m_pCamera->setPitch(m_keypoints[idx_now].cameraPitch);
    m_pCamera->setYaw(m_keypoints[idx_now].cameraYaw);
}

uint8_t CameraController::nextIdx()
{
    uint8_t idx = (idx_now + 1) % 10;
    while (!m_keypoints[idx].used)
    {
        idx = (idx + 1) % 10;
    }
    return idx;
}

uint8_t CameraController::findUnusedIdx()
{
    uint8_t idx = (idx_now + 1) % 10;
    while (m_keypoints[idx].used)
    {
        idx = (idx + 1) % 10;
    }
    return idx;
}

void CameraController::updateKeypoint()
{
    m_keypoints[idx_now].position = m_pCamera->getPosition();
    m_keypoints[idx_now].cameraPitch = m_pCamera->getPitch();
    m_keypoints[idx_now].cameraYaw = m_pCamera->getYaw();
}

void CameraController::addKeypoint(float pitch, float yaw, simd::float3 pos)
{
    if (total_idx == 10) return ;
    total_idx++;
    uint8_t idx = findUnusedIdx();
    m_keypoints[idx].position = pos;
    m_keypoints[idx].cameraPitch = pitch;
    m_keypoints[idx].cameraYaw = yaw;
    m_keypoints[idx].used = true;
    setCamera();
}

void CameraController::updateKeypoint(uint8_t index, float pitch, float yaw, simd::float3 pos)
{
    m_keypoints[index].position = pos;
    m_keypoints[index].cameraPitch = pitch;
    m_keypoints[index].cameraYaw = yaw;

    if (index == idx_now) setCamera();
}

void CameraController::nextKeypoint()
{
    updateKeypoint();
    idx_now = nextIdx();
    setCamera();
}

void CameraController::clearKeypoint()
{
    if (total_idx == 1) return ;
    m_keypoints[idx_now].used = false;
    idx_now = nextIdx();
    setCamera();
}

void CameraController::clearKeypoint(uint8_t index)
{
    if (total_idx == 1) return ;
    if (index == idx_now) clearKeypoint();
    else m_keypoints[index].used = false;
}


void CameraController::moveForward()
{
    m_pCamera->setPosition(m_pCamera->getPosition()
                           + m_keypoints[idx_now].movementSpeed * m_pCamera->getForward());
}

void CameraController::moveLeft()
{
    m_pCamera->setPosition(m_pCamera->getPosition()
                           + m_keypoints[idx_now].movementSpeed * m_pCamera->getLeft());
}

void CameraController::moveRight()
{
    m_pCamera->setPosition(m_pCamera->getPosition()
                           + m_keypoints[idx_now].movementSpeed * m_pCamera->getRight());
}

void CameraController::moveBackward()
{
    m_pCamera->setPosition(m_pCamera->getPosition()
                           + m_keypoints[idx_now].movementSpeed * m_pCamera->getBackward());
}

void CameraController::moveUpward()
{
    m_pCamera->setPosition(m_pCamera->getPosition()
                           + m_keypoints[idx_now].movementSpeed * m_pCamera->getUp());
}

void CameraController::moveDownward()
{
    m_pCamera->setPosition(m_pCamera->getPosition()
                           + m_keypoints[idx_now].movementSpeed * m_pCamera->getDown());
}


void CameraController::mouseMove(float deltaX, float deltaY)
{
    float newYaw = m_pCamera->getYaw() - deltaX * m_cameraRotSpeed;
    float newPitch = m_pCamera->getPitch() - deltaY * m_cameraRotSpeed;
    
    
    if (newYaw < 0.0f) newYaw += 360.0f;
    if (newYaw > 360.0f) newYaw -= 360.0f;

    if (newPitch > 89.0f) newPitch = 89.0f;
    if (newPitch < -89.0f) newPitch = -89.0f;

    m_pCamera->setYaw(newYaw);
    m_pCamera->setPitch(newPitch);
}

}
