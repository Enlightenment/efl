#!/usr/bin/env node

var suite;
var assert;

if(typeof process !== 'undefined') {
    console.log('running from nodejs');
    console.log('path', process.env.NODE_PATH);
    console.log("teste1");

    suite = require('ecore_js_suite_mod');
    assert = require('assert');
    assert(suite != null);
} else {
    assert = function(test, message) { if (test !== true) throw message; };
    console.log('running from libv8')
}

function abs(n) {
    if (n < 0)
        return n * -1;
    return n;
}
var TOLERANCE = 0.0001;

// Ecore initialization

suite.ecore_init();

console.log("Finished init");

var captured = false;

// Handlers

suite.ecore_mainloop_thread_safe_call_async(function() {
    captured = true;
    suite.ecore_mainloop_quit();
});

// Ecore mainloop init

suite.ecore_mainloop_begin();

// ...

assert(captured);

// Timers

var p = 2.5;
suite.ecore_timer_precision_set(p);
assert(abs(suite.ecore_timer_precision_get() - p) < TOLERANCE);

p = 0.5;
suite.ecore_timer_precision_set(p);
assert(abs(suite.ecore_timer_precision_get() - p) < TOLERANCE);

var dump = suite.ecore_timer_dump();
assert(typeof(dump) === 'string');
console.log("### BEGINING of Timer dump: ###");
console.log(dump);
console.log("### END of Timer dump ###");

var ncalls = 0;

captured = false;
suite.ecore_timer_add(1, function() {
    ++ncalls;
    if (ncalls != 4)
        return true;

    captured = true;
    suite.ecore_mainloop_thread_safe_call_async(suite.ecore_mainloop_quit);
    return false;
});

suite.ecore_mainloop_begin();
assert(captured);

ncalls = 0;
captured = false;

suite.ecore_timer_loop_add(1, function() {
    ++ncalls;
    if (ncalls != 4)
        return true;

    captured = true;
    suite.ecore_mainloop_thread_safe_call_async(suite.ecore_mainloop_quit);
    return false;
});

suite.ecore_mainloop_begin();
assert(captured);

captured = false;

var timer = suite.ecore_timer_add(1, function() {
    captured = true;
    return false;
});

assert(timer.freeze_get() === false);

timer.freeze();

assert(timer.freeze_get() === true);

timer.thaw();

assert(timer.freeze_get() === false);

timer.del();

suite.ecore_timer_add(2, function() {
    suite.ecore_mainloop_thread_safe_call_async(suite.ecore_mainloop_quit);
    return false;
});

suite.ecore_mainloop_begin();
assert(captured === false);

// Ecore shutdown

suite.ecore_shutdown();

console.log("Finished shutdown");

// finished tests

console.log("Test execution with success");
