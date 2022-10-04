//
//  CameraController.hpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/27.
//

#ifndef CameraController_hpp
#define CameraController_hpp

#import "Camera.h"
#import <vector>

// Stores a list of keypoints.
//  Supports attaching to a camera, then updates to the controller time with
//  `updateTimeInSeconds` updates the camera transform.
//  Keypoints can be added and removed and serialized to/from file.

namespace CHA {
struct CameraKeypoint
{
    CameraKeypoint() = default;
    // Constructors.
    CameraKeypoint(float pitch, float yaw, simd::float3 pos) :
    position(pos)
    ,cameraPitch(pitch)
    ,cameraYaw(yaw)
    {}

    simd::float3 position;  // Position of the keypoint.
    float        cameraPitch;
    float        cameraYaw;
    float        movementSpeed { 20.f };
    bool         used {false};
};


class CameraController {
public:
    CameraController() = delete;
    CameraController(Camera *camera);
    
    // Keypoint access and modification.
    void addKeypoint(float pitch, float yaw, simd::float3 pos);

    void updateKeypoint(uint8_t index, float pitch, float yaw, simd::float3 pos);
    void nextKeypoint();
    void clearKeypoint();
    void clearKeypoint(uint8_t index);

    
    // Control Moving
    void moveForward();
    void moveLeft();
    void moveRight();
    void moveBackward();
    void moveUpward();
    void moveDownward();
    void mouseMove(float deltaX, float deltaY);
    

private:
    void setCamera();
    uint8_t nextIdx();
    uint8_t findUnusedIdx();
    void updateKeypoint();
    
    float                       m_cameraRotSpeed { 2.f };
    Camera*                     m_pCamera;
    CameraKeypoint              m_keypoints[10];
    uint8_t                     total_idx { 1 };
    uint8_t                     idx_now { 0 };
};



};

#endif /* CameraController_hpp */
