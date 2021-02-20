open Objc;

let bench = (name, fn) => {
  log("------------ Running benchmark " ++ name);
  let startSeconds = Sys.time();

  let numIterations = 10000000;
  for (_ in 0 to numIterations - 1) {
    fn();
  };
  let endSeconds = Sys.time();
  // log(
  //   "--------------------- Total s: "
  //   ++ string_of_float(endSeconds -. startSeconds),
  // );
  log(
    "------------ Average ns: "
    ++ string_of_float(
         1000000000.
         *. ((endSeconds -. startSeconds) /. float_of_int(numIterations)),
       ),
  );
};

/// performance of msgSend comparison:
/// https://www.mikeash.com/pyblog/friday-qa-2016-04-15-performance-comparisons-of-common-operations-2016-edition.html
let runBenchMarks = () => {
  let selStr = "dummyMethod";
  let s = sel(selStr);
  let dummy =
    getClass("DummyClass") |> msgSend(~sel="alloc") |> msgSend(~sel="init");

  bench("unboxed, no return", () => {msgSend_unboxed_unit(dummy, ~sel=s)});

  bench("unboxed", () => {
    let _ = msgSend_unboxed(dummy, ~sel=s);
    ();
  });

  bench("unboxed sel string", () => {
    let s = sel(selStr);
    let _ = msgSend_unboxed(dummy, ~sel=s);
    ();
  });

  bench("unit  no alloc", () => {msgSend_unit_noalloc(dummy, ~sel=s)});

  bench("unit", () => {msgSend_unit(dummy, ~sel=s)});

  bench("Fixed selector", () => {
    let _ = msgSend_sel(dummy, ~sel=s);
    ();
  });

  bench("String selector", () => {
    let _ = msgSend(dummy, ~sel=selStr);
    ();
  });

  bench("String selector with ocaml annot", () => {
    let _ = msgSend_alt(dummy, ~sel=selStr);
    ();
  });

  bench("Get sel in ocaml", () => {
    let _ = msgSend_sel(dummy, ~sel=sel(selStr));
    ();
  });

  bench("fancy", () => {
    let _ = dummy |> send0(sel(selStr));
    ();
  });

  let _ = msgSend(dummy, ~sel="release");
  ();
};
