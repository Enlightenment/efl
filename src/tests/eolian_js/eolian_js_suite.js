#!/usr/bin/env node

// Preamble //
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

    console.log('efl.node loaded')
    
    // Setup output aliases
    print = console.log;
    printError = function() {
        if (process.argv.indexOf("--supress-errors") == -1)
            console.error.apply(null, arguments);
    };
    printInfo = function() {
        if (process.argv.indexOf("--verbose") != -1)
            console.info.apply(null, arguments);
    };
    exit = efl.Ecore.Mainloop.quit;
}
else
{
    assert = function(test, message) { if (!test) throw message; };
    print('running from libv8')
    //FIXME Add levels to v8 tests
    printError = print
    printInfo = print
    exit = function() {}
}

// Global flag for suite success //
suiteSuccess = true;
// Global test summary
suiteOk = 0;
suiteFail = []; // Will store the name of the failures

// Basic test function //
function startTest(testName, testFunc) {
  print("[ RUN         ]  eolian_js_suite: " + testName);
  var testResult = true;
  try {
    testFunc();
  } catch (e) {
    suiteSuccess = false;
    testResult = false;
    printError("Error: ", e, e.stack);
  }
  print("[        " + (testResult ? "PASS" : "FAIL") + " ]  eolian_js_suite: " + testName);
  if (testResult)
    suiteOk += 1;
  else
    suiteFail.push(testName);
}
// end Preamble

// eolian Preamble
if (typeof process != "undefined")
{
    suite = require('eolian_js_suite_mod');
    assert(suite != null, "could not load eolian_js_suite_mod");
    ConstructorMethodClass = suite.ConstructorMethodClass;
    TestObject = suite.Test.Object;
}
else
{
    ConstructorMethodClass = suite.ConstructorMethodClass;
    TestObject = suite.Test.Object;
}

function expectException(func) {
  var exceptionCaught = false;
  try {
    func();
  } catch (e) {
    exceptionCaught = true;
  }
  assert(exceptionCaught, "Exception expected but not thrown!");
}
// end eolian Preamble

// Test cases //
startTest("constructor_null_parent", function() {
  var obj = new TestObject(null);
  assert(obj !== null);
});

