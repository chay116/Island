//
//  GameViewController.h
//  Island
//
//  Created by Chaeyoung Lim on 2022/09/24.
//

#ifndef ViewController_h
#define ViewController_h

#import <Metalkit/MetalKit.h>
#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

@interface View : MTKView
- (BOOL)acceptsFirstResponder;
- (BOOL)acceptsFirstMouse:(NSEvent *)event;
@end

@interface ViewController : NSViewController<MTKViewDelegate>
@end

#endif
