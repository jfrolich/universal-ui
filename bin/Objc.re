// almost all objects are NSObjects, and we need a
// namespace here
// all modules are prefixed with NS, to have no namespace collisions

type boolean;
type obj = [ | `obj];
type cls = [ | `cls];
type block = [ | `block];
type sel;
type protocol;
type value('a) = int64;

module Value = {
  let fromInt64 = value => value;
};

let nil = Int64.zero;

let makeDouble = (floatValue: float) => {
  Int64.bits_of_float(floatValue);
};

[@noalloc]
external getClass: string => [@unboxed] int64 =
  "caml_objc_getClass" "objc_getClass";

[@noalloc]
external sel: string => [@unboxed] int64 =
  "caml_objc_sel_registerName" "sel_registerName";

module NSObject = {
  let cls = getClass("NSObject");
  // opaque type for `id`
  type t('a) = value([< | `cls | `obj | `block] as 'a);

  external toInt64: int64 => int64 = "%identity";
  let toString = t => t |> toInt64 |> Int64.to_string;
  external unsafe_fromInt64: int64 => int64 = "%identity";
};

module NSBool = {
  type t = int64;
  let true_ = 1L;
  let false_ = 0L;
};

module NSSelector = {
  let make = sel;
  type t = int64;
  let init = sel("init");
  let alloc = sel("alloc");
  external toInt64: t => int64 = "%identity";
  external unsafe_fromInt64: int64 => t = "%identity";
  external getName: t => string = "caml_objc_sel_getName";
};

module NSClass = {
  type t = int64;
  external toInt64: int64 => int64 = "%identity";
  external toObj: int64 => int64 = "%identity";
  external unsafe_fromObj: int64 => int64 = "%identity";

  external addProtocol:
    ([@unboxed] int64, [@unboxed] int64) => [@unboxed] int64 =
    "caml_objc_class_addProtocol" "class_addProtocol";
};

module NSProtocol = {
  type t = int64;
  external toInt64: int64 => int64 = "%identity";

  external get: string => [@unboxed] int64 =
    "caml_objc_getProtocol" "objc_getProtocol";
};

// class for an id that is referring to a classs
// not sure if we need to special case it, can just use obj if that makes more
// sene

module CGRect = {
  type t;
  external make:
    ([@unboxed] float, [@unboxed] float, [@unboxed] float, [@unboxed] float) =>
    t =
    "no_byte" "caml_CGRect_make";
  external log: t => unit = "caml_CGRect_log";

  external alloc: unit => t = "caml_alloc_CGRect";
};

[@noalloc]
external msgSend_unit_noalloc: (int64, ~sel: int64) => unit =
  "caml_objc_msgSend_noalloc";
external msgSend_unit: (int64, ~sel: int64) => unit =
  "caml_objc_msgSend_noalloc";
external msgSend_sel: (int64, ~sel: int64) => int64 = "caml_objc_msgSend_sel";
external msgSend: (int64, ~sel: string) => int64 = "caml_objc_msgSend";

[@noalloc]
external msgSend_unboxed_unit:
  ([@unboxed] int64, ~sel: [@unboxed] int64) => unit =
  "caml_objc_msgSend" "objc_msgSend";

external msgSend_alt: (int64, ~sel: string) => int64 = "caml_objc_msgSend_alt";

