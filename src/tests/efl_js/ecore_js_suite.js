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
  print("[ RUN         ]  ecore_js_suite: " + test_name);
  var test_result = true;
  try {
    test_func();
  } catch (e) {
    suite_success = false;
    test_result = false;
    print_error("Error: ", e, e.stack);
  }
  print("[        " + (test_result ? "PASS" : "FAIL") + " ]  ecore_js_suite: " + test_name);
  if (test_result)
    suite_ok += 1;
  else
    suite_fail.push(test_name);
}
// end Preamble

// ecore preamble
function abs(n) {
    if (n < 0)
        return n * -1;
    return n;
}
var TOLERANCE = 0.0001;
// end ecore preamble

start_test("ecore timers", function () {
    var p = 2.5;
    efl.Ecore.Timer.setPrecision(p);
    assert(abs(efl.Ecore.Timer.getPrecision() - p) < TOLERANCE);
    
    p = 0.5;
    efl.Ecore.Timer.setPrecision(p);
    assert(abs(efl.Ecore.Timer.getPrecision() - p) < TOLERANCE);
    
    var ncalls = 0;

    captured = false;
    efl.Ecore.Timer.add(1, function() {
        ++ncalls;
        if (ncalls != 4)
            return true;
        
        captured = true;
        efl.Ecore.Job.add(efl.Ecore.Mainloop.quit);
        return false;
    });

    efl.Ecore.Mainloop.begin();
    assert(captured);

    ncalls = 0;
    captured = false;

    efl.Ecore.Timer.addLoop(1, function() {
        ++ncalls;
        if (ncalls != 4)
            return true;

        captured = true;
        efl.Ecore.Job.add(efl.Ecore.Mainloop.quit);
        return false;
    });

    efl.Ecore.Mainloop.begin();
    assert(captured);

    captured = false;

    var timer = efl.Ecore.Timer.add(1, function() {
        captured = true;
        return false;
    });

    assert(timer.freeze_get() === false);

    timer.freeze();

    assert(timer.freeze_get() === true);

    timer.thaw();

    assert(timer.freeze_get() === false);

    timer.del();

    efl.Ecore.Timer.add(2, function() {
        efl.Ecore.Job.add(efl.Ecore.Mainloop.quit);
        return false;
    });

    efl.Ecore.Mainloop.begin();
    assert(captured === false);
});

// Ecore event
start_test("ecore event", function () {
    var myevent = efl.Ecore.Event.newType();

    captured = [0, 0, 0]

    var handler1 = efl.Ecore.Event.addHandler(myevent, function(event) {
        print_info("CALLBACK_PASS_ON pre assert");
        assert(efl.Ecore.Event.getCurrentType() === myevent);
        assert(event === myevent);
        captured[0] += 1;
        print_info("CALLBACK_PASS_ON post assert");
        return efl.Ecore.Mainloop.CALLBACK_PASS_ON;
    });

    var handler2 = efl.Ecore.Event.addHandler(myevent, function(event) {
        print_info("CALLBACK_DONE #1 pre assert");
        assert(efl.Ecore.Event.getCurrentType() === myevent);
        assert(event === myevent);
        captured[1] += 1;
        print_info("CALLBACK_DONE #1 post assert");
        return efl.Ecore.Mainloop.CALLBACK_DONE;
    });

    var handler3 = efl.Ecore.Event.addHandler(myevent, function(event) {
        print_info("CALLBACK_DONE #2 pre assert");
        assert(efl.Ecore.Event.getCurrentType() === myevent);
        assert(event === myevent);
        captured[2] += 1;
        print_info("CALLBACK_DONE #2 post assert");
        return efl.Ecore.Mainloop.CALLBACK_DONE;
    });

    efl.Ecore.Timer.add(1, function() {
        efl.Ecore.Event.add(myevent);
        print_info("Timer.add(1, callback) before assert");
        assert(captured[0] === 0 && captured[1] === 0 && captured[2] === 0);
        print_info("Timer.add(1, callback) after assert");
        efl.Ecore.Timer.add(1, function() {
            print_info("inner Timer.add(1, callback) before assert");
            assert(captured[0] === 1 && captured[1] === 1 && captured[2] === 0);
            print_info("inner Timer.add(1, callback) after assert");
            handler1.del();
            efl.Ecore.Event.add(myevent);
            efl.Ecore.Event.add(myevent);
            efl.Ecore.Event.add(myevent).del();
            efl.Ecore.Timer.add(1, function() {
                print_info("twice inner Timer.add(1, callback) before assert");
                assert(captured[0] === 1 && captured[1] === 3 && captured[2] === 0);
                print_info("twice inner Timer.add(1, callback) after assert");
                efl.Ecore.Mainloop.quit();
            });
        });
    });

    efl.Ecore.Mainloop.begin();

    efl.Ecore.Event.add(myevent);
    efl.Ecore.Event.add(myevent);
    efl.Ecore.Event.add(myevent);

    print_info("will add filter");

    var filter = efl.Ecore.Event.addFilter(function() {
        return {count: 0};
    }, function(loop_data, event) {
        assert(event === myevent);
        var c = loop_data.count;
        ++loop_data.count;
        return c != 0;
    }, function(loop_data) {});
    
    efl.Ecore.Timer.add(1, efl.Ecore.Mainloop.quit);
    efl.Ecore.Mainloop.begin();
    assert(captured[1] === 5);
    filter.del();

    efl.Ecore.Event.add(myevent);
    efl.Ecore.Event.add(myevent);
    efl.Ecore.Event.add(myevent);
    
    efl.Ecore.Timer.add(1, efl.Ecore.Mainloop.quit);
    efl.Ecore.Mainloop.begin();
    assert(captured[1] === 8);

    handler2.del();
    handler3.del();
});

    // Ecore job
