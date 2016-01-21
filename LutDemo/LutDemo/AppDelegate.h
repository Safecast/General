//
//  AppDelegate.h
//  LutDemo
//
//  Created by Nicholas Dolezal on 1/20/16.
//
//

// Docs: see "AppDelegate.m"

#import <Cocoa/Cocoa.h>
#import "gbThreadGuard.h"
#import "gbLUT.h"

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    gbLUT*         _gb_lut;
    gbThreadGuard* _lut_thread_guard;
}


@end

