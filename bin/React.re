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
  log("---> MOVE NODE...");
  parent;
};

let onStale = RemoteAction.create();

addStaleTreeHandler(() => onStale |> RemoteAction.send(~action=()));

let executeUpdates = element => {
  let newElement = ref(element);
  // execute the updates on the main thread
  let makeUpdates = () => {
    log("Making updates......");
    let _ = RenderedElement.executeHostViewUpdates(element);
    newElement := RenderedElement.executePendingEffects(element);
    returnUnit();
  };

  if (getClass("NSThread") |> send0(sel("isMainThread")) != nil) {
    log("RUNNIGN ON MAIN THREAD");
    let _ = makeUpdates();
    ();
  } else {
    log("RUNNIGN ON OTHER THREAD");
    makeUpdates |> createBlock0 |> runOnMainThread;
  };
  newElement^;
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

  rendered := executeUpdates(rendered^);

  let unsubscribe =
    onStale
    |> RemoteAction.subscribe(~handler=() => {
         log("---------------->>>> rerendering root");
         let nextElement = RenderedElement.flushPendingUpdates(rendered^);
         let element = executeUpdates(nextElement);
         rendered := element;
       });

  unsubscribe;
};
