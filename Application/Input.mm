//
//  Input.cpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#include "Input.h"
#include "CameraController.h"
#import <simd/simd.h>

void Input::initialize(CHA::CameraController* pCameraController)
{
    pressedKeys         = [NSMutableSet set];
    justDownKeys        = [NSMutableSet set];
    touches             = [NSMutableArray array];
    m_pCameraController = pCameraController;
}

void Input::update()
{
    if(rightMouseDown)
    {
        m_pCameraController->mouseMove(mouseDeltaX, mouseDeltaY);
    }
    mouseDeltaX = 0.0f;
    mouseDeltaY = 0.0f;
    if([pressedKeys containsObject: @(ControlsForward)]) m_pCameraController->moveForward();
    if([pressedKeys containsObject: @(ControlsBackward)]) m_pCameraController->moveBackward();
    if([pressedKeys containsObject: @(ControlsStrafeLeft)]) m_pCameraController->moveLeft();
    if([pressedKeys containsObject: @(ControlsStrafeRight)]) m_pCameraController->moveRight();
    if([pressedKeys containsObject: @(ControlsStrafeUp)]) m_pCameraController->moveUpward();
    if([pressedKeys containsObject: @(ControlsStrafeDown)]) m_pCameraController->moveDownward();
}

void Input::clear()
{
    mouseDeltaX = 0.0f;
    mouseDeltaY = 0.0f;

    [justDownKeys removeAllObjects];
    [touches removeAllObjects];
}
