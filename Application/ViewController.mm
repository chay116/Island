//
//  GameViewController.m
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#import "Input.h"
#import "ViewController.h"
#import "WorldRenderer.h"
#import <simd/simd.h>


@implementation View

// signal that we want our window be the first responder of user inputs
- (BOOL)acceptsFirstResponder { return YES; }

- (void)awakeFromNib
{
    // on osx, create a tracking area to keep track of the mouse movements and events
    NSTrackingAreaOptions options = (NSTrackingActiveAlways
                                     | NSTrackingInVisibleRect
                                     | NSTrackingMouseEnteredAndExited
                                     | NSTrackingMouseMoved);

    NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                        options:options
                                                          owner:self
                                                       userInfo:nil];
    [self addTrackingArea:area];
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    return YES;
}

@end


@implementation ViewController
{
    id <MTLDevice>          _device;
    MTL::Device*            m_pDevice;
    Input                   m_input;
    MTKView*                m_pView;
    CHA::WorldRenderer*     m_pRenderer;
    float                   m_centerHeight;
    float                   m_centerWidth;
    
}

// capture shift and ctrl
-(void)flagsChanged:(NSEvent*)event
{
    NSLog(@"flagsChanged %ui", event.keyCode);
    if(event.modifierFlags&NSEventModifierFlagShift)
        [m_input.pressedKeys addObject:@(ControlsFast)];
    else
        [m_input.pressedKeys removeObject:@(ControlsFast)];

    if (event.modifierFlags&NSEventModifierFlagControl)
        [m_input.pressedKeys addObject:@(ControlsSlow)];
    else
        [m_input.pressedKeys removeObject:@(ControlsSlow)];
}

// capture mouse and keyboard events
-(void)mouseExited:(NSEvent *)event
{
    m_input.rightMouseDown = false;
}
-(void)mouseMoved:(NSEvent *)event
{
//    m_input.mouseDeltaX = (event.locationInWindow.x - m_centerWidth) / m_centerWidth;
//    m_input.mouseDeltaY = (event.locationInWindow.y - m_centerHeight) / m_centerHeight;

    m_input.mouseDeltaX = event.deltaX;
    m_input.mouseDeltaY = -event.deltaY;
    
//    if (m_input.rightMouseDown)
//    {
//        CGWarpMouseCursorPosition({event.window.frame.size.width, event.window.frame.size.height});
//    }
}

-(void)rightMouseDown:(NSEvent *)event
{
    m_input.rightMouseDown = !m_input.rightMouseDown;
}

-(void)rightMouseUp:(NSEvent *)event
{
//    m_input.rightMouseDown = false;
}

-(void)mouseDown:(NSEvent *)event
{
    m_input.mouseDown = true;
}

-(void)mouseUp:(NSEvent *)event
{
    m_input.mouseDown = false;
}

-(void)mouseDragged:(NSEvent *)event
{

}

-(void)rightMouseDragged:(NSEvent *)event
{
//    m_input.mouseDeltaX = (float)event.deltaX;
//    m_input.mouseDeltaY = (float)event.deltaY;
}

-(void)keyUp:(NSEvent*)event
{
    [m_input.pressedKeys removeObject:@(event.keyCode)];
}

-(void)keyDown:(NSEvent*)event
{
    if (!event.ARepeat)
    {
        [m_input.pressedKeys addObject:@(event.keyCode)];
        [m_input.justDownKeys addObject:@(event.keyCode)];
    }
}


- (void)viewDidLoad
{
    [super viewDidLoad];

    m_pView = (MTKView *)self.view;

    m_pDevice = MTL::CreateSystemDefaultDevice();
    m_pView.device = (__bridge id<MTLDevice>)m_pDevice;
    m_pView.delegate = self;

    
    CGSize size = m_pView.drawableSize;
    m_centerHeight  = size.height / 2;
    m_centerWidth   = size.width / 2;
    m_pRenderer = new CHA::WorldRenderer( m_pDevice, MTL::Size::Make(size.width, size.height, 0) );

    m_input.initialize(m_pRenderer->getCameraController());
    m_pView.depthStencilPixelFormat = (MTLPixelFormat) m_pRenderer->depthStencilTargetPixelFormat();
    m_pView.colorPixelFormat = (MTLPixelFormat) m_pRenderer->colorTargetPixelFormat();
    m_pView.sampleCount = 1;

    m_pRenderer->drawableSizeWillChange( MTL::Size::Make(size.width, size.height, 0), MTL::StorageModePrivate);
}


- (void)dealloc
{
    delete m_pRenderer;
    m_pDevice->release();
}


- (void)drawInMTKView:(nonnull MTKView *)view
{
    m_input.update();
    m_pRenderer->drawInView((__bridge void*)m_pView);
    m_input.clear();
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    m_pRenderer->drawableSizeWillChange( MTL::Size::Make(size.width, size.height, 0), MTL::StorageModePrivate);
    m_centerHeight  = size.height;
    m_centerWidth   = size.width;
}
@end






