#!/usr/bin/env node

var suite;
var assert;

if(typeof process !== 'undefined')
{
    console.log('running from nodejs');
    console.log('path', process.env.NODE_PATH);
    console.log("teste1");

    suite = require('eina_js_suite_mod');
    assert = require('assert');
    assert(suite != null);
}
else
{
    assert = function(test, message) { if (test !== true) throw message; };
    console.log('running from libv8')
}

// container tests

console.log("teste");
var l1 = suite.raw_list;
console.log ("l1 ", l1.toString());
assert (l1.length == 3);
var l2 = suite.raw_list;
console.log ("l2 ", l2.toString());
assert (l2.length == 3);
var c = l1.concat(l2);
console.log ("c ", c.toString());
assert (c.length == (l1.length + l2.length));
assert (c[0] == l1[0]);
assert (c[1] == l1[1]);
assert (c[2] == l1[2]);
assert (c[3] == l2[0]);
assert (c[4] == l2[1]);
assert (c[5] == l2[2]);
assert (c.indexOf(c[0]) == 0);
assert (c.indexOf(c[1]) == 1);
assert (c.indexOf(c[2]) == 2);
assert (c.indexOf(c[3]) == 0);
assert (c.indexOf(c[4]) == 1);
assert (c.indexOf(c[5]) == 2);
assert (c.lastIndexOf(c[0]) == 3);
assert (c.lastIndexOf(c[1]) == 4);
assert (c.lastIndexOf(c[2]) == 5);
assert (c.lastIndexOf(c[3]) == 3);
assert (c.lastIndexOf(c[4]) == 4);
assert (c.lastIndexOf(c[5]) == 5);
var s1 = l1.slice(1, 3);
console.log ("s1 ", s1.toString());
assert (s1.length == 2);
assert (s1[0] == l1[1]);
assert (s1[1] == l1[2]);
var s2 = c.slice(1, 3);
console.log ("s2 ", s2.toString());
assert (s2.length == 2);
assert (s2[0] == l1[1]);
assert (s2[1] == l1[2]);

console.log("Finished containers");

// error tests

var captured = false;
console.log("#1");
try {
console.log("#2");
  suite.clear_eina_error();
console.log("#3");
} catch(e) {
console.log("#4");
  console.log("Exception ", e.toString());
console.log("#5");
  captured = true;
}
console.log("#6");
assert(captured === false, 'error #1');
console.log("#7");

captured = false;
try {
console.log("#1");
  suite.set_eina_error();
console.log("#2");
} catch(e) {
console.log("#3");
  console.log("Exception ", e.toString());
console.log("#4");
  assert(e.code === 'Eina_Error', 'error #2');
console.log("#5");
  assert(e.value === 'foobar', 'error #3');
console.log("#6");
  captured = true;
console.log("#7");
}
console.log("#8");
assert(captured === true, 'error #4');

console.log("Finished errors");

// accessor tests

assert(suite.acc.get(0) === 42, 'accessor #1');
assert(suite.acc.get(1) === 24, 'accessor #2');

console.log("Finished accessors");

// iterator tests

assert(suite.it.next().value === 42, 'iterator #1');
assert(suite.it.next().value === 24, 'iterator #2');

console.log("Finished iterators");

// value tests

console.log("x");

var my_value = new suite.value(1);
console.log("x");
var wrapped = my_value.get();
console.log("x");
assert(typeof(wrapped) === 'number', 'value #1');
console.log("x");
assert(wrapped === 1, 'value #2');
console.log("x");

my_value.set(2);
console.log("x");
assert(wrapped === 1, 'value #3');
console.log("x");
wrapped = my_value.get();
console.log("x");
assert(typeof(wrapped) === 'number', 'value #4');
console.log("x");
assert(wrapped === 2, 'value #5');
console.log("x");

my_value.set(true);
console.log("x");
assert(wrapped === 2, 'value #6');
console.log("x");
wrapped = my_value.get();
console.log("x");
// boolean is represented as integer in the efl::eina::value layer
console.log("x");
assert(typeof(wrapped) === 'number', 'value #7');
console.log("x");
assert(wrapped === 1, 'value #8');
console.log("x");

console.log("x");
var captured = false;
console.log("x");
try {
console.log("x");
  my_value.set({type: 'complex object'});
console.log("x");
} catch(e) {
console.log("x");
  assert(e.code === 'std::bad_cast', 'value #9');
console.log("x");
  captured = true;
console.log("x");
}
console.log("x");
assert(captured === true, 'value #10');
console.log("x");

