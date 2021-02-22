open Objc;

include Brisk_reconciler;
let insertNode = (~parent, ~child, ~position as _) => {
  let _ = msgSend1(parent, ~sel="addSubview:", ~arg=child);
  log("--> INSERT NODE: " ++ NSObject.toString(child));
  parent;
};

let deleteNode = (~parent, ~child, ~position as _) => {
  let _ = msgSend(child, ~sel="removeFromSuperview");
  log("-->DELETE NODE: " ++ NSObject.toString(child));
  parent;
};

let moveNode = (~parent, ~child as _, ~from as _, ~to_ as _) => {
  parent;
};

let onStale = RemoteAction.create();

addStaleTreeHandler(() => onStale |> RemoteAction.send(~action=()));

let executeUpdates = element => {
  // execute the updates on the main thread
  let makeUpdates = () => {
    let _ = RenderedElement.executeHostViewUpdates(element);
    let _ = RenderedElement.executePendingEffects(element);
    returnUnit();
  };
  // if (getClass("NSThread") |> send0(sel("isMainThread")) != nil) {
  //   let _ = makeUpdates();
  //   ();
  // } else {
  makeUpdates |> createBlock0 |> runOnMainThread;
  // };
};

let createRoot = (view, element) => {
  log("---------------->>>> creating root");
  let rendered =
    ref(
      RenderedElement.render(
        {node: view, insertNode, deleteNode, moveNode},
        element,
      ),
    );

  executeUpdates(rendered^);

  let unsubscribe =
    onStale
    |> RemoteAction.subscribe(~handler=() => {
         log("---------------->>>> rerendering root");
         let nextElement = RenderedElement.flushPendingUpdates(rendered^);
         executeUpdates(nextElement);
         rendered := nextElement;
       });

  unsubscribe;
};
