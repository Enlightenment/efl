#!/usr/bin/env node

// Preamble
function assert(condition, message) {
  if (!condition) {
      print("Assertion failed ", message);
      throw new Error(message || "Assertion failed");
  }
}

if(typeof process !== 'undefined')
{
    console.log('running from nodejs');
    console.log('path', process.env.NODE_PATH);

    efl = require('efl');
    assert(efl != null, "could not load efl node module");

    // Setup output aliases
    print = console.log;
    print_error = function() {
        if (process.argv.indexOf("--supress-errors") == -1)
            console.error.apply(null, arguments);
    };
    print_info = function() {
        if (process.argv.indexOf("--verbose") != -1)
            console.info.apply(null, arguments);
    };
    exit = efl.Ecore.Mainloop.quit;
}
else
{
    assert = function(test, message) { if (test !== true) throw message; };
    print('running from libv8')
    //FIXME Add levels to v8 tests
    print_error = print
    print_info = print
    exit = function() {}
}

// Global flag for suite success //
suite_success = true;
// Global test summary
suite_ok = 0;
suite_fail = []; // Will store the name of the failures

// Basic test function //
function start_test(test_name, test_func) {
  print("[ RUN         ]  eina_js_suite: " + test_name);
  var test_result = true;
  try {
    test_func();
  } catch (e) {
    suite_success = false;
    test_result = false;
    print_error("Error: ", e, e.stack);
  }
  print("[        " + (test_result ? "PASS" : "FAIL") + " ]  eina_js_suite: " + test_name);
  if (test_result)
    suite_ok += 1;
  else
    suite_fail.push(test_name);
}
// end Preamble
if (typeof process != "undefined")
{
    benchmark = require('benchmark_object');
    assert(benchmark != null, "could not load benchmark_object.node");
    Benchmark_Object = benchmark.BenchmarkObject;
    benchmark_emptyarg = benchmark.benchmark_emptyarg;
    benchmark_onearg = benchmark.benchmark_onearg;
    benchmark_twoarg = benchmark.benchmark_twoarg;
    benchmark_tenarg = benchmark.benchmark_tenarg;
    benchmark_onecomplexarg = benchmark.benchmark_onecomplexarg;
    benchmark_tencomplexarg = benchmark.benchmark_tencomplexarg;
}
else
{
    Benchmark_Object = suite.Benchmark_Object;
}

start_test("empty function", function(){
    object = new Benchmark_Object(null);

    for(i = 0; i != 10; i++)
    {
        object.emptyarg();
    }
    
    time = process.hrtime();
    for(i = 0; i != 20000; i++)
    {
        object.emptyarg();
    }
    time = process.hrtime(time);
    console.log('JS empty function %d', time[0]*1000000000 + time[1]);
    benchmark_emptyarg();    
});

start_test("onearg function", function(){
    object = new Benchmark_Object(null);
    for(i = 0; i != 10; i++)
    {
        object.onearg(1);
    }

    time = process.hrtime();
    for(i = 0; i != 20000; i++)
    {
        object.onearg(1);
    }
    time = process.hrtime(time);
    console.log('JS onearg function %d', time[0]*1000000000 + time[1]);

    benchmark_onearg();    
});

start_test("twoarg function", function(){
    object = new Benchmark_Object(null);
    for(i = 0; i != 10; i++)
    {
        object.twoarg(1, 2);
    }

    time = process.hrtime();
    for(i = 0; i != 20000; i++)
    {
        object.twoarg(1, 2);
    }
    time = process.hrtime(time);
    console.log('JS twoarg function %d', time[0]*1000000000 + time[1]);

    benchmark_twoarg();    
});

start_test("tenarg function", function(){
    object = new Benchmark_Object(null);
    for(i = 0; i != 10; i++)
    {
        object.tenarg(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    }

    time = process.hrtime();
    for(i = 0; i != 20000; i++)
    {
        object.tenarg(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    }
    time = process.hrtime(time);
    console.log('JS tenarg function %d', time[0]*1000000000 + time[1]);
    benchmark_tenarg();    
});

start_test("onecomplexarg function", function(){
    object = new Benchmark_Object(null);
    list = new efl.List('int')
    for(i = 0; i != 10; i++)
    {
        object.onecomplexarg(list);
    }

    time = process.hrtime();
    for(i = 0; i != 20000; i++)
    {
        object.onecomplexarg(list);
    }
    time = process.hrtime(time);
    console.log('JS onecomplexarg function %d', time[0]*1000000000 + time[1]);
    benchmark_onecomplexarg();    
});

start_test("tencomplexarg function", function(){
    object = new Benchmark_Object(null);
    list = new efl.List('int')
    for(i = 0; i != 10; i++)
    {
        object.tencomplexarg(list, list, list, list, list, list, list, list, list, list);
    }

    time = process.hrtime();
    for(i = 0; i != 20000; i++)
    {
        object.tencomplexarg(list, list, list, list, list, list, list, list, list, list);
    }
    time = process.hrtime(time);
    console.log('JS tencomplexarg function %d', time[0]*1000000000 + time[1]);
    benchmark_tencomplexarg();    
});

// footer

if (!suite_success) {
  print ("[ Total tests run: %s ]", suite_ok + suite_fail.length);
  print ("[ Total successful: %s ]", suite_ok);
  print ("[ Total failures: %s ]", suite_fail.length);
  print ("[ Tests failed: ]");
  for (var i = 0; i < suite_fail.length; i++) {
    print ("[    %s]", suite_fail[i]);
  };
  assert(false, "[ Test suite fail ]");
} else {
  print ("[ Test execution with success ]");
  print ("[ Total tests run: %s ]", suite_ok);
}

exit();
