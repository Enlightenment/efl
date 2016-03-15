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
    console.log("teste1");

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
  print("[ RUN         ]  eldbus_js_suite: " + test_name);
  var test_result = true;
  try {
    test_func();
  } catch (e) {
    suite_success = false;
    test_result = false;
    print_error("Error: ", e, e.stack);
  }
  print("[        " + (test_result ? "PASS" : "FAIL") + " ]  eldbus_js_suite: " + test_name);
  if (test_result)
    suite_ok += 1;
  else
    suite_fail.push(test_name);
}
// end Preamble

// Eldbus core tests
start_test('test constants', function () {
    assert(typeof(efl.ELDBUS_FDO_BUS) === 'string');
    assert(typeof(efl.ELDBUS_FDO_PATH) === 'string');
    assert(typeof(efl.ELDBUS_FDO_INTERFACE) === 'string');
    assert(typeof(efl.ELDBUS_FDO_INTERFACE_PROPERTIES) === 'string');
    assert(typeof(efl.ELDBUS_FDO_INTERFACE_INTROSPECTABLE) === 'string');
    assert(typeof(efl.ELDBUS_FDO_INTEFACE_PEER) === 'string');
    assert(typeof(efl.ELDBUS_ERROR_PENDING_CANCELED) === 'string');
    assert(typeof(efl.ELDBUS_ERROR_PENDING_TIMEOUT) === 'string');
});

// Eldbus connection tests
start_test('test connection constants', function () {
    assert(typeof(efl.ELDBUS_TIMEOUT_INFINITE) === 'number');
    assert(typeof(efl.ELDBUS_CONNECTION_TYPE_UNKNOWN) === 'number');
    assert(typeof(efl.ELDBUS_CONNECTION_TYPE_SESSION) === 'number');
    assert(typeof(efl.ELDBUS_CONNECTION_TYPE_SYSTEM) === 'number');
    assert(typeof(efl.ELDBUS_CONNECTION_TYPE_STARTER) === 'number');
    assert(typeof(efl.ELDBUS_CONNECTION_TYPE_ADDRESS) === 'number');
    assert(typeof(efl.ELDBUS_CONNECTION_TYPE_LAST) === 'number');
    assert(typeof(efl.ELDBUS_CONNECTION_EVENT_DEL) === 'number');
    assert(typeof(efl.ELDBUS_CONNECTION_EVENT_DISCONNECTED) === 'number');
    assert(typeof(efl.ELDBUS_CONNECTION_EVENT_LAST) === 'number');
});
// Eldbus connection

// TODO...

// finished tests

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