start_test("ecore jobs", function () {
    captured = false;

    efl.Ecore.Job.add(function() {
        captured = true;
        efl.Ecore.Mainloop.quit();
    });

    assert(captured === false);
    efl.Ecore.Mainloop.begin();
    assert(captured === true);

    captured = false;
    var job = efl.Ecore.Job.add(function() {
        captured = true;
    });
    efl.Ecore.Job.add(efl.Ecore.Mainloop.quit);
    job.del();
    efl.Ecore.Mainloop.begin();
    assert(captured === false);
});

start_test("ecore idle", function () {
    // Ecore idle
    var counter = 1;
    captured = [0, 0, 0, 0, 0];

    efl.Ecore.Idle.add(function() {
        captured[0] = counter;
        counter += 1;
        efl.Ecore.Job.add(function() { print_info('ecore job handler 1'); });
        return efl.Ecore.Mainloop.CALLBACK_DONE;
    });

    efl.Ecore.Idle.addEnterer(function() {
        captured[1] = counter;
        counter += 1;
        return efl.Ecore.Mainloop.CALLBACK_DONE;
    });

    efl.Ecore.Idle.addEnterer(function() {
        captured[2] = counter;
        counter += 1;
        return efl.Ecore.Mainloop.CALLBACK_DONE;
    });

    efl.Ecore.Idle.addEntererBefore(function() {
        captured[3] = counter;
        counter += 1;
        return efl.Ecore.Mainloop.CALLBACK_DONE;
    });

    efl.Ecore.Idle.addExiter(function() {
        captured[4] = counter;
        counter += 1;
        efl.Ecore.Mainloop.quit();
        return efl.Ecore.Mainloop.CALLBACK_DONE;
    });

    efl.Ecore.Mainloop.begin();

    assert(captured[0] === 4, "Ecore.Idle.add test");
    assert(captured[1] === 3, "Ecore.Idle.addEnterer test");
    assert(captured[2] === 2, "Ecore.Idle.addEnterer test two");
    assert(captured[3] === 1, "Ecore.Idle.addEntererBefore test");
    assert(captured[4] === 5, "Ecore.Idle.addExiter test");
});

// Ecore animator
start_test("ecore animator", function () {
    efl.Ecore.Animator.setFrametime(1);
    assert(efl.Ecore.Animator.getFrametime() === 1);
    efl.Ecore.Animator.setFrametime(1 / 50);
    assert(efl.Ecore.Animator.getFrametime() === (1 / 50));

    assert(efl.Ecore.Animator.posMap(0.5, efl.Ecore.Animator.POS_MAP_LINEAR, 0, 0)
           === 0.5);

    efl.Ecore.Animator.setSource(efl.Ecore.Animator.SOURCE_TIMER);
    assert(efl.Ecore.Animator.getSource() === efl.Ecore.Animator.SOURCE_TIMER);
});

// Ecore poller
start_test("ecore poller", function () {
    efl.Ecore.Poller.setPollInterval(efl.Ecore.Poller.CORE, 42);
    assert(efl.Ecore.Poller.getPollInterval(efl.Ecore.Poller.CORE) === 42);
    efl.Ecore.Poller.setPollInterval(efl.Ecore.Poller.CORE, 2);
    assert(efl.Ecore.Poller.getPollInterval(efl.Ecore.Poller.CORE) === 2);
});

start_test("ecore throttle", function () {
    // Ecore throttle

    efl.Ecore.Throttle.adjust(3);
    assert(efl.Ecore.Throttle.get() === 3);
    efl.Ecore.Throttle.adjust(-3);
    assert(efl.Ecore.Throttle.get() === 0);
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
