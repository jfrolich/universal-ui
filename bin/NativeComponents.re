open Objc;

let%nativeComponent text = (~text, ~frame: CGRect.t, (), hooks) => (
  {
    make: () => {
      let label = UILabel.cls |> send0(alloc) |> send0(init);

      log("--> Make Label " ++ NSObject.toString(label));
      label;
    },
    configureInstance: (~isFirstRender, node) => {
      if (isFirstRender) {
        log("--> First Render of Label " ++ NSObject.toString(node));
      } else {
        log("--> Update of Label" ++ NSObject.toString(node));
      };
      let labelStr = NSString.make(text);
      let _ = node |> msgSend1(~sel="setText:", ~arg=labelStr);
      let _ = node |> msgSend1_str(~sel="setFrame:", ~arg=frame);
      node;
    },
    children: React.empty,
    insertNode: React.insertNode,
    deleteNode: React.insertNode,
    moveNode: React.moveNode,
  },
  hooks,
);

module ClickDelegate = {
  let cls =
    NSObject.cls |> allocateClassPair(~name="REClickDelegate", ~size=0);

  cls |> addMethod("buttonPressed", ~returnType=Void);
  let _ = registerClassPair(cls);
};

let yellowColor = classToObj("UIColor") |> msgSend(~sel="yellowColor");
let%nativeComponent button =
                    (~text, ~frame: CGRect.t, ~onPress: unit => unit, ()) => {
  let%hook clickDelegateInstance = React.Hooks.ref(nil);

  {
    React.make: () => {
      let button =
        getClass("UIButton") |> msgSend1(~sel="buttonWithType:", ~arg=1);
      log("--> Make Button" ++ NSObject.toString(button));

      clickDelegateInstance :=
        ClickDelegate.cls |> msgSend(~sel="alloc") |> msgSend(~sel="init");

      let _ =
        msgSend3(
          button,
          ~sel="addTarget:action:forControlEvents:",
          ~arg1=clickDelegateInstance^,
          ~arg2=sel("buttonPressed"),
          ~arg3=64,
        );

      button;
    },
    configureInstance: (~isFirstRender, button) => {
      if (isFirstRender) {
        log("--> First Render of button " ++ NSObject.toString(button));
      } else {
        log("--> Update of button " ++ NSObject.toString(button));
      };
      clickDelegateInstance^
      |> addCallback(
           sel("buttonPressed"),
           ~fn=_ => {
             onPress();
             returnUnit();
           },
         );

      let buttonStr = NSString.make(text);
      let _ =
        msgSend2(button, ~sel="setTitle:forState:", ~arg1=buttonStr, ~arg2=0);

      let _ = buttonStr |> msgSend(~sel="release");
      let _ = msgSend1_str(button, ~sel="setFrame:", ~arg=frame);
      let _ = msgSend1(button, ~sel="setBackgroundColor:", ~arg=yellowColor);

      button;
    },
    children: React.empty,
    insertNode: React.insertNode,
    deleteNode: (~parent, ~child, ~position) => {
      removeCallback(clickDelegateInstance^, sel("buttonPressed"));
      let _ = msgSend(clickDelegateInstance^, ~sel="release");
      React.deleteNode(~parent, ~child, ~position);
    },
    moveNode: React.moveNode,
  };
};

module TableViewDataSource = {
  let cls =
    getClass("NSObject")
    |> allocateClassPair(~name="ReTableViewDataSource", ~size=0);
  let _ = NSClass.addProtocol(cls, NSProtocol.get("UITableViewDataSource"));

  cls |> addMethod("tableView:cellForRowAtIndexPath:", ~returnType=Object);
  cls |> addMethod("tableView:numberOfRowsInSection:", ~returnType=Int);
  cls |> addMethod("numberOfSectionsInTableView:", ~returnType=Int);
  let _ = registerClassPair(cls);
};

module TableViewDelegate = {
  let cls =
    getClass("NSObject")
    |> allocateClassPair(~name="ReTableViewDelegate", ~size=0);
  let _ = NSClass.addProtocol(cls, NSProtocol.get("UITableViewDelegate"));
  let _ = registerClassPair(cls);
  cls |> addMethod("tableView:didSelectRowAtIndexPath:", ~returnType=Void);
};

