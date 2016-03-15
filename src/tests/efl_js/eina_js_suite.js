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

function array_equal(a, b) {
  if (a === b) return true;
  if (a == null || b == null) return false;
  if (a.length != b.length) return false;

  for (var i = 0; i < a.length; ++i) {
    if (a[i] !== b[i]
        && (Object.prototype.toString.call(a[i]) === '[object Array]'
             && Object.prototype.toString.call(b[i] === '[object Array]')
             && !array_equal(a[i], b[i])))
      return false;
  }
  return true;
}
// end Preamble

// eina Preamble
// Functions to get the line in NodeJS and V8
Object.defineProperty(global, '__stack', {
get: function() {
        var orig = Error.prepareStackTrace;
        Error.prepareStackTrace = function(_, stack) {
            return stack;
        };
        var err = new Error;
        Error.captureStackTrace(err, arguments.callee);
        var stack = err.stack;
        Error.prepareStackTrace = orig;
        return stack;
    }
});

Object.defineProperty(global, '__line', {
get: function() {
        return __stack[1].getLineNumber();
    }
});

Object.defineProperty(global, '__function', {
get: function() {
        return __stack[1].getFunctionName();
    }
});
// end eina Preamble

// value tests
start_test("value tests int get test", function () {
    print_info("x");

    var my_value = new efl.value(1);
    print_info("x");
    var wrapped = my_value.get();
    print_info("x");
    assert(typeof(wrapped) === 'number', 'typeof wrapped is not number');
    print_info("x");
    assert(wrapped === 1, 'wrapped should be 1');
    print_info("x");
});

start_test("value tests int set test", function(){
    var my_value = new efl.value(1);
    var wrapped = my_value.get();
    my_value.set(2);
    print_info("x");
    assert(wrapped === 1, 'Setting a value should not change a previous wrapped result');
    print_info("x");
    wrapped = my_value.get();
    print_info("x");
    assert(typeof(wrapped) === 'number', 'wrapped value after setting to number should be a number');
    print_info("x");
    assert(wrapped === 2, 'wrapped is different from value set');
    print_info("x");
});

start_test("value tests int set to bool", function(){
    var my_value = new efl.value(2);
    var wrapped = my_value.get();
    my_value.set(true);
    print_info("x");
    assert(wrapped === 2, 'Setting a value should not change a previous wrapped result');
    print_info("x");
    wrapped = my_value.get();
    print_info("x");
    // boolean is represented as integer in the efl::eina::value layer
    print_info("x");
    assert(typeof(wrapped) === 'number', 'value.get from bool value should be a number');
    print_info("x");
    assert(wrapped === 1, 'value.get from a true boolean should be 1');
    print_info("x");
});

start_test("value tests set value to complex object", function() {
    var my_value = new efl.value(1);
    print_info("x");
    var captured = false;
    print_info("x");
    try {
        print_info("x");
        my_value.set({type: 'complex object'});
        print_info("x");
    } catch(e) {
        print_info("x");
        assert(e.code === 'std::bad_cast', 'Trying to set a complex object to a value should raise std::bad_cast');
        print_info("x");
        captured = true;
        print_info("x");
    }
    print_info("x");
    assert(captured === true, 'Trying to set a complex object should raise an exception');
    print_info("x");
});

start_test("value tests create value from complex object", function() {
    print_info("x");
    var captured = false;
    print_info("x");
    try {
        print_info("x");
        my_value = new efl.value({type: 'complex object'});
        print_info("x");
    } catch(e) {
        print_info("e.code ", e.code, ' ', typeof e);
        assert(e.code === 'std::bad_cast', 'Trying to create a value from a complex object should raise std::bad_cast');
        print_info("x");
        captured = true;
        print_info("x");
    }
    assert(captured === true, 'Trying to create a value from a complex object should raise an exception');
});
// log

