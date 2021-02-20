// let get_sync = url =>
//   Lwt_result.Syntax.(
//     Lwt_main.run(
//       {
//         log("Sending request...");
//         let* response = Piaf.Client.Oneshot.get(Uri.of_string(url));
//         if (Piaf.Status.is_successful(response.status)) {
//           Piaf.Body.to_string(response.body);
//         } else {
//           let message = Piaf.Status.to_string(response.status);
//           Lwt.return(Error(`Msg(message)));
//         };
//       },
//     )
//   );
// module Node = {
//   type context = unit;
//   let nullContext = ();
// };
// module LayoutTypes = LayoutTypes.Create(Node, Flex.FixedEncoding);
// module Style = {
//   type t = {
//     width: float,
//     height: float,
//     flexGrow: float,
//     flexBasis: float,
//     flexShrink: float,
//     flexDirection: LayoutTypes.flexDirection,
//     flexWrap: LayoutTypes.wrapType,
//     justifyContent: LayoutTypes.justify,
//     alignItems: LayoutTypes.align,
//     alignSelf: LayoutTypes.align,
//   };
// };
