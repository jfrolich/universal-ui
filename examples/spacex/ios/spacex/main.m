//
//  main.m
//  reason-mobile
//
//  Created by Jaap Frolich on 28/10/20.
//

#import <UIKit/UIKit.h>
#import <Foundation/Foundation.h>
#ifndef CAML_NAME_SPACE
#define CAML_NAME_SPACE
#endif
#import "caml/callback.h"


@interface DummyClass: NSObject
- (void)dummyMethod;
@end
@implementation DummyClass
- (void)dummyMethod {}
@end


void caml_objc_benchmark() {
  double startSeconds = [[NSDate date] timeIntervalSince1970];
  int numIterations = 10000000;
  DummyClass *dummy = [[DummyClass alloc] init];

  for(int i=0; i<numIterations; i++) {
    [dummy dummyMethod];
  }
  [dummy release];
  double endSeconds = [[NSDate date] timeIntervalSince1970];
  NSLog(@"----- C Benchmark === This took %f seconds", endSeconds - startSeconds);
  NSLog(@"----- C Benchmark === average ns: %f", (endSeconds - startSeconds)/numIterations * 1000000000);
}


int main(int argc, char * argv[]) {
  // caml_objc_benchmark();
  caml_main((char_os **)argv);
  int ret;
  @autoreleasepool {
    ret = UIApplicationMain(argc, argv, nil, @"AppDelegate");
  }
  return ret;
}