external msgSend1: (int64, ~sel: string, ~arg: 'a) => int64 =
  "caml_objc_msgSend1";
external msgSend2: (int64, ~sel: string, ~arg1: 'a, ~arg2: 'b) => int64 =
  "caml_objc_msgSend2";
external msgSend3:
  (int64, ~sel: string, ~arg1: 'a, ~arg2: 'b, ~arg3: 'c) => int64 =
  "caml_objc_msgSend3";

[@unboxed] [@noalloc]
external msgSend_unboxed: (int64, ~sel: int64) => int64 =
  "caml_objc_msgSend" "objc_msgSend";

[@unboxed] [@noalloc]
external msgSend1_unboxed: (int64, ~sel: int64, ~arg: int64) => int64 =
  "caml_objc_msgSend1" "objc_msgSend";

[@unboxed] [@noalloc]
external msgSend2_unboxed:
  (int64, ~sel: int64, ~arg1: int64, ~arg2: int64) => int64 =
  "caml_objc_msgSend2" "objc_msgSend";

[@unboxed] [@noalloc]
external msgSend3_unboxed:
  (int64, ~sel: int64, ~arg1: int64, ~arg2: int64, ~arg3: int64) => int64 =
  "caml_objc_msgSend3" "objc_msgSend";

// msgSendx_str are if you need to pass struct arguments by value
external msgSend1_str: (int64, ~sel: string, ~arg: 'a) => int64 =
  "caml_objc_msgSend1_str";

external msgSend2_str: (int64, ~sel: string, ~arg1: 'a, ~arg2: 'b) => int64 =
  "caml_objc_msgSend2_str";

external msgSend3_str:
  (int64, ~sel: string, ~arg1: 'a, ~arg2: 'b, ~arg3: 'c) => int64 =
  "caml_objc_msgSend3_str";

// msgSend_stret can be used if the message returns a struct
// (and not a pointer to a struct)
external msgSend_stret: (~ret: 'a, int64, ~sel: string) => 'a =
  "caml_objc_msgSend_stret";

external msgSendSuper: (int64, ~sel: string) => int64 =
  "caml_objc_msgSendSuper";

let send0 = (selector, obj) => {
  msgSend_unboxed(obj, ~sel=selector);
};
let send1 = (selector, arg, obj) => {
  msgSend1_unboxed(obj, ~sel=selector, ~arg);
};
let send2 = (selector, arg1, arg2, obj) => {
  msgSend2_unboxed(obj, ~sel=selector, ~arg1, ~arg2);
};
let send3 = (selector, arg1, arg2, arg3, obj) => {
  msgSend3_unboxed(obj, ~sel=selector, ~arg1, ~arg2, ~arg3);
};

external allocateClassPair:
  ([@unboxed] int64, ~name: string, ~size: [@untagged] int) => [@unboxed] int64 =
  "caml_objc_allocateClassPair" "objc_allocateClassPair";
// external allocateClassPair: (cls, ~name: string, ~size: int) => cls =
//   "caml_objc_allocateClassPair";

external getSuperclass: int64 => int64 = "caml_objc_class_getSuperclass";

external registerClassPair: int64 => unit = "caml_objc_registerClassPair";

external class_getName: int64 => string = "caml_objc_class_getName";

external log: string => unit = "caml_objc_log";
external logObj: int64 => unit = "caml_objc_obj_log";

let classToObj = className => getClass(className);

external blablaSetText: int64 => int64 = "caml_blablaSetText";

//// adding methods
type return = int64;
let returnBool: bool => return =
  value => {
    value == true ? 1L : 0L;
  };
let returnInt: int => return = value => Obj.magic(Int64.of_int(value));

external returnUnit: unit => int64 = "%identity";
external returnObj: int64 => int64 = "%identity";

let methods: Hashtbl.t((int64, int64), int64 => int64) =
  Hashtbl.create(100);
let methods1: Hashtbl.t((int64, int64), (int64, int64) => return) =
  Hashtbl.create(100);
let methods2: Hashtbl.t((int64, int64), (int64, int64, int64) => return) =
  Hashtbl.create(100);
let methods3:
  Hashtbl.t((int64, int64), (int64, int64, int64, int64) => return) =
  Hashtbl.create(100);

let callback0 = (instance: int64, selector: int64) => {
  log(
    "Callback0: "
    ++ NSSelector.getName(selector)
    ++ "from class: "
    ++ (msgSend(instance, ~sel="class") |> class_getName)
    ++ "Instance: "
    ++ NSObject.toString(instance),
  );
  let cls = instance |> msgSend(~sel="class");
  switch (
    Hashtbl.find_opt(methods, (cls, selector)),
    Hashtbl.find_opt(methods, (instance, selector)),
  ) {
  | (_, Some(fn)) => fn(instance)
  | (Some(fn), _) => fn(instance)
  | (None, None) =>
    log("Not found!!");
    0L;
  };
};
Callback.register("objc_meth_callback0", callback0);

let callback1 = (instance: int64, selector: int64, arg1: int64) => {
  log("Callback1");
  let cls = instance |> msgSend(~sel="class");
  switch (
    Hashtbl.find_opt(methods1, (cls, selector)),
    Hashtbl.find_opt(methods1, (instance, selector)),
  ) {
  | (_, Some(fn)) => fn(instance, arg1)
  | (Some(fn), _) => fn(instance, arg1)
  | (None, None) => 0L
  };
};
Callback.register("objc_meth_callback1", callback1);

let callback2 = (instance: int64, selector: int64, arg1: int64, arg2: int64) => {
  log("Callback2");
  let cls = instance |> msgSend(~sel="class");
  switch (
    Hashtbl.find_opt(methods2, (cls, selector)),
    Hashtbl.find_opt(methods2, (instance, selector)),
  ) {
  | (_, Some(fn)) => fn(instance, arg1, arg2)
  | (Some(fn), _) => fn(instance, arg1, arg2)
  | (None, None) => 0L
  };
};
Callback.register("objc_meth_callback2", callback2);

let callback3 =
    (instance: int64, selector: int64, arg1: int64, arg2: int64, arg3: int64) => {
  let cls = instance |> msgSend(~sel="class");
  log(
    "Callback3!" ++ Int64.to_string(cls) ++ "," ++ Int64.to_string(selector),
  );
  switch (
    Hashtbl.find_opt(methods3, (cls, selector)),
    Hashtbl.find_opt(methods3, (instance, selector)),
  ) {
  | (_, Some(fn)) => fn(instance, arg1, arg2, arg3)
  | (Some(fn), _) => fn(instance, arg1, arg2, arg3)
  | (None, None) => 0L
  };
};
Callback.register("objc_meth_callback3", callback3);

let blockCallbacks3: Hashtbl.t(int64, (int64, int64, int64) => return) =
  Hashtbl.create(100);
let blockCallbacks0: Hashtbl.t(int64, unit => return) = Hashtbl.create(100);

let blockCallback3 =
    (blockPointer: int64, arg1: int64, arg2: int64, arg3: int64) => {
  log("Running blockcallback!");
  switch (Hashtbl.find_opt(blockCallbacks3, blockPointer)) {
  | Some(fn) => fn(arg1, arg2, arg3)
  | None =>
    log(
      "ERROR: block callback not found in 0!: "
      ++ Int64.to_string(blockPointer),
    );
    Hashtbl.iter(
      (key, _) => log("Found " ++ Int64.to_string(key)),
      blockCallbacks3,
    );
    0L;
  };
};
Callback.register("objc_block_callback3", blockCallback3);

let blockCallback0 = (blockPointer: int64) => {
  switch (Hashtbl.find_opt(blockCallbacks0, blockPointer)) {
  | Some(fn) => fn()
  | None =>
    log(
      "ERROR: block callback not found in 3!: "
      ++ Int64.to_string(blockPointer),
    );
    Hashtbl.iter(
      (key, _) => log("Found " ++ Int64.to_string(key)),
      blockCallbacks3,
    );
    0L;
  };
};
Callback.register("objc_block_callback0", blockCallback0);

type returnType =
  | Void
  | Object
  | Bool
  | Int;

let returnTypeToInt =
  fun
  | Void => 0
  | Object => 1
  | Bool => 2
  | Int => 3;

external addMethod_: (int64, int64, 'a, 'b) => unit =
  "caml_objc_class_addMethod";

let addMethod = (selector, ~returnType, cls) => {
  let arity = ref(0);
  String.iter(
    fun
    | ':' => arity := arity^ + 1
    | _ => (),
    selector,
  );
  log(
    "Adding method " ++ selector ++ " with arity " ++ string_of_int(arity^),
  );
  addMethod_(cls, sel(selector), arity^, returnTypeToInt(returnType));
};

external createBlock3_: unit => int64 = "caml_objc_createBlock3";
external createBlock0_: unit => int64 = "caml_objc_createBlock0";

let createBlock3 = fn => {
  let blockPtr = createBlock3_();
  log("Block pointer is" ++ Int64.to_string(blockPtr));
  Hashtbl.replace(blockCallbacks3, blockPtr, fn);
  blockPtr;
};

let createBlock0 = fn => {
  let blockPtr = createBlock0_();
  Hashtbl.replace(blockCallbacks0, blockPtr, fn);
  blockPtr;
};

let addCallback = (selector, ~fn, instance) => {
  Hashtbl.replace(methods, (instance, selector), fn);
};

let addCallback1 = (selector, ~fn, instance) => {
  Hashtbl.replace(methods1, (instance, selector), fn);
};
let addCallback2 = (selector, ~fn, instance) => {
  Hashtbl.replace(methods2, (instance, selector), fn);
};
let addCallback3 = (selector, ~fn, instance) => {
  Hashtbl.replace(methods3, (instance, selector), fn);
};

let removeCallback = (obj, sel) => {
  Hashtbl.remove(methods, (obj, sel));
};
let removeCallback1 = (obj, sel) => {
  Hashtbl.remove(methods1, (obj, sel));
};
let removeCallback2 = (obj, sel) => {
  Hashtbl.remove(methods2, (obj, sel));
};
let removeCallback3 = (obj, sel) => {
  Hashtbl.remove(methods3, (obj, sel));
};

module NSString = {
  let cls = getClass("NSString");
  let make = str => {
    log("Creating NSString with string: " ++ str);
    cls |> msgSend1(~sel="stringWithUTF8String:", ~arg=str);
  };
};

module UILabel = {
  let cls = getClass("UILabel");
};

module UIColor = {
  let cls = getClass("UIColor");

  let redColor = () => cls |> send0(sel("redColor"));
  let whiteColor = () => cls |> send0(sel("whiteColor"));
};

external fromCString: ([@unboxed] int64, [@unboxed] int64) => string =
  "caml_from_c_string" "caml_from_c_string";

external runOnMainThread: int64 => unit =
  "caml_objc_run_on_main" "caml_objc_run_on_main";

module NSData = {
  external toString: ([@unboxed] int64) => string =
    "caml_nsdata_to_string" "caml_nsdata_to_string";
};
let alloc = NSSelector.alloc;
let init = NSSelector.init;