console.log("x");
captured = false;
console.log("x");
try {
console.log("x");
    my_value = new suite.value({type: 'complex object'});
console.log("x");
} catch(e) {
  console.log("e.code ", e.code, ' ', typeof e);
  assert(e.code === 'std::bad_cast', 'value #11');
  console.log("x");
  captured = true;
  console.log("x");
}
assert(captured === true, 'value #12');

// log

function f1(){ suite.log_print(suite.LOG_DOMAIN_GLOBAL, suite.LOG_LEVEL_DBG, 'I changed again'); }; f1();
suite.log_print(suite.LOG_DOMAIN_GLOBAL, suite.LOG_LEVEL_CRITICAL, 'Cool to Hate');
function f2(){ suite.log_print(suite.LOG_DOMAIN_GLOBAL, suite.LOG_LEVEL_WARN, 'One Fine Day'); }; f2();
function f3(){ suite.log_print(suite.mydomain, suite.LOG_LEVEL_INFO, 'Never Gonna Find Me'); }; f3();

mydomain2 = suite.log_domain_register('mydomain2', '');
suite.log_domain_registered_level_set(mydomain2, suite.LOG_LEVEL_DBG);
assert(suite.log_domain_registered_level_get(mydomain2) === suite.LOG_LEVEL_DBG, '#1');

suite.log_print(mydomain2, suite.LOG_LEVEL_ERR, "The Kids Aren't Alright");

suite.log_domain_unregister(mydomain2);
mydomain2 = undefined;

suite.log_color_disable_set(true);
assert(suite.log_color_disable_get() === true, '#2');
suite.log_color_disable_set(false);
assert(suite.log_color_disable_get() === false, '#3');

suite.log_file_disable_set(true);
assert(suite.log_file_disable_get() === true, '#4');
suite.log_file_disable_set(false);
assert(suite.log_file_disable_get() === false, '#5');

suite.log_function_disable_set(true);
assert(suite.log_function_disable_get() === true, '#6');
suite.log_function_disable_set(false);
assert(suite.log_function_disable_get() === false, '#7');

suite.log_abort_on_critical_set(true);
assert(suite.log_abort_on_critical_get() === true, '#8');
suite.log_abort_on_critical_set(false);
assert(suite.log_abort_on_critical_get() === false, '#9');

var entered = false;
suite.log_print_cb_set(function(domain, color, level, file, func, line, msg) {
  assert(domain === 'mydomain', '#10');
  assert(color === '', '#11');
  assert(level === suite.LOG_LEVEL_WARN, '#12');
  assert(file === 'eina_js_suite.js', '#13');
  assert(func === 'f4', '#14');
  assert(line === 236, '#15');
  assert(msg === 'What Happened To You', '#16');
  entered = true;
});

function f4(){ suite.log_print(suite.mydomain, suite.LOG_LEVEL_WARN, 'What Happened To You'); }; f4();
assert(entered === true, '#17');

assert(suite.log_level_get() === suite.LOG_LEVEL_DBG, '#18');
assert(suite.log_level_check(suite.LOG_LEVEL_INFO) === true, '#19');
suite.log_level_set(suite.LOG_LEVEL_CRITICAL);
assert(suite.log_level_get() === suite.LOG_LEVEL_CRITICAL, '#20');
assert(suite.log_level_check(suite.LOG_LEVEL_INFO) === false, '#21');

assert(suite.log_abort_on_critical_get() === false, '#22');
suite.log_abort_on_critical_set(true);
assert(suite.log_abort_on_critical_get() === true, '#23');

suite.log_abort_on_critical_level_set(suite.LOG_LEVEL_CRITICAL);
assert(suite.log_abort_on_critical_level_get() == suite.LOG_LEVEL_CRITICAL, '#24');
suite.log_abort_on_critical_level_set(suite.LOG_LEVEL_ERR);
assert(suite.log_abort_on_critical_level_get() == suite.LOG_LEVEL_ERR, '#25');

suite.log_domain_level_set('mydomain', suite.LOG_LEVEL_WARN);
assert(suite.log_domain_level_get('mydomain') === suite.LOG_LEVEL_WARN, '#26');
suite.log_domain_level_set('mydomain', suite.LOG_LEVEL_INFO);
assert(suite.log_domain_level_get('mydomain') === suite.LOG_LEVEL_INFO, '#27');

assert(typeof(suite.LOG_STATE_START) === 'number', '#28');
assert(typeof(suite.LOG_STATE_STOP) === 'number', '#29');
assert(typeof(suite.log_timing) === 'function', '#30');

// finished tests

console.log ("Test execution with success");

