open Objc;

let fetch = (url, query, callback) => {
  let url =
    getClass("NSURL") |> send1(sel("URLWithString:"), NSString.make(url));

  log("FETCHING");
  let session = getClass("NSURLSession") |> send0(sel("sharedSession"));

  let request =
    getClass("NSMutableURLRequest")
    |> send0(alloc)
    |> send1(sel("initWithURL:"), url);

  let query =
    String.split_on_char('\n', query)
    |> List.fold_left(
         (aggr, str) =>
           switch (aggr) {
           | "" => str
           | aggr => aggr ++ "\\n" ++ str
           },
         "",
       );
  let requestStr = {|{"query": "|} ++ query ++ {|", "variables": null}|};
  // log("REquest body");
  // log(requestStr);

  let requestBody =
    getClass("NSData")
    |> msgSend2(
         ~sel="dataWithBytes:length:",
         ~arg1=requestStr,
         ~arg2=String.length(requestStr),
       );
  let _ = request |> send1(sel("setHTTPBody:"), requestBody);
  let _ = request |> send1(sel("setHTTPMethod:"), NSString.make("POST"));
  let _ =
    request
    |> send2(
         sel("addValue:forHTTPHeaderField:"),
         NSString.make("application/json"),
         NSString.make("Content-Type"),
       );

  let completionHandler =
    createSingleUseBlock3((data, _response, _error) => {
      // let description = data |> send0(sel("description"));
      // NSString *myString = [[NSString alloc] initWithData:myData encoding:NSUTF8StringEncoding];
      let string = NSData.toString(data);
      // getClass("NSString")
      // |> send0(alloc)
      // |> send2(
      //      sel("initWithData:encoding:"),
      //      data,
      //      /* NSUTF8StringEncoding */ Value.fromInt64(4L),
      //    );

      // logObj(string);
      callback(string);
      returnUnit();
    });

  let dataTask =
    session
    |> send2(
         sel("dataTaskWithRequest:completionHandler:"),
         request,
         completionHandler,
       );
  let _ = dataTask |> send0(sel("resume"));
  ();
};