start_test("log tests basic", function () {
    function f1(){ efl.logPrint(efl.LOG_DOMAIN_GLOBAL, efl.LOG_LEVEL_DBG, 'I changed again'); }; f1();
    efl.logPrint(efl.LOG_DOMAIN_GLOBAL, efl.LOG_LEVEL_CRITICAL, 'Cool to Hate');
    function f2(){ efl.logPrint(efl.LOG_DOMAIN_GLOBAL, efl.LOG_LEVEL_WARN, 'One Fine Day'); }; f2();
    function f3(){ efl.logPrint(efl.mydomain, efl.LOG_LEVEL_INFO, 'Never Gonna Find Me'); }; f3();
});

start_test("log tests domains", function() {
    var mydomain2 = efl.registerLogDomain('mydomain2', '');
    efl.setLogDomainRegisteredLevel(mydomain2, efl.LOG_LEVEL_DBG);
    assert(efl.getLogDomainRegisteredLevel(mydomain2) === efl.LOG_LEVEL_DBG, 'Level set is not LEVEL_DBG');
    efl.logPrint(mydomain2, efl.LOG_LEVEL_ERR, "The Kids Aren't Alright");

    efl.unregisterLogDomain(mydomain2);
});

start_test("log tests properties", function() {
    efl.setLogColorDisable(true);
    assert(efl.getLogColorDisable() === true, 'getLogColorDisable is false after setting to true');
    efl.setLogColorDisable(false);
    assert(efl.getLogColorDisable() === false, 'getLogColorDisable is true after setting to false');

    efl.setLogFileDisable(true);
    assert(efl.getLogFileDisable() === true, 'getLogFileDisable is false after setting to true');
    efl.setLogFileDisable(false);
    assert(efl.getLogFileDisable() === false, 'getLogFileDisable is true after setting to false');

    efl.setLogFunctionDisable(true);
    assert(efl.getLogFunctionDisable() === true, 'getLogFunctionDisable is false after setting to true');
    efl.setLogFunctionDisable(false);
    assert(efl.getLogFunctionDisable() === false, 'getLogFunctionDisable is true after setting to false');

    efl.setLogAbortOnCritical(true);
    assert(efl.getLogAbortOnCritical() === true, 'getLogAbortOnCritical is false after setting to true');
    efl.setLogAbortOnCritical(false);
    assert(efl.getLogAbortOnCritical() === false, 'getLogAbortOnCritical is true after setting to false');

});

start_test("log tests print callback", function() {
    var entered = false;
    var line_number = 0;

    // Setup
    var mydomain = efl.registerLogDomain('mydomain', 'deadbeef');
    efl.setLogDomainRegisteredLevel(mydomain, efl.LOG_LEVEL_WARN);

    efl.setLogPrintCb(function(domain, color, level, file, func, line, msg) {
        assert(domain === 'mydomain', 'log print callback incorrect domain');
        assert(color === 'deadbeef', 'log print callback incorrect color');
        assert(level === efl.LOG_LEVEL_WARN, 'log print callback incorrect level');
        assert(function(){ var components = file.split('/'); return components[components.length - 1]; }() === 'eina_js_suite.js'
               , 'log print callback incorrect file');
        assert(func === 'f4', 'log print callback incorrect func');
        assert(line === line_number, 'log print callback incorrect line');
        assert(msg === 'What Happened To You', 'log print callback incorrect message');
        entered = true;
    });

    function f4() {
      line_number = __line + 1;
      efl.logPrint(mydomain, efl.LOG_LEVEL_WARN, 'What Happened To You');
    };
    f4();
    assert(entered === true, 'log_print callback was not called');

});