let%nativeComponent tableView =
                    (
                      ~frame: CGRect.t,
                      ~onPress: (int64, int64) => unit,
                      ~titles,
                      (),
                    ) => {
  let%hook tableViewDelegate = React.Hooks.ref(Objc.nil);
  let%hook tableViewDataSource = React.Hooks.ref(Objc.nil);
  let%hook storedTitles = React.Hooks.ref(titles);

  {
    React.make: () => {
      log("->>>>>MAKING TABLEVIEW");
      let view = classToObj("UITableView") |> msgSend(~sel="alloc");

      tableViewDelegate :=
        TableViewDelegate.cls |> send0(alloc) |> send0(init);
      tableViewDataSource :=
        TableViewDataSource.cls |> send0(alloc) |> send0(init);

      let _ = msgSend1(view, ~sel="setDelegate:", ~arg=tableViewDelegate^);
      let _ =
        msgSend1(view, ~sel="setDataSource:", ~arg=tableViewDataSource^);
      let _ =
        msgSend2_str(
          view,
          ~sel="initWithFrame:style:",
          ~arg1=frame,
          ~arg2=Int64.zero,
        );

      view;
    },
    configureInstance: (~isFirstRender, view) => {
      log("->>>>>CONFIGURING TABLEVIEW");
      storedTitles.contents = titles;
      if (isFirstRender) {
        log("--> First Render of TableView " ++ NSObject.toString(view));
      } else {
        log("--> Update of TableView " ++ NSObject.toString(view));
      };

      log(
        "Titles: "
        ++ (
          titles
          |> Array.to_list
          |> List.fold_left((acc, title) => acc ++ title, "")
        ),
      );

      if (isFirstRender) {
        tableViewDelegate^
        |> addCallback2(
             sel("tableView:didSelectRowAtIndexPath:"),
             ~fn=(_, _, indexPath) => {
               onPress(
                 indexPath |> send0(sel("section")) |> NSObject.toInt64,
                 indexPath |> send0(sel("row")) |> NSObject.toInt64,
               );

               returnUnit();
             },
           );

        tableViewDataSource^
        |> addCallback2(
             sel("tableView:numberOfRowsInSection:"),
             ~fn=(_obj, _tableView, _section) =>
             returnInt(Array.length(storedTitles.contents))
           );

        tableViewDataSource^
        |> addCallback1(
             sel("numberOfSectionsInTableView:"), ~fn=(_obj, _tableView) => {
             returnInt(1)
           });

        tableViewDataSource^
        |> addCallback2(
             sel("tableView:cellForRowAtIndexPath:"),
             ~fn=(_obj, tableView, indexPath) => {
               let cellId = NSString.make("haha");
               let cell =
                 tableView
                 |> msgSend1(
                      ~sel="dequeueReusableCellWithIdentifier:",
                      ~arg=cellId,
                    );

               let cell =
                 if (cell == nil) {
                   classToObj("UITableViewCell")
                   |> send0(alloc)
                   |> msgSend2(
                        ~sel="initWithStyle:reuseIdentifier:",
                        ~arg1=0L,
                        ~arg2=cellId,
                      );
                 } else {
                   cell;
                 };

               let contentConfiguration =
                 cell |> send0(sel("defaultContentConfiguration"));

               let title =
                 NSString.make(
                   storedTitles.contents[msgSend(indexPath, ~sel="row")
                                         |> NSObject.toInt64
                                         |> Int64.to_int],
                 );

               let _ =
                 contentConfiguration |> msgSend1(~sel="setText:", ~arg=title);

               let _ =
                 cell
                 |> msgSend1(
                      ~sel="setContentConfiguration:",
                      ~arg=contentConfiguration,
                    );

               returnObj(cell);
             },
           );

        let _ = msgSend1_str(view, ~sel="setFrame:", ~arg=frame);
        ();
      };
      let _ = send0(sel("reloadData"), view);

      view;
    },
    children: React.empty,
    insertNode: React.insertNode,
    deleteNode: (~parent, ~child, ~position) => {
      removeCallback2(
        tableViewDelegate^,
        sel("tableView:didSelectRowAtIndexPath:"),
      );
      let _ = msgSend(tableViewDelegate^, ~sel="release");
      let _ = msgSend(tableViewDataSource^, ~sel="release");
      React.deleteNode(~parent, ~child, ~position);
    },
    moveNode: React.moveNode,
  };
};
