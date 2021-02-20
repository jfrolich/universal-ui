/* -*- objc -*- */


#import <objc/runtime.h>
#import <objc/message.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#ifndef CAML_NAME_SPACE
#define CAML_NAME_SPACE
#endif

#import <caml/mlvalues.h>
#import <caml/alloc.h>
#import <caml/memory.h>
#import <caml/custom.h>
#import <caml/callback.h>
#include <caml/threads.h>

// uuuu uuuu uuuu uuuu xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xuuu
// Intel
//
//                                                                  eeee eeee ttti
// xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxx1
//
//
// Arm64
// ieee eeee e                                                                 ttt
// 1xxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx xxxx
//
// it probably works if the second bit is lost for both platforms

// Reasons to box:
// - There doesn't need to be a bitwise check and fixup (bithshifts)
// - The tagged pointer cannot easily lose a bit
// - We can also represent floats as we have the full 64 bits
// - No branching a value is always boxed (keeps it simple)

//inline static long to_ocaml_int(long obj) {
//  if (!(obj & 4611686018427387904)) {
//    return (((((unsigned long long) obj) >> 63) << 1) + ((unsigned long long) obj << 2) + 1);
//  } else {
//    value wrapper = caml_alloc_small(1, Abstract_tag);
//    Store_field(wrapper, 0, (value)obj);
//    return wrapper;
//  }
//}
//
//inline static long to_c_int(value obj) {
//  if (Is_long(obj)) {
//    return  ((((unsigned long long) obj & 2) << 62) | (unsigned long long) obj >> 2);
//  } else {
//    return *((long *) Data_abstract_val(obj));
//  }
//}

#define Is_Int64(val) (Tag_val(val) == Custom_tag && Wosize_hd(Hd_val(val)) == 1)

static long to_ocaml_int(unsigned long obj) {
  //value wrapper = caml_alloc_small(1, Custom_tag);
  //Store_field(wrapper, 0, (value)obj);
  //NSLog(@"Wosize_hd: %d", Wosize_hd(Hd_val(wrapper)));
  return caml_copy_int64(obj);
}

static long to_c_int(value obj) {
  if (Is_long(obj)) {
    return  (Long_val(obj));
  } else if (Tag_val(obj) == Custom_tag) {
    return Int64_val(obj);
  } else {
    return obj;
  }
  //return Int64_val(obj);
}


//////////////////////////////////////////////////
// CGRect
/////////////////////////////////////////////////

char str_gcrect[] = "objc_cgrect";

static struct custom_operations objc_cgrect = {
  str_gcrect,
  custom_finalize_default,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default
};


value caml_CGRect_make(double x, double y, double width, double height) {
  CAMLparam0();
  value res = caml_alloc_custom(&objc_cgrect, sizeof(CGRect), 0, 1);

  if (res) {
    CGRect *rect = Data_custom_val(res);
    rect->origin.x = x;
    rect->origin.y = y;
    rect->size.width = width;
    rect->size.height = height;
  }
  CAMLreturn(res);
}

value caml_alloc_CGRect() {
  CAMLparam0();
  CAMLreturn(caml_alloc_custom(&objc_cgrect, sizeof(CGRect), 0, 1));
}

void caml_CGRect_log(value gcRect) {
  CAMLparam1(gcRect);
  CGRect *rect = Data_custom_val(gcRect);
  NSLog(@"GCRect: (%f, %f, %f, %f)", rect->origin.x, rect->origin.y, rect->size.width, rect->size.height);
  CAMLreturn0;
}

value caml_objc_getClass(value className) {
  CAMLparam1(className);
  Class class_ = objc_getClass(String_val(className));
  
  NSLog(@"class with pointer %s - %lld", String_val(className), class_);
  CAMLreturn(to_ocaml_int(class_));
}


value caml_objc_msgSend_sel(value obj, value selector) {
  //CAMLparam2(obj, selector);
  //SEL selector_ = sel_registerName(String_val(selector));
  //SEL selector_ = to_c_int(selector);
  //NSLog(@"calling %lld - %s", (id) to_c_int(obj), String_val(selector));
  
  id (*typed_msgSend)(id, SEL) = (id)objc_msgSend;

  id result = typed_msgSend((id) to_c_int(obj), to_c_int(selector));
  //NSLog(@"Result -> %lld", result);
  //CAMLlocal1(result_);
  return(to_ocaml_int(result));
}


