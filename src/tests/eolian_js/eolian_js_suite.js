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
    console.log("teste1");

    efl = require('efl');
    assert(efl != null, "could not load efl node module");

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
    exit = efl.ecore_mainloop_quit;
}
else
{
    assert = function(test, message) { if (test !== true) throw message; };
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
  obj.methodIntegralInA(expectedValue);

  var actualValue = obj.methodIntegralOutA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

startTest("integral_inout_parameter", function() {
  var obj = new TestObject(null);

  var expectedValue = 1234;
  var actualValue = obj.methodIntegralInout(-expectedValue);
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);

  var expectedValue = -4321;
  var actualValue = obj.methodIntegralInout(-expectedValue);
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

startTest("integral_return_value", function() {
  var obj = new TestObject(null);

  var expectedValue = 1234;
  obj.methodIntegralInA(expectedValue);

  var actualValue = obj.methodIntegralReturnA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

startTest("more_parameters_than_expected_is_ok", function() {
  var obj = new TestObject(null);
  var expectedValue = 1234;
  obj.methodIntegralInA(expectedValue, 4321);

  var actualValue = obj.methodIntegralOutA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

startTest("less_parameters_that_expected_fails", function() {
  var obj = new TestObject(null);
  expectException(function() {
    obj.methodIntegralInA();
  });
});

startTest("wrong_parameter_type_fails", function() {
  var obj = new TestObject(null);
  expectException(function() {
    obj.methodIntegralInA('string');
  });
});

startTest("mixed_in_out_and_result", function() {
  var obj = new TestObject(null);
  var ret = obj.methodDivMod(7, 3);
  var success = ret[0];
  var quotient = ret[1];
  var remainder = ret[2];
  assert(success);
  assert(2 == quotient);
  assert(1 == remainder);

  ret = obj.methodDivMod(42, 0);
  success = ret[0];
  assert(!success);
});

startTest("boolean", function() {
  var obj = new TestObject(null);
  var ret = obj.methodAnd(false, false);
  assert(ret == false);
  ret = obj.methodAnd(false, true);
  assert(ret == false);
  ret = obj.methodAnd(true, false);
  assert(ret == false);
  ret = obj.methodAnd(true, true);
  assert(ret);
});

startTest("floating_point", function() {
  var obj = new TestObject(null);
  var ret = obj.methodModf(3.14159);
  assert(Math.abs(ret[0] - 0.14159) < 0.00001, "Math.abs(ret[0] - 0.14159) < 0.00001 (" + Math.abs(ret[0] - 0.14159) + " < 0.00001)");
  assert(ret[1] == 3, "ret[1] == 3 (" + ret[1] + " == 3)");
});

startTest("string_inout", function() {
  var obj = new TestObject(null);
  var ret = obj.methodUppercase('hello world');
  assert(ret == "HELLO WORLD", "ret == " + ret);
});

startTest("in_null_string", function() {
  var obj = new TestObject(null);
  var success = obj.methodInNull(null);
  assert(success, "success == " + success);
});


startTest("out_null_string", function() {
  var obj = new TestObject(null);
  var ret = obj.methodOutNull();
  assert(ret[0], "success == " + ret[0]);
  assert(ret[1] === null, "output == " + ret[1]);
});


startTest("inout_null_string", function() {
  var obj = new TestObject(null);
  var ret = obj.methodInoutNull(null);
  assert(ret[0], "success == " + ret[0]);
  assert(ret[1] === null, "output == " + ret[1]);
});


startTest("return_null_string", function() {
  var obj = new TestObject(null);
  var ret = obj.methodReturnNull();
  assert(ret === null, "ret == " + ret);
});


startTest("null_values", function() {
  var obj = new TestObject(null);
  var ret = obj.methodNull(null, null);
  assert(ret[0] === null, "ret == " + ret[0]);
  assert(ret[1] === null, "ret == " + ret[1]);
  assert(ret[2] === null, "ret == " + ret[2]);
});

startTest("enum_values", function() {
  var obj = new TestObject(null);
  var ret = obj.methodInEnumReturnEnum(suite.Test.EnumEx.SECOND);
  assert(ret === suite.Test.EnumEx.SECOND);
  var value = suite.Test.EnumEx.THIRD;
  assert(value === 2);
  ret = obj.methodInEnumReturnEnum(value);
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
  ret = obj.methodInStructReturnStruct(newStruct);
  assert(ret.valueInt === 42);
  assert(ret.valueEnum === suite.Test.EnumEx.FOURTH);
});

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
   obj.callEvent();
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
         o.methodIntegralInA(expectedValue);
         var actualValue = o.methodIntegralOutA();
         assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
         v = true;
     }
    );
   printInfo('going to call event');
   obj.callEvent();
   printInfo('is event called?');
   assert(v);
});

startTest("event_structarg", function() {
  var v = false;
  var obj = new TestObject(null);
    var ret = obj.on
    ("test_structarg",
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
   obj.callEvent();
   printInfo('is event called?');
   assert(v);
});

startTest("event_stringarg", function() {
  var v = false;
  var obj = new TestObject(null);
    var ret = obj.on
    ("test_stringarg",
     function()
     {
         printInfo('Event called');
         var s = arguments[1];
         assert(s === "foo");
         v = true;
     }
    );
   printInfo('going to call event');
   obj.callEvent();
   printInfo('is event called?');
   assert(v);
});

// // TODO: disabled. Not implemented yet
// startTest("integral_array", function() {
//   var obj = new TestObject(null);
//   var ret = obj.methodArrayAt([1, 2, 3, 4, 5], 1);
//   assert(ret == 2, "ret == " + ret);
// });

startTest("array_in_array_out", function() {
  var obj = new TestObject(null);
  var newArray = obj.methodArrayWith42();
  assert(newArray != null);
  var arr = obj.methodArrayInArrayOut(newArray);
  assert(arr != null);
  assert(arr[0] === 42);
  assert(newArray[0] === arr[0]);
});

startTest("method_array_of_objects", function() {
  var obj = new TestObject(null);
  var arr = obj.methodArrayOfObjects(null);
  assert(arr != null);
  assert(arr[0] != null);
  arr = obj.methodArrayOfObjects(arr);
  assert(arr != null);
  var v = arr[0];
  assert(v != null);
  // assert(v == obj); // TODO: check if same Eo object pointer?
  var expectedValue = 1234;
  v.methodIntegralInA(expectedValue);
  var actualValue = v.methodIntegralOutA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

// FIXME
// startTest("method_array_of_strings", function() {
//   var obj = new TestObject(null);
//   var arr = obj.methodArrayOfStrings(null);
//   assert(arr != null);
//   assert(arr[0] === "foo");
//   arr = obj.methodArrayOfStrings(arr);
//   assert(arr != null);
//   assert(arr[0] === "foo");
// });

startTest("method_array_of_ints", function() {
  var obj = new TestObject(null);
  var arr = obj.methodArrayOfInts(null);
  assert(arr != null);
  assert(arr[0] === 42);
  arr = obj.methodArrayOfInts(arr);
  assert(arr != null);
  assert(arr[0] === 42);
});

startTest("method_array_of_bools", function() {
  var obj = new TestObject(null);
  var arr = obj.methodArrayOfBools(null);
  assert(arr != null);
  assert(arr[0] === true);
  arr = obj.methodArrayOfBools(arr);
  assert(arr != null);
  assert(arr[0] === true);
});

startTest("method_array_of_doubles", function() {
  var obj = new TestObject(null);
  var arr = obj.methodArrayOfDoubles(null);
  assert(arr != null);
  assert(arr[0] === 42.0);
  arr = obj.methodArrayOfDoubles(arr);
  assert(arr != null);
  assert(arr[0] === 42.0);
});

startTest("method_array_of_enums", function() {
  var obj = new TestObject(null);
  var arr = obj.methodArrayOfEnums(null);
  assert(arr != null);
  assert(arr[0] === suite.Test.EnumEx.THIRD);
  arr = obj.methodArrayOfEnums(arr);
  assert(arr != null);
  assert(arr[0] === suite.Test.EnumEx.THIRD);
});

startTest("method_array_of_structs", function() {
  var obj = new TestObject(null);
  var arr = obj.methodArrayOfStructs(null);
  assert(arr != null);
  var s = arr[0];
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
  arr = obj.methodArrayOfStructs(arr);
  s = arr[0];
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
});

startTest("list_in_list_out", function() {
  var obj = new TestObject(null);
  var newList = obj.methodListWith42();
  assert(newList != null);
  var lis = obj.methodListInListOut(newList);
  assert(lis != null);
  // assert(lis == newList); // TODO: check same list pointer?
  assert(lis[0] === 42);
  assert(newList[0] === lis[0]);
});

startTest("method_list_of_objects", function() {
  var obj = new TestObject(null);
  var lis = obj.methodListOfObjects(null);
  assert(lis != null);
  assert(lis[0] != null);
  lis = obj.methodListOfObjects(lis);
  assert(lis != null);
  var v = lis[0];
  assert(v != null);
  // assert(v == obj); // TODO: check if same Eo object pointer?
  var expectedValue = 1234;
  v.methodIntegralInA(expectedValue);
  var actualValue = v.methodIntegralOutA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

// FIXME
// startTest("method_list_of_strings", function() {
//   var obj = new TestObject(null);
//   var lis = obj.methodListOfStrings(null);
//   assert(lis != null);
//   assert(lis[0] === "foo");
//   lis = obj.methodListOfStrings(lis);
//   assert(lis != null);
//   assert(lis[0] === "foo");
// });

startTest("method_list_of_ints", function() {
  var obj = new TestObject(null);
  var lis = obj.methodListOfInts(null);
  assert(lis != null);
  assert(lis[0] === 42);
  lis = obj.methodListOfInts(lis);
  assert(lis != null);
  assert(lis[0] === 42);
});

startTest("method_list_of_bools", function() {
  var obj = new TestObject(null);
  var lis = obj.methodListOfBools(null);
  assert(lis != null);
  assert(lis[0] === true);
  lis = obj.methodListOfBools(lis);
  assert(lis != null);
  assert(lis[0] === true);
});

startTest("method_list_of_doubles", function() {
  var obj = new TestObject(null);
  var lis = obj.methodListOfDoubles(null);
  assert(lis != null);
  assert(lis[0] === 42.0);
  lis = obj.methodListOfDoubles(lis);
  assert(lis != null);
  assert(lis[0] === 42.0);
});

startTest("method_list_of_enums", function() {
  var obj = new TestObject(null);
  var lis = obj.methodListOfEnums(null);
  assert(lis != null);
  assert(lis[0] === suite.Test.EnumEx.THIRD);
  lis = obj.methodListOfEnums(lis);
  assert(lis != null);
  assert(lis[0] === suite.Test.EnumEx.THIRD);
});

startTest("method_list_of_structs", function() {
  var obj = new TestObject(null);
  var lis = obj.methodListOfStructs(null);
  assert(lis != null);
  var s = lis[0];
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
  lis = obj.methodListOfStructs(lis);
  s = lis[0];
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
});

startTest("method_accessor_of_objects", function() {
  var obj = new TestObject(null);
  var acc = obj.methodAccessorOfObjects(null);
  assert(acc != null);
  assert(acc.get(0) != null);
  acc = obj.methodAccessorOfObjects(acc);
  assert(acc != null);
  var v = acc.get(0);
  assert(v != null);
  var expectedValue = 1234;
  v.methodIntegralInA(expectedValue);
  var actualValue = v.methodIntegralOutA();
  assert(actualValue == expectedValue, actualValue + " == " + expectedValue);
});

// FIXME
// startTest("method_accessor_of_strings", function() {
//   var obj = new TestObject(null);
//   var acc = obj.methodAccessorOfStrings(null);
//   assert(acc != null);
//   assert(acc.get(0) === "foo");
//   acc = obj.methodAccessorOfStrings(acc);
//   assert(acc != null);
//   assert(acc.get(0) === "foo");
// });

startTest("method_accessor_of_ints", function() {
  var obj = new TestObject(null);
  var acc = obj.methodAccessorOfInts(null);
  assert(acc != null);
  assert(acc.get(0) === 42);
  acc = obj.methodAccessorOfInts(acc);
  assert(acc != null);
  assert(acc.get(0) === 42);
});

startTest("method_accessor_of_bools", function() {
  var obj = new TestObject(null);
  var acc = obj.methodAccessorOfBools(null);
  assert(acc != null);
  assert(acc.get(0) === true);
  acc = obj.methodAccessorOfBools(acc);
  assert(acc != null);
  assert(acc.get(0) === true);
});

startTest("method_accessor_of_doubles", function() {
  var obj = new TestObject(null);
  var acc = obj.methodAccessorOfDoubles(null);
  assert(acc != null);
  assert(acc.get(0) === 42.0);
  acc = obj.methodAccessorOfDoubles(acc);
  assert(acc != null);
  assert(acc.get(0) === 42.0);
});

startTest("method_accessor_of_enums", function() {
  var obj = new TestObject(null);
  var acc = obj.methodAccessorOfEnums(null);
  assert(acc != null);
  assert(acc.get(0) === suite.Test.EnumEx.THIRD);
  acc = obj.methodAccessorOfEnums(acc);
  assert(acc != null);
  assert(acc.get(0) === suite.Test.EnumEx.THIRD);
});

startTest("method_accessor_of_structs", function() {
  var obj = new TestObject(null);
  var acc = obj.methodAccessorOfStructs(null);
  assert(acc != null);
  var s = acc.get(0);
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
  acc = obj.methodAccessorOfStructs(acc);
  assert(acc != null);
  s = acc.get(0);
  assert(s != null);
  assert(s.valueInt === 42);
  assert(s.valueEnum === suite.Test.EnumEx.THIRD);
});

// Combinations of complex types //

// FIXME
// startTest("method_array_of_arrays_of_ints", function() {
//   var obj = new TestObject(null);
//   var arr = obj.methodArrayOfArraysOfInts(null);
//   assert(arr != null);
//   var a = arr[0];
//   assert(a != null);
//   assert(a[0] === 42);
//   arr = obj.methodArrayOfArraysOfInts(arr);
//   assert(arr != null);
//   a = arr[0];
//   assert(a != null);
//   assert(a[0] === 42);
// });

// FIXME
// startTest("method_list_of_lists_of_ints", function() {
//   var obj = new TestObject(null);
//   var lis = obj.methodListOfListsOfInts(null);
//   assert(lis != null);
//   var l = lis[0];
//   assert(l != null);
//   assert(l[0] === 42);
//   lis = obj.methodListOfListsOfInts(lis);
//   assert(lis != null);
//   l = lis[0];
//   assert(l != null);
//   assert(l[0] === 42);
// });

// FIXME
// startTest("method_array_of_lists_of_ints", function() {
//   var obj = new TestObject(null);
//   var arr = obj.methodArrayOfListsOfInts(null);
//   assert(arr != null);
//   var l = arr[0];
//   assert(l != null);
//   assert(l[0] === 42);
//   arr = obj.methodArrayOfListsOfInts(arr);
//   assert(arr != null);
//   l = arr[0];
//   assert(l != null);
//   assert(l[0] === 42);
// });

// FIXME
// startTest("method_list_of_arrays_of_ints", function() {
//   var obj = new TestObject(null);
//   var lis = obj.methodListOfArraysOfInts(null);
//   assert(lis != null);
//   var a = lis[0];
//   assert(a != null);
//   assert(a[0] === 42);
//   lis = obj.methodListOfArraysOfInts(lis);
//   assert(lis != null);
//   a = lis[0];
//   assert(a != null);
//   assert(a[0] === 42);
// });

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