startTest("integral_in_and_out_parameters", function() {
  var obj = new TestObject(null);

  var expectedValue = 1234;
  obj.checkMethodIntegralInA(expectedValue);

  var actualValue = obj.checkMethodIntegralOutA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

startTest("integral_inout_parameter", function() {
  var obj = new TestObject(null);

  var expectedValue = 1234;
  var actualValue = obj.checkMethodIntegralInout(-expectedValue);
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);

  var expectedValue = -4321;
  var actualValue = obj.checkMethodIntegralInout(-expectedValue);
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

startTest("integral_return_value", function() {
  var obj = new TestObject(null);

  var expectedValue = 1234;
  obj.checkMethodIntegralInA(expectedValue);

  var actualValue = obj.checkMethodIntegralReturnA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

startTest("more_parameters_than_expected_is_ok", function() {
  var obj = new TestObject(null);
  var expectedValue = 1234;
  obj.checkMethodIntegralInA(expectedValue, 4321);

  var actualValue = obj.checkMethodIntegralOutA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

startTest("less_parameters_that_expected_fails", function() {
  var obj = new TestObject(null);
  expectException(function() {
    obj.checkMethodIntegralInA();
  });
});

startTest("wrong_parameter_type_fails", function() {
  var obj = new TestObject(null);
  expectException(function() {
    obj.checkMethodIntegralInA('string');
  });
});

startTest("mixed_in_out_and_result", function() {
  var obj = new TestObject(null);
  var ret = obj.checkMethodDivMod(7, 3);
  var success = ret[0];
  var quotient = ret[1];
  var remainder = ret[2];
  assert(success);
  assert(2 == quotient);
  assert(1 == remainder);

  ret = obj.checkMethodDivMod(42, 0);
  success = ret[0];
  assert(!success);
});

startTest("boolean", function() {
  var obj = new TestObject(null);
  var ret = obj.checkMethodAnd(false, false);
  assert(ret == false);
  ret = obj.checkMethodAnd(false, true);
  assert(ret == false);
  ret = obj.checkMethodAnd(true, false);
  assert(ret == false);
  ret = obj.checkMethodAnd(true, true);
  assert(ret);
});

startTest("floating_point", function() {
  var obj = new TestObject(null);
  var ret = obj.checkMethodModf(3.14159);
  assert(Math.abs(ret[0] - 0.14159) < 0.00001, "Math.abs(ret[0] - 0.14159) < 0.00001 (" + Math.abs(ret[0] - 0.14159) + " < 0.00001)");
  assert(ret[1] == 3, "ret[1] == 3 (" + ret[1] + " == 3)");
});

startTest("string_inout", function() {
  var obj = new TestObject(null);
  var ret = obj.checkMethodUppercase('hello world');
  assert(ret == "HELLO WORLD", "ret == " + ret);
});

startTest("in_null_string", function() {
  var obj = new TestObject(null);
  var success = obj.checkMethodInNull(null);
  assert(success, "success == " + success);
});


startTest("out_null_string", function() {
  var obj = new TestObject(null);
  var ret = obj.checkMethodOutNull();
  assert(ret[0], "success == " + ret[0]);
  assert(ret[1] === null, "output == " + ret[1]);
});


startTest("inout_null_string", function() {
  var obj = new TestObject(null);
  var ret = obj.checkMethodInoutNull(null);
  assert(ret[0], "success == " + ret[0]);
  assert(ret[1] === null, "output == " + ret[1]);
});


startTest("return_null_string", function() {
  var obj = new TestObject(null);
  var ret = obj.checkMethodReturnNull();
  assert(ret === null, "ret == " + ret);
});


startTest("null_values", function() {
  var obj = new TestObject(null);
  var ret = obj.checkMethodNull(null, null);
  assert(ret[0] === null, "ret == " + ret[0]);
  assert(ret[1] === null, "ret == " + ret[1]);
  assert(ret[2] === null, "ret == " + ret[2]);
});

startTest("enum_values", function() {
  var obj = new TestObject(null);
  var ret = obj.checkMethodInEnumReturnEnum(suite.Test.EnumEx.SECOND);
  assert(ret === suite.Test.EnumEx.SECOND);
  var value = suite.Test.EnumEx.THIRD;
  assert(value === 2);
  ret = obj.checkMethodInEnumReturnEnum(value);
  assert(ret === value);
});

startTest("struct_values", function() {
  var obj = new TestObject(null);
  var newStruct = new suite.Test.StructEx;
  newStruct.valueInt = 42;
  newStruct.valueEnum = suite.Test.EnumEx.FOURTH;
  var ret = newStruct.valueInt;
  assert(ret === 42);
  ret = newStruct.valueEnum;
  assert(ret === suite.Test.EnumEx.FOURTH);
  ret = obj.checkMethodInStructReturnStruct(newStruct);
  assert(ret.valueInt === 42);
  assert(ret.valueEnum === suite.Test.EnumEx.FOURTH);
});

// Events //

startTest("event_simple", function() {
  var v = false;
  var obj = new TestObject(null);
    var ret = obj.on
    ("test",
     function()
     {
         printInfo('Event called')
         v = true;
     }
    );
   printInfo('going to call event');
   obj.emitEvent();
   printInfo('is event called?');
   assert(v);
});

startTest("event_object_call", function() {
  var v = false;
  var obj = new TestObject(null);
    var ret = obj.on
    ("test",
     function()
     {
         printInfo('Event called');
         var o = arguments[0];
         assert(o != null);
         var expectedValue = 1234;
         o.checkMethodIntegralInA(expectedValue);
         var actualValue = o.checkMethodIntegralOutA();
         assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
         v = true;
     }
    );
   printInfo('going to call event');
   obj.emitEvent();
   printInfo('is event called?');
   assert(v);
});

startTest("event_structarg", function() {
  var v = false;
  var obj = new TestObject(null);
    var ret = obj.on
    ("test,structarg",
     function()
     {
         printInfo('Event called');
         var s = arguments[1];
         assert(s != null);
         assert(s.valueInt === 42);
         assert(s.valueEnum === suite.Test.EnumEx.THIRD);
         v = true;
     }
    );
   printInfo('going to call event');
   obj.emitEvent();
   printInfo('is event called?');
   assert(v);
});

startTest("event_stringarg", function() {
  var v = false;
  var obj = new TestObject(null);
    var ret = obj.on
    ("test,stringarg",
     function()
     {
         printInfo('Event called');
         var s = arguments[1];
         assert(s === "foo");
         v = true;
     }
    );
   printInfo('going to call event');
   obj.emitEvent();
   printInfo('is event called?');
   assert(v);
});

// Array //

// // TODO: disabled. Not implemented yet
// startTest("integral_array", function() {
//   var obj = new TestObject(null);
//   var ret = obj.checkMethodArrayAt([1, 2, 3, 4, 5], 1);
//   assert(ret == 2, "ret == " + ret);
// });

startTest("array_in_array_out", function() {
  var obj = new TestObject(null);
  var newArray = obj.checkMethodArrayWith42();
  assert(newArray != null);
  var arr = obj.checkMethodArrayInArrayOut(newArray);
  assert(arr != null);
  assert(arr[0] === 42);
  assert(newArray[0] === arr[0]);
});

startTest("method_array_of_objects", function() {
  var obj = new TestObject(null);
  var arr = obj.checkMethodArrayOfObjects(null);
  assert(arr != null);
  assert(arr[0] != null);
  arr = obj.checkMethodArrayOfObjects(arr);
  assert(arr != null);
  var v = arr[0];
  assert(v != null);
  // assert(v == obj); // TODO: check if same Eo object pointer?
  var expectedValue = 1234;
  v.checkMethodIntegralInA(expectedValue);
  var actualValue = v.checkMethodIntegralOutA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

// FIXME
// startTest("method_array_of_strings", function() {
//   var obj = new TestObject(null);
//   var arr = obj.checkMethodArrayOfStrings(null);
//   assert(arr != null);
//   assert(arr[0] === "foo");
//   arr = obj.checkMethodArrayOfStrings(arr);
//   assert(arr != null);
//   assert(arr[0] === "foo");
// });

startTest("method_array_of_ints", function() {
  var obj = new TestObject(null);
  var arr = obj.checkMethodArrayOfInts(null);
  assert(arr != null);
  assert(arr[0] === 42);
  arr = obj.checkMethodArrayOfInts(arr);
  assert(arr != null);
  assert(arr[0] === 42);
});

startTest("method_array_of_bools", function() {
  var obj = new TestObject(null);
  var arr = obj.checkMethodArrayOfBools(null);
  assert(arr != null);
  assert(arr[0] === true);
  arr = obj.checkMethodArrayOfBools(arr);
  assert(arr != null);
  assert(arr[0] === true);
});

startTest("method_array_of_doubles", function() {
  var obj = new TestObject(null);
  var arr = obj.checkMethodArrayOfDoubles(null);
  assert(arr != null);
  assert(arr[0] === 42.0);
  arr = obj.checkMethodArrayOfDoubles(arr);
  assert(arr != null);
  assert(arr[0] === 42.0);
});

startTest("method_array_of_enums", function() {
  var obj = new TestObject(null);
  var arr = obj.checkMethodArrayOfEnums(null);
  assert(arr != null);
  assert(arr[0] === suite.Test.EnumEx.THIRD);
  arr = obj.checkMethodArrayOfEnums(arr);
  assert(arr != null);
  assert(arr[0] === suite.Test.EnumEx.THIRD);
});

startTest("method_array_of_structs", function() {
  var obj = new TestObject(null);
  var arr = obj.checkMethodArrayOfStructs(null);
  assert(arr != null);
  var s = arr[0];
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
  arr = obj.checkMethodArrayOfStructs(arr);
  s = arr[0];
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
});

// List //

startTest("list_in_list_out", function() {
  var obj = new TestObject(null);
  var newList = obj.checkMethodListWith42();
  assert(newList != null);
  var lis = obj.checkMethodListInListOut(newList);
  assert(lis != null);
  // assert(lis == newList); // TODO: check same list pointer?
  assert(lis[0] === 42);
  assert(newList[0] === lis[0]);
});

startTest("method_list_of_objects", function() {
  var obj = new TestObject(null);
  var lis = obj.checkMethodListOfObjects(null);
  assert(lis != null);
  assert(lis[0] != null);
  lis = obj.checkMethodListOfObjects(lis);
  assert(lis != null);
  var v = lis[0];
  assert(v != null);
  // assert(v == obj); // TODO: check if same Eo object pointer?
  var expectedValue = 1234;
  v.checkMethodIntegralInA(expectedValue);
  var actualValue = v.checkMethodIntegralOutA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

// FIXME
// startTest("method_list_of_strings", function() {
//   var obj = new TestObject(null);
//   var lis = obj.checkMethodListOfStrings(null);
//   assert(lis != null);
//   assert(lis[0] === "foo");
//   lis = obj.checkMethodListOfStrings(lis);
//   assert(lis != null);
//   assert(lis[0] === "foo");
// });

startTest("method_list_of_ints", function() {
  var obj = new TestObject(null);
  var lis = obj.checkMethodListOfInts(null);
  assert(lis != null);
  assert(lis[0] === 42);
  lis = obj.checkMethodListOfInts(lis);
  assert(lis != null);
  assert(lis[0] === 42);
});

startTest("method_list_of_bools", function() {
  var obj = new TestObject(null);
  var lis = obj.checkMethodListOfBools(null);
  assert(lis != null);
  assert(lis[0] === true);
  lis = obj.checkMethodListOfBools(lis);
  assert(lis != null);
  assert(lis[0] === true);
});

startTest("method_list_of_doubles", function() {
  var obj = new TestObject(null);
  var lis = obj.checkMethodListOfDoubles(null);
  assert(lis != null);
  assert(lis[0] === 42.0);
  lis = obj.checkMethodListOfDoubles(lis);
  assert(lis != null);
  assert(lis[0] === 42.0);
});

startTest("method_list_of_enums", function() {
  var obj = new TestObject(null);
  var lis = obj.checkMethodListOfEnums(null);
  assert(lis != null);
  assert(lis[0] === suite.Test.EnumEx.THIRD);
  lis = obj.checkMethodListOfEnums(lis);
  assert(lis != null);
  assert(lis[0] === suite.Test.EnumEx.THIRD);
});

startTest("method_list_of_structs", function() {
  var obj = new TestObject(null);
  var lis = obj.checkMethodListOfStructs(null);
  assert(lis != null);
  var s = lis[0];
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
  lis = obj.checkMethodListOfStructs(lis);
  s = lis[0];
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
});

// Accessor //

startTest("method_accessor_of_objects", function() {
  var obj = new TestObject(null);
  var acc = obj.checkMethodAccessorOfObjects(null);
  assert(acc != null);
  assert(acc.get(0) != null);
  acc = obj.checkMethodAccessorOfObjects(acc);
  assert(acc != null);
  var v = acc.get(0);
  assert(v != null);
  var expectedValue = 1234;
  v.checkMethodIntegralInA(expectedValue);
  var actualValue = v.checkMethodIntegralOutA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

// FIXME
// startTest("method_accessor_of_strings", function() {
//   var obj = new TestObject(null);
//   var acc = obj.checkMethodAccessorOfStrings(null);
//   assert(acc != null);
//   assert(acc.get(0) === "foo");
//   acc = obj.checkMethodAccessorOfStrings(acc);
//   assert(acc != null);
//   assert(acc.get(0) === "foo");
// });

startTest("method_accessor_of_ints", function() {
  var obj = new TestObject(null);
  var acc = obj.checkMethodAccessorOfInts(null);
  assert(acc != null);
  assert(acc.get(0) === 42);
  acc = obj.checkMethodAccessorOfInts(acc);
  assert(acc != null);
  assert(acc.get(0) === 42);
});

startTest("method_accessor_of_bools", function() {
  var obj = new TestObject(null);
  var acc = obj.checkMethodAccessorOfBools(null);
  assert(acc != null);
  assert(acc.get(0) === true);
  acc = obj.checkMethodAccessorOfBools(acc);
  assert(acc != null);
  assert(acc.get(0) === true);
});

startTest("method_accessor_of_doubles", function() {
  var obj = new TestObject(null);
  var acc = obj.checkMethodAccessorOfDoubles(null);
  assert(acc != null);
  assert(acc.get(0) === 42.0);
  acc = obj.checkMethodAccessorOfDoubles(acc);
  assert(acc != null);
  assert(acc.get(0) === 42.0);
});

startTest("method_accessor_of_enums", function() {
  var obj = new TestObject(null);
  var acc = obj.checkMethodAccessorOfEnums(null);
  assert(acc != null);
  assert(acc.get(0) === suite.Test.EnumEx.THIRD);
  acc = obj.checkMethodAccessorOfEnums(acc);
  assert(acc != null);
  assert(acc.get(0) === suite.Test.EnumEx.THIRD);
});

startTest("method_accessor_of_structs", function() {
  var obj = new TestObject(null);
  var acc = obj.checkMethodAccessorOfStructs(null);
  assert(acc != null);
  var s = acc.get(0);
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
  acc = obj.checkMethodAccessorOfStructs(acc);
  assert(acc != null);
  s = acc.get(0);
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
});

// Iterator //

startTest("method_iterator_of_objects", function() {
  var obj = new TestObject(null);
  var it = obj.checkMethodIteratorOfObjects(null);
  assert(it != null);
  it = obj.checkMethodIteratorOfObjects(it);
  assert(it != null);
  var idx = 0;
  for (var n = it.next(); !n.done; n = it.next()) {
    var v = n.value;
    assert(v != null);
    var expectedValue = 1234;
    v.checkMethodIntegralInA(expectedValue);
    var actualValue = v.checkMethodIntegralOutA();
    assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
    idx++;
  }
  assert(idx == 2, idx + " == 2");
});

startTest("method_iterator_of_strings", function() {
  var obj = new TestObject(null);
  var it = obj.checkMethodIteratorOfStrings(null);
  assert(it != null);
  it = obj.checkMethodIteratorOfStrings(it);
  assert(it != null);
  var cmp = ["foo", "bar"];
  var idx = 0;
  for (var n = it.next(); !n.done; n = it.next()) {
    var v = n.value;
    assert(v === cmp[idx], idx+": "+v+" === "+cmp[idx]);
    idx++;
  }
  assert(idx == 2, idx + " == 2");
});

startTest("method_iterator_of_ints", function() {
  var obj = new TestObject(null);
  var it = obj.checkMethodIteratorOfInts(null);
  assert(it != null);
  it = obj.checkMethodIteratorOfInts(it);
  assert(it != null);
  var cmp = [42, 24];
  var idx = 0;
  for (var n = it.next(); !n.done; n = it.next()) {
    var v = n.value;
    assert(v === cmp[idx], idx+": "+v+" === "+cmp[idx]);
    idx++;
  }
  assert(idx == 2, idx + " == 2");
});

startTest("method_iterator_of_bools", function() {
  var obj = new TestObject(null);
  var it = obj.checkMethodIteratorOfBools(null);
  assert(it != null);
  it = obj.checkMethodIteratorOfBools(it);
  assert(it != null);
  var cmp = [true, false];
  var idx = 0;
  for (var n = it.next(); !n.done; n = it.next()) {
    var v = n.value;
    assert(v === cmp[idx], idx+": "+v+" === "+cmp[idx]);
    idx++;
  }
  assert(idx == 2, idx + " == 2");
});

startTest("method_iterator_of_doubles", function() {
  var obj = new TestObject(null);
  var it = obj.checkMethodIteratorOfDoubles(null);
  assert(it != null);
  it = obj.checkMethodIteratorOfDoubles(it);
  assert(it != null);
  var cmp = [42.0, 24.0];
  var idx = 0;
  for (var n = it.next(); !n.done; n = it.next()) {
    var v = n.value;
    assert(v === cmp[idx], idx+": "+v+" === "+cmp[idx]);
    idx++;
  }
  assert(idx == 2, idx + " == 2");
});

startTest("method_iterator_of_enums", function() {
  var obj = new TestObject(null);
  var it = obj.checkMethodIteratorOfEnums(null);
  assert(it != null);
  it = obj.checkMethodIteratorOfEnums(it);
  assert(it != null);
  var cmp = [suite.Test.EnumEx.THIRD, suite.Test.EnumEx.FIRST];
  var idx = 0;
  for (var n = it.next(); !n.done; n = it.next()) {
    var v = n.value;
    assert(v === cmp[idx], idx+": "+v+" === "+cmp[idx]);
    idx++;
  }
  assert(idx == 2, idx + " == 2");
});

startTest("method_iterator_of_structs", function() {
  var obj = new TestObject(null);
  var it = obj.checkMethodIteratorOfStructs(null);
  assert(it != null);
  it = obj.checkMethodIteratorOfStructs(it);
  assert(it != null);
  var cmp = [[42, suite.Test.EnumEx.THIRD], [24, suite.Test.EnumEx.FIRST]];
  var idx = 0;
  for (var n = it.next(); !n.done; n = it.next()) {
    var v = n.value;
    assert(v != null);
    assert(v.valueInt === cmp[idx][0], idx+": "+v.valueInt+" === "+cmp[idx][0]);
    assert(v.valueEnum === cmp[idx][1], idx+": "+v.valueEnum+" === "+cmp[idx][1]);
    idx++;
  }
  assert(idx == 2, idx + " == 2");
});

// Combinations of complex types //

// FIXME
// startTest("method_array_of_arrays_of_ints", function() {
//   var obj = new TestObject(null);
//   var arr = obj.checkMethodArrayOfArraysOfInts(null);
//   assert(arr != null);
//   var a = arr[0];
//   assert(a != null);
//   assert(a[0] === 42);
//   arr = obj.checkMethodArrayOfArraysOfInts(arr);
//   assert(arr != null);
//   a = arr[0];
//   assert(a != null);
//   assert(a[0] === 42);
// });

// FIXME
// startTest("method_list_of_lists_of_ints", function() {
//   var obj = new TestObject(null);
//   var lis = obj.checkMethodListOfListsOfInts(null);
//   assert(lis != null);
//   var l = lis[0];
//   assert(l != null);
//   assert(l[0] === 42);
//   lis = obj.checkMethodListOfListsOfInts(lis);
//   assert(lis != null);
//   l = lis[0];
//   assert(l != null);
//   assert(l[0] === 42);
// });

// FIXME
// startTest("method_array_of_lists_of_ints", function() {
//   var obj = new TestObject(null);
//   var arr = obj.checkMethodArrayOfListsOfInts(null);
//   assert(arr != null);
//   var l = arr[0];
//   assert(l != null);
//   assert(l[0] === 42);
//   arr = obj.checkMethodArrayOfListsOfInts(arr);
//   assert(arr != null);
//   l = arr[0];
//   assert(l != null);
//   assert(l[0] === 42);
// });

// FIXME
// startTest("method_list_of_arrays_of_ints", function() {
//   var obj = new TestObject(null);
//   var lis = obj.checkMethodListOfArraysOfInts(null);
//   assert(lis != null);
//   var a = lis[0];
//   assert(a != null);
//   assert(a[0] === 42);
//   lis = obj.checkMethodListOfArraysOfInts(lis);
//   assert(lis != null);
//   a = lis[0];
//   assert(a != null);
//   assert(a[0] === 42);
// });

// Garbage Collection //
startTest("gc_object", function() {
  var freed = false;
  (function() {
    var obj = new TestObject(null);
    obj.on("del", function() {
           printInfo('Object destructed')
           freed = true;
    });
  }());
  printInfo('going to garbage collect');
  global.gc();
  printInfo('is object destructed?');
  assert(freed);
});

startTest("new Constructor_Method_Class", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  assert(obj);
  assert(obj.getFail() == false, "object fail flag set");
});

startTest("Constructor_Method_Class method1", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  obj.method1(2);
  assert(obj.getFail() == false, "object fail flag set");
});

startTest("Constructor_Method_Class method2", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  var r = obj.method2(3);
  assert(r === 5);
  assert(obj.getFail() == false, "object fail flag set");
});

startTest("Constructor_Method_Class method3", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  obj.method3(3, 11.1);
  assert(obj.getFail() == false, "object fail flag set");
});

startTest("Constructor_Method_Class method4", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  var r = obj.method4(3, 11.1);
  assert(r == 5);
  assert(obj.getFail() == false, "object fail flag set");
});

startTest("Constructor_Method_Class method3 less parameters than expected", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  expectException(function() {
    obj.method3(3);
  });
});

startTest("Constructor_Method_Class constructor less parameters than expected", function() {
  expectException(function() {
    var obj = new ConstructorMethodClass(null, 5);
  });
});

startTest("Constructor_Method_Class constructor wrong parameter types", function() {
  expectException(function() {
    var obj = new ConstructorMethodClass(null, 5.1, 10);
  });
});

// Out parameters //

startTest("Constructor_Method_Class outmethod1", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  var r = obj.outmethod1();
  assert(r == 5);
  assert(obj.getFail() == false, "object fail flag set");
});

startTest("Constructor_Method_Class outmethod2", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  var r = obj.outmethod2();
  assert(Array.isArray(r));
  printInfo("Return is array from outmethod2");
  assert(r[0] == 11);
  assert(r[1] == 10);
  assert(obj.getFail() == false, "object fail flag set");
});

startTest("Constructor_Method_Class outmethod3", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  var r = obj.outmethod3();
  assert(Array.isArray(r));
  printInfo("Return is array from outmethod3");
  assert(r[0] == 11);
  assert(r[1] == 5.1);
  assert(obj.getFail() == false, "object fail flag set");
});

startTest("Constructor_Method_Class classoutmethod1", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  var c = obj.classoutmethod1(5, 10.0);
  assert(c);
  var r = c.method4(3, 11.1);
  assert(r == 5);
  assert(c.getFail() == false, "object fail flag set");
  assert(obj.getFail() == false, "object fail flag set");
});

startTest("Constructor_Method_Class classoutmethod2", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  printInfo('1');
  var c = obj.classoutmethod2(5, 10.0);
  assert(c);
  var r = c.method4(3, 11.1);
  assert(r);
  assert(r == 5);
  assert(c.getFail() == false, "object fail flag set");
  assert(obj.getFail() == false, "object fail flag set");
});

startTest("cast", function() {
  var obj = new ConstructorMethodClass(null, 5, 10.0);
  var c = obj.classoutmethod1(5, 10.0);
  print('classoutmethod1');
  assert(c);
  var d = c.cast('Constructor_Method_Class');
  assert(d);
});

if (!suiteSuccess) {
  print ("[ Total tests run: %s ]", suiteOk + suiteFail.length);
  print ("[ Total successful: %s ]", suiteOk);
  print ("[ Total failures: %s ]", suiteFail.length);
  print ("[ Tests failed: ]");
  for (var i = 0; i < suiteFail.length; i++) {
    print ("[    %s]", suiteFail[i]);
  };
  assert(false, "[ Test suite fail ]");
} else {
  print ("[ Test execution with success ]");
  print ("[ Total tests run: %s ]", suiteOk);
}

exit();
