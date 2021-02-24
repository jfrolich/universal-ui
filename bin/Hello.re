open Objc;
open UIKit;
open NativeComponents;

type navScreen =
  (~key: React.Key.t=?, ~frame: CGRect.t, ~push: value(obj) => unit, unit) =>
  React.element(value(obj));

[%graphql
  {|
  query LaunchesQuery {
    launchesPast(limit: 10) {
      mission_name
    }
  }
|}
];

type action =
  | Increment
  | Decrement;

let reducer = (action, state) =>
  switch (action) {
  | Increment => state + 1
  | Decrement => state - 1
  };

let createDetailVC = component => {
  let vc = BasicViewController.cls |> send0(alloc);
  log("Creating Detail VC2, instance: " ++ NSObject.toString(vc));

  let unsubscribe = ref(None);

  vc
  |> addCallback(
       sel("viewDidLoad"),
       ~fn=(obj: NSObject.t('a)) => {
         let _ = obj |> msgSendSuper(~sel="viewDidLoad");
         msgSend1(obj, ~sel="setTitle:", ~arg=NSString.make("Counter"))
         |> ignore;

         let view = msgSend(obj, ~sel="view");

         let _ =
           view
           |> msgSend1(~sel="setBackgroundColor:", ~arg=UIColor.whiteColor());

         let unsubscribe_: unit => unit =
           React.createRoot(view, <component />);

         unsubscribe := Some(unsubscribe_);
         returnUnit();
       },
     );

  vc
  |> addCallback(sel("dealloc"), ~fn=_ => {
       switch (unsubscribe.contents) {
       | None => returnUnit()
       | Some(unsubscribe) =>
         unsubscribe();
         returnUnit();
       }
     });

  vc |> send0(init);
};

let%component counter = () => {
  let%hook (state, dispatch) = React.Hooks.reducer(~initialState=0, reducer);
  <>
    <text
      text={"Count = " ++ string_of_int(state)}
      frame={CGRect.make(115., 150., 200., 30.)}
    />
    <button
      text="Decrement"
      frame={CGRect.make(110.0, 200.0, 100.0, 30.0)}
      onPress={() => dispatch(Decrement)}
    />
    <button
      text="Increment"
      frame={CGRect.make(110.0, 300.0, 100.0, 30.0)}
      onPress={() => dispatch(Increment)}
    />
  </>
  |> React.listToElement;
};
let%component firstScreen = (~frame, ~push, ()) => {
  let%hook (titles, setTitles) = React.Hooks.state(["loollll"]);
  let%hook () =
    React.Hooks.effect(
      React.Hooks.Effect.OnMount,
      () => {
        log("MOUNTING FIRST SCREEN");
        GraphQL.fetch(
          "https://api.spacex.land/graphql",
          LaunchesQuery.query,
          result => {
            // log("RESULT");
            // log(result);
            let json = Yojson.Basic.from_string(result);
            let data =
              switch (json) {
              | `Assoc(fields) =>
                switch (
                  fields
                  |> List.filter(((string, _json)) => string == "data")
                ) {
                | [(_, data)] => Some(data)
                | _ => None
                }

              | _ => None
              };

            switch (data) {
            | Some(data) =>
              let titles =
                switch (
                  LaunchesQuery.parse(LaunchesQuery.unsafe_fromJson(data))
                ) {
                | {launchesPast: Some(launchesPast)} =>
                  launchesPast
                  |> Array.to_list
                  |> List.filter_map(a => a)
                  |> List.map((a: LaunchesQuery.t_launchesPast) =>
                       a.mission_name
                     )
                  |> List.filter_map(a => a)
                | _ => []
                };

              setTitles(_ => titles);
            | None => ()
            };
          },
        );
        None;
      },
    );

  <tableView
    frame
    titles={titles |> Array.of_list}
    onPress={(_section, _row) => {push(createDetailVC(counter))}}
  />;
};

let createRootVC = (~push, component: navScreen) => {
  log("Creating RootVC 2");
  let vc = BasicViewController.cls |> send0(alloc);
  let unsubscribeRootVC = ref(None);

  vc
  |> addCallback(
       sel("viewDidLoad"),
       ~fn=obj => {
         log("Root viewDidLoad");
         let _ = obj |> msgSendSuper(~sel="viewDidLoad");
         let _ =
           obj
           |> msgSend1(
                ~sel="setTitle:",
                ~arg=NSString.make("SpaceX Launches"),
              );

         let view = msgSend(obj, ~sel="view");
         let bounds = CGRect.alloc();
         let bounds = view |> msgSend_stret(~ret=bounds, ~sel="bounds");
         let viewFrame = bounds;

         let unsubscribe_: unit => unit =
           React.createRoot(
             view,
             <component frame=viewFrame push key={React.Key.create()} />,
           );

         unsubscribeRootVC := Some(unsubscribe_);

         returnUnit();
       },
     );

  vc
  |> addCallback(sel("dealloc"), ~fn=_ => {
       switch (unsubscribeRootVC.contents) {
       | None => returnUnit()
       | Some(unsubscribe) =>
         unsubscribe();
         returnUnit();
       }
     });

  vc |> send0(init);
};

module SceneDelegate = {
  let cls =
    allocateClassPair(
      getClass("UIResponder"),
      ~name="SceneDelegate",
      ~size=0,
    );

  let _ = NSClass.addProtocol(cls, NSProtocol.get("UIWindowSceneDelegate"));

  cls |> addMethod("scene:willConnectToSession:options:", ~returnType=Void);
  cls
  |> addCallback3(
       sel("scene:willConnectToSession:options:"),
       ~fn=(_self, scene, _session, _connectionOptions) => {
         log("SCENE!!");
         let window =
           getClass("UIWindow")
           |> send0(alloc)
           |> msgSend1_str(
                ~sel="initWithFrame:",
                ~arg={
                  let bounds = CGRect.alloc();
                  let _ =
                    getClass("UIScreen")
                    |> send0(sel("mainScreen"))
                    |> msgSend_stret(~ret=bounds, ~sel="bounds");

                  bounds;
                },
              );

         let _ = window |> msgSend1(~sel="setWindowScene:", ~arg=scene);

         let navigationViewController =
           classToObj("UINavigationController") |> send0(alloc);

         let push = pushVC => {
           let _ =
             navigationViewController
             |> send2(
                  sel("pushViewController:animated:"),
                  pushVC,
                  NSObject.unsafe_fromInt64(1L),
                );
           ();
         };

         let _ =
           navigationViewController
           |> msgSend1(
                ~sel="initWithRootViewController:",
                ~arg=createRootVC(~push, firstScreen),
              );

         let _ =
           navigationViewController
           |> send0(sel("navigationBar"))
           |> msgSend1(~sel="setPrefersLargeTitles:", ~arg=NSBool.true_);

         let _ =
           window
           |> msgSend1(
                ~sel="setRootViewController:",
                ~arg=navigationViewController,
              );

         let _ =
           window
           |> msgSend1(~sel="setBackgroundColor:", ~arg=UIColor.whiteColor());

         let _ = window |> msgSend(~sel="makeKeyAndVisible");

         returnUnit();
       },
     );

  let _ = registerClassPair(cls);
};

module AppDelegate = {
  let cls =
    allocateClassPair(getClass("UIResponder"), ~name="AppDelegate", ~size=0);

  let _ = NSClass.addProtocol(cls, NSProtocol.get("UIApplicationDelegate"));

  cls
  |> addMethod("application:didFinishLaunchingWithOptions:", ~returnType=Bool);

  cls
  |> addMethod(
       "application:configurationForConnectingSceneSession:options:",
       ~returnType=Void,
     );

  let _ = registerClassPair(cls);

  cls
  |> addCallback2(
       sel("application:didFinishLaunchingWithOptions:"),
       ~fn=(_self, _application, _launchOptions) => {
       returnBool(true)
     });

  cls
  |> addCallback3(
       sel("application:configurationForConnectingSceneSession:options:"),
       ~fn=(_self, _application, connectingSceneSession, _options) => {
         let _ =
           classToObj("UISceneConfiguration")
           |> msgSend(~sel="alloc")
           |> msgSend2(
                ~sel="initWithName:sessionRole:",
                ~arg1=NSString.make("Default Configuration"),
                ~arg2=msgSend(connectingSceneSession, ~sel="role"),
              );
         returnUnit();
       },
     );
};
