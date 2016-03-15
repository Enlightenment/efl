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
// end Preamble

start_test("container_test default list should have size zero", function(){
  var container = new efl.List;
  assert (container.length == 0);
});

start_test("container test default Array should have size zero", function(){
  var container = new efl.Array;
  assert (container.length == 0);
});

start_test("container_test empty list access should return undefined", function(){
  var empty_l = new efl.List;
  assert (typeof empty_l[0] === "undefined");
  assert (typeof empty_l[1] === "undefined");
});

start_test("container_test empty Array access should return undefined", function(){
  var empty_l = new efl.Array;
  assert (typeof empty_l[0] === "undefined");
  assert (typeof empty_l[1] === "undefined");
});

start_test("container_test empty list indexOf should return -1", function(){
  var empty_l = new efl.List;
  assert(empty_l.indexOf(0) == -1);
  assert(empty_l.indexOf(42) == -1);
});

start_test("container_test empty Array indexOf should return -1", function(){
  var empty_l = new efl.Array;
  assert(empty_l.indexOf(0) == -1);
  assert(empty_l.indexOf(42) == -1);
});

start_test("container_test List indexOf should not break with wrong type", function(){
  container_indexof_wrong_type(efl.List);
});

start_test("container_test Array indexOf should not break with wrong type", function(){
  container_indexof_wrong_type(efl.Array);
});

function container_indexof_wrong_type(T) {
  var x = new T("int");
  var raised = false;
  try {
    x.indexOf("float")
  } catch (err) {
    raised = true;
  }

  assert(!raised, "container index of should not break with wrong types");
}

start_test("container_test list[0] = x on empty list", function(){
  var empty_l = new efl.List;
  assert(empty_l.length == 0);
  empty_l[0] = 42;
  assert(empty_l.length == 1);
  assert(empty_l[0] == 42);
});

start_test("container_test array[0] = x on empty Array", function(){
  var empty_l = new efl.Array;
  assert(empty_l.length == 0);
  empty_l[0] = 42;
  assert(empty_l.length == 1);
  assert(empty_l[0] == 42);
});

start_test("container_test out of bounds x[i] setter should resize list", function(){
  container_out_of_bound_setter(efl.List);
});

start_test("container_test out of bounds x[i] setter should resize Array", function(){
  container_out_of_bound_setter(efl.Array);
});

function container_out_of_bound_setter(T1) {
  var empty_l = new T1;
  assert(empty_l.length == 0);
  empty_l[9] = 1;
  assert(empty_l.length == 10);
  for (var i = 0; i < 9; i++) {
    assert(empty_l[i] == 0);
  };
  assert(empty_l[9] == 1);
}

start_test("container_test push items should increase list length", function(){
  container_push_length(efl.List);
});

start_test("container_test push items should increase Array length", function(){
  container_push_length(efl.Array);
});

function container_push_length(T1) {
  var l1 = new T1;
  print_info("l1 ", l1.toString());
  l1.push(1);
  assert (l1.length == 1);
  l1.push(2);
  assert (l1.length == 2);
  l1.push(3);
  assert (l1.length == 3);
}

start_test("container_test list simple push and access valid items", function(){
  container_push_access_valid(efl.List);
});

start_test("container_test Array simple push and access valid items", function(){
  container_push_access_valid(efl.Array);
});

function container_push_access_valid(T1) {
  var l1 = new T1;
  l1.push(1);
  l1.push(2);
  assert (l1[0] == 1);
  assert (l1[1] == 2);
}

start_test("container_test list simple push and access items out of bounds", function(){
  container_push_out_of_bounds(efl.List);
});

start_test("container_test Array simple push and access items out of bounds", function(){
  container_push_out_of_bounds(efl.Array);
});

function container_push_out_of_bounds(T1) {
  var l1 = new T1;
  l1.push(1);
  l1.push(2);
  assert (typeof l1[3] === "undefined");
  assert (typeof l1[42] === "undefined");
}

start_test("container_test list push and pop", function() {
  container_push_pop(efl.List);
});

start_test("container_test Array push and pop", function() {
  container_push_pop(efl.Array);
});

