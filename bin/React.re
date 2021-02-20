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

addStaleTreeHandler(() => RemoteAction.send(~action=(), onStale));

let createRoot = (view, element) => {
  log("---------------->>>> creating root");
  let rendered =
    ref(
      RenderedElement.render(
        {node: view, insertNode, deleteNode, moveNode},
        element,
      ),
    );

  let _ = RenderedElement.executeHostViewUpdates(rendered^);
  let _ = RenderedElement.executePendingEffects(rendered^);

  let unsubscribe =
    RemoteAction.subscribe(
      ~handler=
        () => {
          log("---------------->>>> rerendering root");
          let nextElement = RenderedElement.flushPendingUpdates(rendered^);
          RenderedElement.executeHostViewUpdates(nextElement) |> ignore;
          rendered := nextElement;
        },
      onStale,
    );

  unsubscribe;
};
