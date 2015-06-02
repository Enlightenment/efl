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

// Ecore shutdown

suite.ecore_shutdown();

console.log("Finished shutdown");

// finished tests

console.log("Test execution with success");