function container_push_pop(T1) {
  var l1 = new T1;

  assert(typeof l1.pop() === "undefined", "pop on empty container should return undefined");

  l1.push(1);
  l1.push(2);
  assert(l1.length == 2);
  assert(l1.pop() == 2, "Pop should return last element");
  assert(l1.length == 1, "Pop should decrease list length");
  assert(l1.pop() == 1, "Pop should return last element even if it's the only element");
  assert(l1.length == 0, "Pop on single element list should leave it empty");
}

start_test("container_test list simple push and indexOf valid elements", function(){
  container_push_indexof_valid(efl.List)
});

start_test("container_test Array simple push and indexOf valid elements", function(){
  container_push_indexof_valid(efl.Array)
});

function container_push_indexof_valid(T1) {
  var l1 = new T1;
  l1.push(1);
  l1.push(2);
  assert(l1.indexOf(1) == 0);
  assert(l1.indexOf(2) == 1);
}

start_test("container_test list simple push and indexOf elements out of bounds", function(){
  container_push_indexof_out_of_bounds(efl.List);
});

start_test("container_test list simple push and indexOf elements out of bounds", function(){
  container_push_indexof_out_of_bounds(efl.Array);
});

function container_push_indexof_out_of_bounds(T1) {
  var l1 = new T1;
  l1.push(1);
  l1.push(2);
  assert(l1.indexOf(3) == -1);
  assert(l1.indexOf(44) == -1);
}

start_test("container_test list concat filled + empty shouldn't change length", function(){
  container_filled_concat_empty_length(efl.List);
});

start_test("container_test array concat filled + empty shouldn't change length", function(){
  container_filled_concat_empty_length(efl.Array);
});

function container_filled_concat_empty_length(T1) {
    var empty_l = new T1;
    var l1 = new T1;
    l1.push(1);
    l1.push(2);
    l1.push(3);
    var cle = l1.concat(empty_l);
    assert (cle.length == 3);
}

start_test("container_test list concat empty + filled shouldn't change length", function(){
  container_empty_concat_filled_length(efl.List);
});

start_test("container_test array concat empty + filled shouldn't change length", function(){
  container_empty_concat_filled_length(efl.Array);
});

function container_empty_concat_filled_length(T1) {
    var empty_l = new T1;
    var l1 = new T1;
    l1.push(1);
    l1.push(2);
    l1.push(3);
    var cel = empty_l.concat(l1);
    assert (cel.length == 3);
}

start_test("container_test concat two valid lists", function(){
  container_concat_two_valid_containers(efl.List, efl.List);
});

start_test("container_test concat two valid arrays", function(){
  container_concat_two_valid_containers(efl.Array, efl.Array);
});

function container_concat_two_valid_containers(T1, T2) {
    l1 = new T1;
    l1.push(1);
    l1.push(2);
    l1.push(3);
    l2 = new T2;
    l2.push(1);
    l2.push(2);
    l2.push(3);
    var c = l1.concat(l2);

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
}

start_test("container_test concat different types", function() {
  container_concat_diff_types(efl.Array, efl.List);
  container_concat_diff_types(efl.List, efl.Array);
});

function container_concat_diff_types(T1, T2) {
  var l1 = new T1;
  var l2 = new T2;
  var raised = false;

  try {
    l1.concat(l2);
    assert(false, "Should raise exception after concatenating two different types.")
  } catch (err) {
    raised = true;
    assert(err.name == "TypeError", "Exception should be TypeError.");
  }

  assert(raised, "Exception was not raised after concatenating different types.");
}

start_test("container_test toString list", function(){
  container_to_string(efl.List);
});

start_test("container_test toString array", function(){
  container_to_string(efl.Array);
});

function container_to_string(T) {
  var x = new T("int");

  assert("" == x.toString(), "toString empty container");

  x.push(1);
  assert("1" == x.toString(), "toString single element");

  x.push(2);
  assert("1,2" == x.toString(), "toString two elements");

  x.push(100);
  assert("1,2,100" == x.toString(), "toString three elements");
}

start_test("container_test join list", function(){
  container_join(efl.List);
});

start_test("container_test join Array", function(){
  container_join(efl.Array);
});