value caml_objc_msgSend(value obj, value selector) {
  //CAMLparam2(obj, selector);
  SEL selector_ = sel_registerName(String_val(selector));
  //SEL selector_ = to_c_int(selector);
  //NSLog(@"calling %lld - %s", (id) to_c_int(obj), String_val(selector));
  id (*typed_msgSend)(id, SEL) = (id)objc_msgSend;
  id result = typed_msgSend((id) to_c_int(obj), selector_);
  //NSLog(@"Result -> %lld", result);
  //CAMLlocal1(result_);
  return(to_ocaml_int(result));
}

value caml_objc_msgSend_alt(value obj, value selector) {
  CAMLparam2(obj, selector);
  SEL selector_ = sel_registerName(String_val(selector));
  id (*typed_msgSend)(id, SEL) = (id)objc_msgSend;
  id result = typed_msgSend((id) to_c_int(obj), selector_);
  CAMLreturn(to_ocaml_int(result));
}



void caml_objc_msgSend_noalloc(value obj, value selector) {
  //CAMLparam2(obj, selector);
  //SEL selector_ = sel_registerName(String_val(selector));
  //SEL selector_ = to_c_int(selector);
  //NSLog(@"calling %lld - %s", (id) to_c_int(obj), String_val(selector));
  id (*typed_msgSend)(id, SEL) = (void *)objc_msgSend;
  typed_msgSend((id) to_c_int(obj), to_c_int(selector));
  //NSLog(@"Result -> %lld", result);
  //CAMLlocal1(result_);
  
}


value caml_objc_msgSend_stret(value stret, value obj, value selector) {
  CAMLparam3(stret, obj, selector);
  SEL selector_ = sel_registerName(String_val(selector));
  void *str = Data_custom_val(stret);
  
  //value res = caml_alloc_custom(&objc_cgrect, sizeof(CGRect), 0, 1);
  //
  #if defined(__x86_64__)
  objc_msgSend_stret(str, (id) to_c_int(obj), selector_);
  #else
  struct custom_operations op = *Custom_ops_val(stret);
  if (op.identifier == str_gcrect) {
    CGRect (*typed_msgSend)(id, SEL) = (void*) objc_msgSend;
    CGRect result = typed_msgSend(to_c_int(obj), selector_);
    CGRect *struc = (CGRect *) str;
    struc->origin.x = result.origin.x;
    struc->origin.y = result.origin.y;
    struc->size.height = result.size.height;
    struc->size.width = result.size.width;
  } else {
    CAMLreturn(0);
  }

  
  #endif
  
  CAMLreturn(stret);
}


value caml_objc_msgSendSuper(value obj, value selector) {
  CAMLparam2(obj, selector);
  SEL selector_ = sel_registerName(String_val(selector));
  NSLog(@"calling %p - %s", (long) to_c_int(obj), String_val(selector));
  id obj_ = to_c_int(obj);
  struct objc_super op = {
    receiver: obj_,
    super_class: class_getSuperclass([obj_ class])
  };
  
  id (*typed_msgSend)(id, SEL) = (id)objc_msgSendSuper;
  id result = typed_msgSend(&op, selector_);
  NSLog(@"Result -> %p", result);
  CAMLreturn(to_ocaml_int(result));
}


value caml_objc_msgSend1(value obj, value selector, value arg1) {
  CAMLparam3(obj, selector, arg1);
  
  SEL selector_ = sel_registerName(String_val(selector));
  //NSLog(@"calling %lld - %s - %lld", to_c_int(obj), String_val(selector), to_c_int(arg1));
  id (*typed_msgSend)(id, SEL, id) = (id)objc_msgSend;
  id result = typed_msgSend(to_c_int(obj), selector_, to_c_int(arg1));
  //NSLog(@"Result -> %p", result);
  CAMLreturn(to_ocaml_int(result));
}

long caml_objc_msgSend1_str(value obj, value selector, value arg1) {
  CAMLparam2(obj, selector);
  //NSLog(@"calling %lld - %s - %lld", to_c_int(obj), String_val(selector), to_c_int(arg1));
  //CAMLlocal1(result)
  SEL selector_ = sel_registerName(String_val(selector));
  id (*typed_msgSend)(id, SEL, id) = (id)objc_msgSend;
  
  // both ints and pointers will be shifted, so they are correctly
  // represented
  if (Is_long(arg1) || Is_Int64(arg1)) {
    return caml_objc_msgSend1(obj, selector, arg1);
  } else if (Tag_val(arg1) == Custom_tag) {
    struct custom_operations op = *Custom_ops_val(arg1);
    void *arg1_;
    // we need to special case all structs, otherwise we cannot pass
    // structs by value. I searched for a long time for some magic that
    // would be able to do this and it looks like this is not possible
    if (op.identifier == str_gcrect) {
      arg1_ = Data_custom_val(arg1);
      id (*typed_msgSend)(id, SEL, CGRect) = (id)objc_msgSend;
      CAMLreturn(to_ocaml_int(typed_msgSend(to_c_int(obj), selector_, *(CGRect*) arg1_)));
    } else {
      CAMLreturn(0);
    }
  } else if (Tag_val(arg1) == String_tag) {
    CAMLreturn(to_ocaml_int(typed_msgSend(to_c_int(obj), selector_, caml_stat_strdup(String_val(arg1)))));
  } else {
    CAMLreturn(0);
  }
}

