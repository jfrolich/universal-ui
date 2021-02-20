// almost all objects are NSObjects, and we need a
// namespace here
// all modules are prefixed with NS, to have no namespace collisions

type boolean;
type obj = [ | `obj];
type cls = [ | `cls];
type block = [ | `block];
type sel;
type protocol;
type value('a);

let makeDouble: float => value(float);

let nil: value(obj);

let getClass: string => value(cls);

let sel: string => value(sel);

module Value: {let fromInt64: int64 => value(int64);};

module NSObject: {
  let cls: value(cls);
  // an object can be both a class and an object
  type t('a) = value([< | `cls | `obj | `block] as 'a);

  let toInt64: t('a) => int64;
  let toString: t('a) => string;
  let unsafe_fromInt64: int64 => t('a);
};

module NSBool: {
  type t = value(boolean);
  let true_: t;
  let false_: t;
};

module NSSelector: {
  type t = value(sel);
  let make: string => t;
  let init: t;
  let alloc: t;
  let toInt64: t => int64;
  let unsafe_fromInt64: int64 => t;
  let getName: t => string;
};

module NSClass: {
  type t = value(cls);
  let toInt64: t => int64;
  let toObj: t => NSObject.t(obj);
  let unsafe_fromObj: NSObject.t('a) => t;
  let addProtocol: (t, value(protocol)) => int64;
};

module NSProtocol: {
  type t = value(protocol);
  let toInt64: t => int64;
  let get: string => t;
};

// class for an id that is referring to a classs
// not sure if we need to special case it, can just use obj if that makes more
// sense

module CGRect: {
  type t;
  let make: (float, float, float, float) => t;
  let log: t => unit;
  let alloc: unit => t;
};

/*
 * OCaml values are tagged with the last bit
 * - if they are boxed the last bit is 0
 * - if they are unboxed the last bit is 1
 * we need to bitshift an int to the right to have them represented
 * correctly for C
 * another approach is to convert pointers to an immediate value by shifting and
 * tagging them. However that will add more overhead in calling between
 * OCaml and the Objective-c runtime.
 * Using the current approach will apparently deprecated in OCaml 5
 */

// external get_class: string => option(cls) = "caml_objc_get_class";
// external description: obj => string = "caml_objc_obj_description";

// external create_object: string => option(obj) =
//   "caml_objc_create_objc_object";
// external send_message:
//   (obj, ~selector: string, ~args: array('args)) => option(obj) =
//   "caml_objc_send_message";
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

////////////////////////////////////////////////////////////////////////////////
// msgSend variants
////////////////////////////////////////////////////////////////////////////////
// c baseline: 2.42
// msgSend_unboxed_unit: 3.66 ns
// msgSend_unboxed: 4.01 ns
// msgSend_unboxed + sel: 26.79 ns
// msgSend_unit_noalloc: 10.53 ns
// msgSend_unit: 10.32 ns
// msgSend_sel: 23.02 ns
// msgSend: 41.77 ns
// msgSend_alt: 43.73 ns
// msgSend_sel + sel: 39.26 ns
// send + sel: 28.9267

let msgSend_unit_noalloc: (NSObject.t('a), ~sel: NSSelector.t) => unit;
let msgSend_unit: (NSObject.t('a), ~sel: NSSelector.t) => unit;
let msgSend_sel: (NSObject.t('a), ~sel: NSSelector.t) => NSObject.t('a);
let msgSend: (NSObject.t('a), ~sel: string) => NSObject.t(obj);
let msgSend_unboxed_unit: (NSObject.t('a), ~sel: NSSelector.t) => unit;
let msgSend_alt: (NSObject.t('a), ~sel: string) => NSObject.t(obj);
let msgSend1: (NSObject.t('a), ~sel: string, ~arg: 'b) => NSObject.t(obj);
let msgSend2:
  (NSObject.t('a), ~sel: string, ~arg1: 'b, ~arg2: 'c) => NSObject.t(obj);
let msgSend3:
  (NSObject.t('a), ~sel: string, ~arg1: 'b, ~arg2: 'c, ~arg3: 'd) =>
  NSObject.t(obj);

let msgSend_unboxed: (NSObject.t('a), ~sel: NSSelector.t) => NSObject.t(obj);
let msgSend1_unboxed:
  (NSObject.t('a), ~sel: NSSelector.t, ~arg: value('a)) => NSObject.t(obj);
let msgSend2_unboxed:
  (
    NSObject.t('a),
    ~sel: NSSelector.t,
    ~arg1: value('a),
    ~arg2: value('b)
  ) =>
  NSObject.t(obj);
let msgSend3_unboxed:
  (
    NSObject.t('a),
    ~sel: NSSelector.t,
    ~arg1: value('a),
    ~arg2: value('b),
    ~arg3: value('c)
  ) =>
  NSObject.t(obj);

// msgSendx_str are if you need to pass struct arguments by value
let msgSend1_str:
  (NSObject.t('a), ~sel: string, ~arg: 'b) => NSObject.t(obj);

let msgSend2_str:
  (NSObject.t('a), ~sel: string, ~arg1: 'b, ~arg2: 'c) => NSObject.t(obj);

let msgSend3_str:
  (NSObject.t('a), ~sel: string, ~arg1: 'b, ~arg2: 'c, ~arg3: 'd) =>
  NSObject.t(obj);

// msgSend_stret can be used if the message returns a struct
// (and not a pointer to a struct)
let msgSend_stret: (~ret: 'a, NSObject.t('b), ~sel: string) => 'a;

let msgSendSuper: (NSObject.t('a), ~sel: string) => NSObject.t(obj);

let send0: (NSSelector.t, NSObject.t('a)) => NSObject.t(obj);
let send1: (NSSelector.t, NSObject.t('a), value('a)) => NSObject.t(obj);
let send2:
  (NSSelector.t, NSObject.t('a), value('b), value('c)) => NSObject.t(obj);
let send3:
  (NSSelector.t, NSObject.t('a), value('b), value('c), value('d)) =>
  NSObject.t(obj);

let allocateClassPair: (NSClass.t, ~name: string, ~size: int) => NSClass.t;
let getSuperclass: NSClass.t => NSClass.t;

let registerClassPair: NSClass.t => unit;

let class_getName: NSClass.t => string;

let log: string => unit;
let logObj: NSObject.t('a) => unit;

let classToObj: string => NSObject.t(obj);
external blablaSetText: NSObject.t('a) => NSObject.t('a) =
  "caml_blablaSetText";

//// adding methods
type return;
let returnBool: bool => return;
let returnInt: int => return;
let returnUnit: unit => return;
let returnObj: NSObject.t('a) => return;

////////////////////////////////////////////////////////////////////////////////
// Method callbacks
////////////////////////////////////////////////////////////////////////////////

type returnType =
  | Void
  | Object
  | Bool
  | Int;

let returnTypeToInt: returnType => int;
let addMethod_: (NSClass.t, NSSelector.t, 'a, 'b) => unit;

let addMethod: (string, ~returnType: returnType, NSClass.t) => unit;

let addCallback:
  (NSSelector.t, ~fn: NSObject.t('a) => return, NSObject.t('a)) => unit;
let addCallback1:
  (
    NSSelector.t,
    ~fn: (NSObject.t('a), value('a)) => return,
    NSObject.t('a)
  ) =>
  unit;
let addCallback2:
  (
    NSSelector.t,
    ~fn: (NSObject.t('a), value('b), value('c)) => return,
    NSObject.t('a)
  ) =>
  unit;
let addCallback3:
  (
    NSSelector.t,
    ~fn: (NSObject.t('a), value('b), value('c), value('d)) => return,
    NSObject.t('a)
  ) =>
  unit;

let removeCallback: (NSObject.t('a), NSSelector.t) => unit;
let removeCallback1: (NSObject.t('a), NSSelector.t) => unit;
let removeCallback2: (NSObject.t('a), NSSelector.t) => unit;
let removeCallback3: (NSObject.t('a), NSSelector.t) => unit;

////////////////////////////////////////////////////////////////////////////////
// Blocks
////////////////////////////////////////////////////////////////////////////////

let createBlock3:
  ((NSObject.t('a), NSObject.t('b), NSObject.t('c)) => return) =>
  NSObject.t(block);

////////////////////////////////////////////////////////////////////////////////
// Bindings to higher level stuff
////////////////////////////////////////////////////////////////////////////////

module NSString: {
  let cls: NSClass.t;
  let make: string => NSObject.t('a);
};

module UILabel: {let cls: NSClass.t;};

module UIColor: {
  let cls: NSClass.t;

  let redColor: unit => NSObject.t('a);
  let whiteColor: unit => NSObject.t('a);
};

let fromCString: (NSObject.t('a), int64) => string;

module NSData: {let toString: NSObject.t('a) => string;};

let alloc: NSSelector.t;
let init: NSSelector.t;