function container_join(T) {
  var x = new T("int");
  var ref = new Array;

  assert(ref.join() == x.join(), "default join on empty containers");
  assert(ref.join(':') == x.join(':'), "arg join on empty containers");

  x.push(42);
  ref.push(42);

  assert(ref.join() == x.join(), "default join on single-element containers");
  assert(ref.join(':') == x.join(':'), "arg join on single-element containers");

  x.push(314);
  ref.push(314);

  assert(ref.join() == x.join(), "default join on multiple-element containers");
  assert(ref.join(':') == x.join(':'), "arg join on multiple-element containers");
}

start_test("container_test join wrong arguments", function(){

});

function container_join_wrong_type(T1, T2) {
  var l1 = new T1("int");
  var raised = false;

  try {
    l1.join({});
    assert(false, "Should raise exception after trying to join with wrong argument type.");
  } catch (err) {
    raised = true;
    assert(err.name == "TypeError", "Exception should be TypeError.");
  }

  assert(raised, "Exception was not raised after join with wrong argument type.");
}

start_test("container_test slice list simple", function () {
    container_slice_simple(efl.List);
});

start_test("container_test slice Array simple", function () {
    container_slice_simple(efl.Array);
});

function container_slice_simple(T1) {
    l1 = new T1;
    l1.push(1);
    l1.push(2);
    l1.push(3);

    var s1 = l1.slice(1, 3);
    assert (s1.length == 2);
    assert (s1[0] == l1[1]);
    assert (s1[1] == l1[2]);
}

start_test("container_test slice list single argument", function() {
    container_slice_single_arg(efl.List);
});

start_test("container_test slice Array single argument", function() {
    container_slice_single_arg(efl.Array);
});

function container_slice_single_arg(T1) {
    l1 = new T1;
    l1.push(0);
    l1.push(1);
    l1.push(2);
    l1.push(3);
    l1.push(4);

    var s1 = l1.slice(1);
    assert(s1.length == (l1.length - 1));
    assert(s1[0] == l1[1]);
    assert(s1[1] == l1[2]);
    assert(s1[2] == l1[3]);
    assert(s1[3] == l1[4]);
}

start_test("container_test slice list no arguments", function() {
    container_slice_no_args(efl.List);
});

start_test("container_test slice Array no arguments", function() {
    container_slice_no_args(efl.Array);
});

function container_slice_no_args(T1) {
    l1 = new T1;
    l1.push(0);
    l1.push(1);
    l1.push(2);
    l1.push(3);
    l1.push(4);

    var s1 = l1.slice();
    assert(s1.length == l1.length);
    assert(s1[0] == l1[0]);
    assert(s1[1] == l1[1]);
    assert(s1[2] == l1[2]);
    assert(s1[3] == l1[3]);
    assert(s1[4] == l1[4]);
}

start_test("container test list of strings", function(){
  container_test_generic(new efl.List("string"), ["The", "quick", "brown", "fox"]);
});

start_test("container test array of strings", function(){
  container_test_generic(new efl.Array("string"), ["The", "quick", "brown", "fox"]);
});

start_test("container test list of floats", function(){
  container_test_generic(new efl.List("float"), [3.42, 3.14, 1.22, 0.0]);
});

start_test("container test Array of floats", function(){
  container_test_generic(new efl.Array("float"), [3.42, 3.14, 1.22, 0.0]);
});

start_test("container test list of bools", function(){
  container_test_generic(new efl.List("bool"), [true, false, false, false]);
});

start_test("container test array of bools", function(){
  container_test_generic(new efl.Array("bool"), [true, false, false, false]);
});

function container_test_generic(list, js_ref_list) {
  assert(list.length == 0);
  list.push(js_ref_list[0]);
  list.push(js_ref_list[1]);
  list.push(js_ref_list[2]);
  list.push(js_ref_list[3]);

  assert(list.indexOf(js_ref_list[0]) == 0, "First pushed element has index 0");
  assert(list.indexOf(js_ref_list[1]) == 1, "Second pushed element has index 1");
  assert(list.lastIndexOf(js_ref_list[0]) == 0, "First element is unique, so its lastIndexOf should be 0");

  assert(list.toString() == js_ref_list.toString(), "toString must be equal to JS.")
  assert(list.pop() == js_ref_list[3], "Pop should return the last list element");
}

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
