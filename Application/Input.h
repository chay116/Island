//
//  Input.hpp
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#ifndef Input_hpp
#define Input_hpp

#import "CameraController.h"
#import <Foundation/Foundation.h>
#import <simd/types.h>

#define MOUSE_SIMULATE_TOUCH    (0 && TARGET_OS_MAC)

#define USE_VIRTUAL_JOYSTICKS   ((1 && TARGET_OS_IPHONE) || (0 && TARGET_OS_MAC))
#define NUM_VIRTUAL_JOYSTICKS   (1)

// Keys used by this demo; the enum values correlate to their key codes in events.h
enum Controls
{
    // translate (keycodes)
    ControlsForward     = 0x0d, // W
    ControlsBackward    = 0x01, // S
    ControlsStrafeUp    = 0x31, // spacebar
    ControlsStrafeDown  = 0x08, // C
    ControlsStrafeLeft  = 0x00, // A
    ControlsStrafeRight = 0x02, //

    // rotate (keycodes)
    ControlsRollLeft    = 0x0c, // Q
    ControlsRollRight   = 0x0e, // E
    ControlsTurnLeft    = 0x7b, // arrow left
    ControlsTurnRight   = 0x7c, // arrow right
    ControlsTurnUp      = 0x7e, // arrow down
    ControlsTurnDown    = 0x7d, // arrow up

    // additional virtual keys, not linked to a key code; 0x80 and up
    ControlsFast        = 0x80, // Shift
    ControlsSlow        = 0x81, // Control

    ControlsToggleFreezeCulling     = 0x06, // Z
    ControlsControlSecondary        = 0x2F, // .
    ControlsCycleDebugView          = 0x05, // G
    ControlsCycleDebugViewBack      = 0x04, // H
    ControlsToggleLightWireframe    = 0x25, // L
    ControlsCycleLightHeatmap       = 0x28, // K
    ControlsCycleLightEnvironment   = 0x12, // 1
    ControlsCycleLights             = 0x13, // 2
    ControlsCycleScatterScale       = 0x14, // 3
    ControlsToggleTemporalAA        = 0x15, // 4
    ControlsToggleWireframe         = 0x17, // 5
    ControlsToggleOccluders         = 0x16, // 6
    ControlsDebugDrawOccluders      = 0x1A, // 7

    ControlsCycleTextureStreaming   = 0x11, // T

    ControlsTogglePlayback          = 0x09, // V

    ControlsToggleDebugK            = 0x1D, // 0 - for local debugging only!
};

// Stores information about a touch.
@interface Touch : NSObject

    @property simd::float2 pos;         // Current position.
    @property simd::float2 startPos;    // Starting position of touch.
    @property simd::float2 delta;       // Offset of touch this frame.

@end

// Encapsulates all of the inputs to be passed to subsystems.
struct Input
{
    // Array of keys currently pressed.
    NSMutableSet<NSNumber*>*    pressedKeys;
    // Array of keys pressed this frame.
    NSMutableSet<NSNumber*>*    justDownKeys;
    // Mouse movement this frame.
    float                       mouseDeltaX;
    float                       mouseDeltaY;
    // Flag to indicate that the mouse button was pressed.
    bool                        mouseDown;
    // Flag to indicate that the mouse button was pressed.
    bool                        rightMouseDown;
    // Location of mouse pointer when the button was pressed.
    simd::float2                mouseDownPos;
    // Location of mouse pointer.
    simd::float2                mouseCurrentPos;

    // Array of touches in progress.
    NSMutableArray<Touch*>* touches;
    CHA::CameraController*  m_pCameraController;

    void initialize(CHA::CameraController* pCameraController);
    void update();
    void clear();
};

// Stores a pointer to a flag and the key that toggles the flag.
typedef struct
{
    bool*   state;
    char    key;
} StateToggle;

// Stores a pointer to a state and the keys that cycle the state upto the
//  specified maximum.
typedef struct
{
    uint*   state;
    char    forwardKey;
    char    backKey;
    int     max;
} StateCycle;

// Stores a pointer to a state and the keys that cycle the state upto the
//  specified maximum, with a fixed step.
typedef struct
{
    float*  state;
    char    forwardKey;
    char    backKey;
    float   max;
    float   step;
} StateCycleFloat;

// Toggles and cycles states based on the keys pressed during this frame.
void processStateChanges(uint numStateToggles, const StateToggle* stateToggles,
                         uint numStateCycles, const StateCycle* stateCycles,
                         uint numStateCyclesFloat, const StateCycleFloat* stateCyclesFloat,
                         NSSet<NSNumber*>* justDownKeys);


#endif /* Input_hpp */