value caml_blablaSetText(value view) {
  CAMLparam1(view);
  CGRect rect =[to_c_int(view) bounds];
  
  NSLog(@"BLABLA: (%f, %f, %f, %f)", rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
    
  CAMLreturn(to_ocaml_int(0.));
}

value caml_objc_msgSend2(value obj, value selector, value arg1, value arg2) {
  CAMLparam4(obj, selector, arg1, arg2);
  SEL selector_ = sel_registerName(String_val(selector));
  id (*typed_msgSend)(id, SEL, id, id) = (id)objc_msgSend;
  CAMLreturn(to_ocaml_int(typed_msgSend(to_c_int(obj), selector_, to_c_int(arg1), to_c_int(arg2))));
}

id invoke_with_args(id obj, SEL selector, int numArgs, ...) {
  NSMethodSignature *sig = [obj methodSignatureForSelector:selector];
  NSInvocation *invocation = [NSInvocation invocationWithMethodSignature:sig];
  invocation.selector = selector;
  invocation.target = obj;

  va_list(argList);
  va_start(argList, numArgs);
  value arg;
  for (int i=2; i < numArgs + 2; i++) {
    arg = va_arg(argList, value);
    if (Is_long(arg) || Is_Int64(arg)) {
      [invocation setArgument: to_c_int(arg) atIndex: i];
    } else if (Tag_val(arg) == Custom_tag) {
      struct custom_operations op = *Custom_ops_val(arg);
      void *arg_;
      if (op.identifier == str_gcrect) {
        arg_ = Data_custom_val(arg);
        [invocation setArgument: ((CGRect*) arg_) atIndex: i];
      }
    } else if (Tag_val(arg) == String_tag) {
      [invocation setArgument: caml_stat_strdup(String_val(arg)) atIndex: i];
    }
  }
  va_end(argList);
  [invocation invoke];
  id result;
  [invocation getReturnValue:&result];
  return result;
}

value caml_objc_msgSend2_str(value obj, value selector, value arg1, value arg2) {
  CAMLparam4(obj, selector, arg1, arg2);
  id obj_ = to_c_int(obj);
  SEL selector_ = sel_registerName(String_val(selector));
  CAMLreturn(to_ocaml_int(invoke_with_args(obj_, selector_, 2, arg1, arg2)));
}

long caml_objc_msgSend3(long obj, value selector, long arg1, long arg2, long arg3) {
  SEL selector_ = sel_registerName(String_val(selector));
  id (*typed_msgSend)(id, SEL, id, id, id) = (id)objc_msgSend;
  return to_ocaml_int(typed_msgSend(to_c_int(obj), selector_, to_c_int(arg1), to_c_int(arg2), to_c_int(arg3)));
}

long caml_objc_msgSend3_str(value obj, value selector, value arg1, value arg2, value arg3) {
  CAMLparam5(obj, selector, arg1, arg2, arg3);
  id obj_ = to_c_int(obj);
  SEL selector_ = sel_registerName(String_val(selector));
  CAMLreturn(to_ocaml_int(invoke_with_args(obj_, selector_, 3, arg1, arg2, arg3)));
}

long caml_objc_sel_registerName(value sel) {
  SEL sel_ = sel_registerName(String_val(sel));
  return to_ocaml_int(sel_);
}

long caml_objc_sel_getName(long sel) {
  char *name = sel_getName((SEL) to_c_int(sel));
  return caml_alloc_initialized_string(strlen(name), name);
}

long caml_objc_class_getName(long cls) {
  char *name = class_getName((Class *) to_c_int(cls));
  return caml_alloc_initialized_string(strlen(name), name);
}

// protocols

long caml_objc_getProtocol(value name) {
  return to_ocaml_int(objc_getProtocol(String_val(name)));
}


static long caml_objc_meth_callback0(id subClass, SEL selector) {
  CAMLparam0();
  CAMLlocal3(subClass_, selector_, result);
  
  caml_acquire_runtime_system();
  
  subClass_ = to_ocaml_int(subClass);
  selector_ = to_ocaml_int(selector);

  NSLog(@"C - Callback0");
  result = caml_callback2(*caml_named_value("objc_meth_callback0"), subClass_, selector_);

  caml_release_runtime_system();
  CAMLreturnT(long, to_c_int(result));
}

static long caml_objc_meth_callback1(id subClass, SEL selector, id arg1) {
  CAMLparam0();
  CAMLlocal4(subClass_, selector_, arg1_, result);
  
  caml_acquire_runtime_system();
  
  subClass_ = to_ocaml_int(subClass);
  selector_ = to_ocaml_int(selector);
  arg1_ = to_ocaml_int(arg1);
  
  NSLog(@"C - Callback1");
  result = caml_callback3(*caml_named_value("objc_meth_callback1"), to_ocaml_int(subClass), to_ocaml_int(selector), to_ocaml_int(arg1));
  
  caml_release_runtime_system();
  CAMLreturnT(long, to_c_int(result));
}

static long caml_objc_meth_callback2(id subClass, SEL selector, id arg1, id arg2) {
  NSLog(@"C - Callback2");
  CAMLparam0();
  CAMLlocalN(args, 5);
  
  CAMLlocal1(result);
  caml_acquire_runtime_system();
  
  args[0] = to_ocaml_int(subClass);
  args[1] = to_ocaml_int(selector);
  args[2] = to_ocaml_int(arg1);
  args[3] = to_ocaml_int(arg2);
  
  result = caml_callbackN(*caml_named_value("objc_meth_callback2"), 4, args);
  
  caml_release_runtime_system();
  CAMLreturnT(long, to_c_int(result));
}

static long caml_objc_meth_callback3(id subClass, SEL selector, id arg1, id arg2, id arg3) {
  CAMLparam0();
  CAMLlocalN(args, 5);
  
  CAMLlocal1(result);
  caml_acquire_runtime_system();
  
  NSLog(@"C - Callback3, %ld, %ld, %ld -- %s", [subClass class], (subClass), (selector), sel_getName(selector));
  
  args[0] = to_ocaml_int(subClass);
  args[1] = to_ocaml_int(selector);
  args[2] = to_ocaml_int(arg1);
  args[3] = to_ocaml_int(arg2);
  args[4] = to_ocaml_int(arg3);

  result = caml_callbackN(*caml_named_value("objc_meth_callback3"), 5, args);
  caml_release_runtime_system();
  CAMLreturnT(long, to_c_int(result));
}

static char caml_objc_meth_callback0_char(id subClass, SEL selector) {
  return (char) caml_objc_meth_callback0(subClass, selector);
}

static char caml_objc_meth_callback1_char(id subClass, SEL selector, id arg1) {
  return (char) caml_objc_meth_callback1(subClass, selector, arg1);
}

static char caml_objc_meth_callback2_char(id subClass, SEL selector, id arg1, id arg2) {
  return (char) caml_objc_meth_callback2(subClass, selector, arg1, arg2);
}

static char caml_objc_meth_callback3_char(id subClass, SEL selector, id arg1, id arg2, id arg3) {
  return (char) caml_objc_meth_callback3(subClass, selector, arg1, arg2, arg3);
}


// return type:
//  0 - v - void
//  1 - @ - id
//  2 - c  - BOOL
//  3 - q - long long

// See https://nshipster.com/type-encodings/ for more info
static char returnTypeToChar(long returnType) {
  switch(returnType) {
    case 0: return 'v';
    case 1: return '@';
    case 2: return 'c';
    case 3: return 'q';
    default: return '@';
  }
}


void caml_objc_class_addMethod(value subClass, value selector, long arity, long returnType) {
  NSLog(@"AddMethod");
  IMP fn;
  // is the type encoding even necessary I think not!
  // it's probably only used for the compiler to keep track of things
  char *typeEncoding;
  char typeEncodingIncludingReturnType[10];
  returnType = to_c_int(returnType);
  arity = to_c_int(arity);
  
  
  NSLog(@"Add method %ld (%ld, %ld)", arity, to_c_int(subClass), to_c_int(selector));
  NSLog(@"returnType: %ld", returnType);
  
  switch (arity) {
    case 0:
      NSLog(@"Arity is 0");
      fn = returnType == 2 ?  (IMP) caml_objc_meth_callback0_char : (IMP) caml_objc_meth_callback0;
      typeEncoding = "@:";
      break;
    case 1:
      fn = returnType == 2 ?  (IMP) caml_objc_meth_callback1_char : (IMP) caml_objc_meth_callback1;
      typeEncoding = "@:@";
      break;
    case 2:
      fn = returnType == 2 ?  (IMP) caml_objc_meth_callback2_char : (IMP) caml_objc_meth_callback2;
      typeEncoding = "@:@@";
      break;
    case 3:
      fn = returnType == 2 ?  (IMP) caml_objc_meth_callback3_char : (IMP) caml_objc_meth_callback3;
      typeEncoding = "@:@@@";
      break;
    default:
      NSLog(@"Arity 4+ not supported yet");
      return;
  };
  sprintf(typeEncodingIncludingReturnType, "%c%s", returnTypeToChar(returnType), typeEncoding);
  class_addMethod((Class)to_c_int(subClass), (SEL) to_c_int(selector), fn, typeEncodingIncludingReturnType);
}

static long caml_objc_block_callback0(id blockPointer) {
  CAMLparam0();
  caml_acquire_runtime_system();
  CAMLlocal2(blockPointer_, result);
  blockPointer_ = to_ocaml_int((long) blockPointer);
  result = caml_callback(*caml_named_value("objc_block_callback0"), blockPointer_);
  caml_release_runtime_system();
  CAMLreturnT(long, to_c_int(result));
}

// copied from OCAML 4.11! This can be removed later
/* [len] is a number of bytes (chars) */
value caml_from_c_string (long len, const char *p)
{
  CAMLparam2(len, p);
  value result = caml_alloc_string (len);
  memcpy((char *)String_val(result), p, len);
  CAMLreturn(result);
}

value caml_nsdata_to_string (NSData* data) {
  CAMLparam1(data);
  //  char nul = '\0';
  long len = [data length];
//  value result = caml_alloc_string (len + 1);
  value result = caml_alloc_string (len);

  const char *p = [data bytes];
  memcpy((char *)String_val(result), p, len);
  CAMLreturn(result);
}

static long caml_objc_block_callback3(id blockPointer, id arg1, id arg2, id arg3) {
  CAMLparam0();
  caml_acquire_runtime_system();
  
  CAMLlocal1(result);
  
  NSLog(@"Block callback!");
  
  
  dispatch_async(dispatch_get_main_queue(), ^{
    CAMLlocalN(args, 4);
    args[0] = to_ocaml_int(blockPointer);
    args[1] = to_ocaml_int(arg1);
    args[2] = to_ocaml_int(arg2);
    args[3] = to_ocaml_int(arg3);
    caml_callbackN(*caml_named_value("objc_block_callback3"), 4, args);
  });
  
  NSLog(@"Did call callback, returning...");
  caml_release_runtime_system();
  CAMLreturnT(long, to_c_int(result));
}

value caml_objc_createBlock0() {
  __block id blockPointer = 0;
  blockPointer = (^{
    return caml_objc_block_callback0(blockPointer);
  });
  
  return to_ocaml_int((long) [[blockPointer copy] autorelease]);
}

value caml_objc_createBlock3() {
  __block id blockPointer = 0;
  blockPointer = [[(^(id arg1, id arg2, id arg3){
    return caml_objc_block_callback3(blockPointer, arg1, arg2, arg3);
  }) copy] autorelease];
  
  return to_ocaml_int((long) blockPointer);
}

long caml_objc_class_getSuperclass(long cls) {
  Class result = class_getSuperclass(to_c_int(cls));
  NSLog(@"Superclass: %p", result);
  return to_ocaml_int(result);
}

void caml_objc_log(value str) {
  NSLog(@"CAMLog: %s", String_val(str));
}

void caml_objc_obj_log(long obj) {
  NSLog(@"CAMLObj: %@", to_c_int(obj));
}

long caml_objc_allocateClassPair(long superClass, value className, long size) {
  NSLog(@"Allocate class pair %p, %s, %d", to_c_int(superClass), String_val(className), Long_val(size));
  long result = to_ocaml_int(objc_allocateClassPair(to_c_int(superClass), String_val(className), Long_val(size)));
  NSLog(@"Class pair %p", to_c_int(result));
  return result;
}

void caml_objc_registerClassPair(value class) {
  NSLog(@"Register class pair %lld", to_c_int(class));
  Class class_ = to_c_int(class);
  
  if (class_) {
    objc_registerClassPair((Class) class_);
  }
}

long caml_objc_class_addProtocol(long cls, long protocol) {
  return class_addProtocol((Class *)to_c_int(cls), (Protocol *)to_c_int(protocol));
}
