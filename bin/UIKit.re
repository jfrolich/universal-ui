open Objc;

module BasicViewController: {let cls: NSClass.t;} = {
  // global state for now
  let cls =
    getClass("UIViewController")
    |> allocateClassPair(~name="HelloRootViewController", ~size=0);

  cls |> addMethod("viewDidLoad", ~returnType=Void);
  cls |> addMethod("dealloc", ~returnType=Void);

  let _ = registerClassPair(cls);
};