start_test("log tests levels", function(){
    efl.setLogLevel(efl.LOG_LEVEL_DBG);
    assert(efl.getLogLevel() === efl.LOG_LEVEL_DBG, 'log level was not DBG');
    assert(efl.checkLogLevel(efl.LOG_LEVEL_INFO) === true, 'log level INFO was not accepted when DBG level set');
    efl.setLogLevel(efl.LOG_LEVEL_CRITICAL);
    assert(efl.getLogLevel() === efl.LOG_LEVEL_CRITICAL, 'log level was not CRITICAL');
    assert(efl.checkLogLevel(efl.LOG_LEVEL_INFO) === false, 'log level INFO was accepted when CRITICAL level set');

    assert(efl.getLogAbortOnCritical() === false, 'log abort on critical get should not be true');
    efl.setLogAbortOnCritical(true);
    assert(efl.getLogAbortOnCritical() === true, 'log abort on critical should be true');

    efl.setLogAbortOnCriticalLevel(efl.LOG_LEVEL_CRITICAL);
    assert(efl.getLogAbortOnCriticalLevel() == efl.LOG_LEVEL_CRITICAL, 'log critical trigger level is not CRITICAL');
    efl.setLogAbortOnCriticalLevel(efl.LOG_LEVEL_ERR);
    assert(efl.getLogAbortOnCriticalLevel() == efl.LOG_LEVEL_ERR, 'log critical trigger level is not ERR');

    efl.setLogDomainLevel('mydomain', efl.LOG_LEVEL_WARN);
    assert(efl.getLogDomainLevel('mydomain') === efl.LOG_LEVEL_WARN, 'log level for mydomain is not WARN');
    efl.setLogDomainLevel('mydomain', efl.LOG_LEVEL_INFO);
    assert(efl.getLogDomainLevel('mydomain') === efl.LOG_LEVEL_INFO, 'log level for mydomain is not INFO');

    assert(typeof(efl.LOG_STATE_START) === 'number', 'type of LOG_STATE_START is not number');
    assert(typeof(efl.LOG_STATE_STOP) === 'number', 'type of LOG_STATE_STOP is not number');
    assert(typeof(efl.logTiming) === 'function', 'type of log_timing is not function');
});

start_test("log helper functions", function(){
    var messages = [
        [efl.LOG_LEVEL_CRITICAL, "critical"],
        [efl.LOG_LEVEL_ERR, "error"],
        [efl.LOG_LEVEL_WARN, "warning"],
        [efl.LOG_LEVEL_INFO, "info"],
        [efl.LOG_LEVEL_DBG, "debug"]
    ];
    var caught_messages = [];

    efl.setLogLevel(efl.LOG_LEVEL_WARN);
    efl.setLogAbortOnCritical(false);

    efl.setLogPrintCb(function(domain, color, level, file, func, line, msg) {
        caught_messages.push([level, msg]);
    });

    efl.logCritical(messages[0][1]);
    efl.logError(messages[1][1]);
    efl.logWarning(messages[2][1]);
    efl.logInfo(messages[3][1]);
    efl.logDebug(messages[4][1]);

    assert(array_equal(messages.slice(0, 3), caught_messages), "Must received only the messages above the desired level.");
});

start_test("log helper wrong type", function(){
    var raised = false;
    try {
        efl.log_info(442);
    } catch (err) {
        raised = true;
        assert(err.name == "TypeError", "Raised exception must be TypeError.");
    }
    assert(raised, "Passing anything other than a string must raise TypeError.");
});

start_test("log color constants", function(){
    assert(typeof efl.COLOR_LIGHTRED == "string", "efl.COLOR_LIGHTRED must be a string.");
    assert(typeof efl.COLOR_RED == "string", "efl.COLOR_RED must be a string.");
    assert(typeof efl.COLOR_LIGHTBLUE == "string", "efl.COLOR_LIGHTBLUE must be a string.");
    assert(typeof efl.COLOR_BLUE == "string", "efl.COLOR_BLUE must be a string.");
    assert(typeof efl.COLOR_GREEN == "string", "efl.COLOR_GREEN must be a string.");
    assert(typeof efl.COLOR_YELLOW == "string", "efl.COLOR_YELLOW must be a string.");
    assert(typeof efl.COLOR_ORANGE == "string", "efl.COLOR_ORANGE must be a string.");
    assert(typeof efl.COLOR_WHITE == "string", "efl.COLOR_WHITE must be a string.");
    assert(typeof efl.COLOR_LIGHTCYAN == "string", "efl.COLOR_LIGHTCYAN must be a string.");
    assert(typeof efl.COLOR_CYAN == "string", "efl.COLOR_CYAN must be a string.");
    assert(typeof efl.COLOR_RESET == "string", "efl.COLOR_RESET must be a string.");
    assert(typeof efl.COLOR_HIGH == "string", "efl.COLOR_HIGH must be a string.");
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
