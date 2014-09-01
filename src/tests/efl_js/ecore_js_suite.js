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
    exit = efl.ecore_mainloop_quit;
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

start_test("timers", function () {
    var p = 2.5;
    efl.ecore_timer_precision_set(p);
    assert(abs(efl.ecore_timer_precision_get() - p) < TOLERANCE);
    
    p = 0.5;
    efl.ecore_timer_precision_set(p);
    assert(abs(efl.ecore_timer_precision_get() - p) < TOLERANCE);
    
    var dump = efl.ecore_timer_dump();
    assert(typeof(dump) === 'string');
    print_info("### BEGINING of Timer dump: ###");
    print_info(dump);
    print_info("### END of Timer dump ###");

    var ncalls = 0;

    captured = false;
    efl.ecore_timer_add(1, function() {
        print_info('ecore_timer_add handler');
        ++ncalls;
        if (ncalls != 4)
            return true;
        
        captured = true;
        efl.ecore_job_add(efl.ecore_mainloop_quit);
        return false;
    });

    efl.ecore_mainloop_begin();
    assert(captured);

    ncalls = 0;
    captured = false;

    efl.ecore_timer_loop_add(1, function() {
        ++ncalls;
        if (ncalls != 4)
            return true;

        captured = true;
        efl.ecore_job_add(efl.ecore_mainloop_quit);
        return false;
    });

    efl.ecore_mainloop_begin();
    assert(captured);

    captured = false;

    var timer = efl.ecore_timer_add(1, function() {
        captured = true;
        return false;
    });

    assert(timer.freeze_get() === false);

    timer.freeze();

    assert(timer.freeze_get() === true);

    timer.thaw();

    assert(timer.freeze_get() === false);

    timer.del();

    efl.ecore_timer_add(2, function() {
        efl.ecore_job_add(efl.ecore_mainloop_quit);
        return false;
    });

    efl.ecore_mainloop_begin();
    assert(captured === false);
});

// Ecore event
start_test("ecore event", function () {
    var myevent = efl.ecore_event_type_new();

    captured = [0, 0, 0]

    var handler1 = efl.ecore_event_handler_add(myevent, function(event) {
        assert(efl.ecore_event_current_type_get() === myevent);
        assert(event === myevent);
        captured[0] += 1;
        return efl.ECORE_CALLBACK_PASS_ON;
    });

    var handler2 = efl.ecore_event_handler_add(myevent, function(event) {
        assert(efl.ecore_event_current_type_get() === myevent);
        assert(event === myevent);
        captured[1] += 1;
        return efl.ECORE_CALLBACK_DONE;
    });

    var handler3 = efl.ecore_event_handler_add(myevent, function(event) {
        assert(efl.ecore_event_current_type_get() === myevent);
        assert(event === myevent);
        captured[2] += 1;
        return efl.ECORE_CALLBACK_DONE;
    });

    efl.ecore_timer_add(1, function() {
        efl.ecore_event_add(myevent);
        assert(captured[0] === 0 && captured[1] === 0 && captured[2] === 0);
        efl.ecore_timer_add(1, function() {
            assert(captured[0] === 1 && captured[1] === 1 && captured[2] === 0);
            handler1.del();
            efl.ecore_event_add(myevent);
            efl.ecore_event_add(myevent);
            efl.ecore_event_add(myevent).del();
            efl.ecore_timer_add(1, function() {
                assert(captured[0] === 1 && captured[1] === 3 && captured[2] === 0);
                efl.ecore_mainloop_quit();
            });
        });
    });

    efl.ecore_mainloop_begin();

    efl.ecore_event_add(myevent);
    efl.ecore_event_add(myevent);
    efl.ecore_event_add(myevent);

    var filter = efl.ecore_event_filter_add(function() {
        return {count: 0};
    }, function(loop_data, event) {
        assert(event === myevent);
        var c = loop_data.count;
        ++loop_data.count;
        return c != 0;
    }, function(loop_data) {});
    
    efl.ecore_timer_add(1, efl.ecore_mainloop_quit);
    efl.ecore_mainloop_begin();
    assert(captured[1] === 5);
    filter.del();

    efl.ecore_event_add(myevent);
    efl.ecore_event_add(myevent);
    efl.ecore_event_add(myevent);
    
    efl.ecore_timer_add(1, efl.ecore_mainloop_quit);
    efl.ecore_mainloop_begin();
    assert(captured[1] === 8);

    handler2.del();
    handler3.del();
});

    // Ecore job
start_test("ecore jobs", function () {
    captured = false;

    efl.ecore_job_add(function() {
        captured = true;
        efl.ecore_mainloop_quit();
    });

    assert(captured === false);
    efl.ecore_mainloop_begin();
    assert(captured === true);

    captured = false;
    var job = efl.ecore_job_add(function() {
        captured = true;
    });
    efl.ecore_job_add(efl.ecore_mainloop_quit);
    job.del();
    efl.ecore_mainloop_begin();
    assert(captured === false);
});

start_test("ecore idle", function () {
    // Ecore idle
    var counter = 1;
    captured = [0, 0, 0, 0, 0];

    efl.ecore_idler_add(function() {
        print_info('ecore idle handler 1');
        captured[0] = counter;
        counter += 1;
        efl.ecore_job_add(function() { print_info('ecore job handler 1'); });
        return efl.ECORE_CALLBACK_DONE;
    });

    print_info('ecore idle 1');

    efl.ecore_idle_enterer_add(function() {
        print_info('ecore idle handler 2');
        captured[1] = counter;
        counter += 1;
        return efl.ECORE_CALLBACK_DONE;
    });

    print_info('ecore idle 2');

    efl.ecore_idle_enterer_add(function() {
        print_info('ecore idle handler 3');
        captured[2] = counter;
        counter += 1;
        return efl.ECORE_CALLBACK_DONE;
    });

    print_info('ecore idle 3');

    efl.ecore_idle_enterer_before_add(function() {
        print_info('ecore idle handler 4');
        captured[3] = counter;
        counter += 1;
        return efl.ECORE_CALLBACK_DONE;
    });

    print_info('ecore idle 4');

    efl.ecore_idle_exiter_add(function() {
        print_info('ecore idle handler 5');
        captured[4] = counter;
        counter += 1;
        efl.ecore_mainloop_quit();
        return efl.ECORE_CALLBACK_DONE;
    });

    print_info('ecore idle 5');

    efl.ecore_mainloop_begin();

    print_info('ecore idle 6');

    assert(captured[0] === 4, "ecore_idler_add test");
    assert(captured[1] === 2, "ecore_idle_enterer_add test");
    assert(captured[2] === 3, "ecore_idle_enterer_add test two");
    assert(captured[3] === 1, "ecore_idle_enterer_before_add test");
    assert(captured[4] === 5, "ecore_idle_exiter_add test");
});

// Ecore animator
start_test("ecore animator", function () {
    efl.ecore_animator_frametime_set(1);
    assert(efl.ecore_animator_frametime_get() === 1);
    efl.ecore_animator_frametime_set(1 / 50);
    assert(efl.ecore_animator_frametime_get() === (1 / 50));

    assert(efl.ecore_animator_pos_map(0.5, efl.ECORE_POS_MAP_LINEAR, 0, 0)
           === 0.5);

    efl.ecore_animator_source_set(efl.ECORE_ANIMATOR_SOURCE_CUSTOM);
    assert(efl.ecore_animator_source_get()
           === efl.ECORE_ANIMATOR_SOURCE_CUSTOM);
    efl.ecore_animator_source_set(efl.ECORE_ANIMATOR_SOURCE_TIMER);
    assert(efl.ecore_animator_source_get() === efl.ECORE_ANIMATOR_SOURCE_TIMER);
});
    // Ecore poller
start_test("ecore poller", function () {
    efl.ecore_poller_poll_interval_set(efl.ECORE_POLLER_CORE, 42);
    assert(efl.ecore_poller_poll_interval_get(efl.ECORE_POLLER_CORE) === 42);
    efl.ecore_poller_poll_interval_set(efl.ECORE_POLLER_CORE, 2);
    assert(efl.ecore_poller_poll_interval_get(efl.ECORE_POLLER_CORE) === 2);
});

start_test("ecore throttle", function () {
    // Ecore throttle

    efl.ecore_throttle_adjust(3);
    assert(efl.ecore_throttle_get() === 3);
    efl.ecore_throttle_adjust(-3);
    assert(efl.ecore_throttle_get() === 0);
});

// Ecore file
start_test("ecore file", function () {
    assert(typeof(efl.ECORE_FILE_EVENT_NONE) === 'number');
    assert(typeof(efl.ECORE_FILE_EVENT_CREATED_FILE) === 'number');
    assert(typeof(efl.ECORE_FILE_EVENT_CREATED_DIRECTORY) === 'number');
    assert(typeof(efl.ECORE_FILE_EVENT_DELETED_FILE) === 'number');
    assert(typeof(efl.ECORE_FILE_EVENT_DELETED_DIRECTORY) === 'number');
    assert(typeof(efl.ECORE_FILE_EVENT_DELETED_SELF) === 'number');
    assert(typeof(efl.ECORE_FILE_EVENT_MODIFIED) === 'number');
    assert(typeof(efl.ECORE_FILE_EVENT_CLOSED) === 'number');
    assert(typeof(efl.ECORE_FILE_PROGRESS_CONTINUE) === 'number');
    assert(typeof(efl.ECORE_FILE_PROGRESS_ABORT) === 'number');

    var buf = efl.environment_tmp() + '/.ecore_js_test.tmp.XXXXXXXXXXXX';
    print('buf ', buf)
    var name = efl.mkstemp(buf);

    print('name of file ', name);
    assert(efl.ecore_file_remove(name) === true, 'Cannot remove temporary file');

    assert(typeof(efl.ecore_file_recursive_rm(name)) === 'boolean', 'fb0');
    assert(efl.ecore_file_exists('/') === true, "Root (/) doesn't exist?");
    assert(efl.ecore_file_is_dir('/') === true, 'fb1');
    assert((efl.ecore_file_mod_time('/') instanceof Date) === true, 'fb2');
    assert(typeof(efl.ecore_file_size('/')) === 'number', 'fb3');
    assert(efl.ecore_file_mkdir(name) === true, 'fb4');
    assert(efl.ecore_file_mkdir(name) === false, 'fb5');
    assert(efl.ecore_file_rmdir(name) === true, 'fb6');
    assert(efl.ecore_file_mkdirs([name]) === 1, 'fb7');
    assert(efl.ecore_file_mksubdirs(name, ['0', '1']) === 2, 'fb8');
    assert(efl.ecore_file_symlink('0', name + '/2') === true, 'fb9');
    assert(efl.ecore_file_readlink(name + '/2') === '0', 'fb23');
    assert(efl.ecore_file_unlink(name + '/2') === true, 'fb10');
    assert(efl.ecore_file_remove(name + '/1') === true, 'fb11');
    assert(efl.ecore_file_recursive_rm(name) === true, 'fb12');
    assert(efl.ecore_file_mkpath(name + '/0/1') === true, 'fb13');
    assert(efl.ecore_file_mkpaths([name + '/0/1', name + '/1/2']) === 2, 'fb14');
    assert(efl.ecore_file_cp(name + '/0/1', name + '/2') === true, 'fb15');
    assert(efl.ecore_file_mv(name + '/1/2', name + '/2/0') === false, 'fb16');
    assert(typeof(efl.ecore_file_realpath(name)) === 'string', 'fb17');
    print('file_get ', efl.ecore_file_file_get(name));
    assert(efl.ecore_file_file_get(name) === name.substr(name.lastIndexOf('/')+1
                                                         , name.length - 1 - name.lastIndexOf('/')), 'fb18');
    assert(efl.ecore_file_dir_get(name + '/2') === name, 'fb19');
    assert(efl.ecore_file_can_read(name) === true, 'fb20');
    assert(efl.ecore_file_can_write(name) === true, 'fb21');
    assert(efl.ecore_file_can_exec(name) === true, 'fb22');
    var ls = efl.ecore_file_ls(name);
    assert(ls.length === 3, 'fb23');
    assert(ls[0] === '0', 'fb24');
    assert(ls[1] === '1', 'fb25');
    assert(ls[2] === '2', 'fb26');
    assert(typeof(efl.ecore_file_app_exe_get('auidadsfas')) === 'string', 'fb27');
    assert(efl.ecore_file_escape_name('/\\') === '/\\\\', 'fb28');
    assert(efl.ecore_file_strip_ext('test.abc') === 'test', 'fb29');
    assert(efl.ecore_file_dir_is_empty(name) === 0, 'fb30');
    assert(efl.ecore_file_download_protocol_available('file://') === true, 'fb31');
    print('exists ', name, '? ', efl.ecore_file_path_dir_exists(name));
    assert(efl.ecore_file_path_dir_exists(name) === true, 'fb32');
    assert(efl.ecore_file_app_installed(' dsuyabu UBYOS') === false, 'fb33');
    assert(Array.isArray(efl.ecore_file_app_list()) === true, 'fb34');
    efl.ecore_file_monitor_add(name, function() { print('empty monitor called'); }).del();
    function cb(em, e, path) {
        print('cb called');
        assert(em.path_get() === name, 'fb38');
        assert(e === efl.ECORE_FILE_EVENT_CREATED_DIRECTORY, 'fb36');
        assert(path === name + '/909', 'fb37');
        em.del();
        efl.ecore_mainloop_quit();
    };
    var monitor_job = efl.ecore_file_monitor_add(name, cb);
    assert(efl.ecore_file_mkdir(name + '/909') === true, 'fb35');

    efl.ecore_mainloop_begin();
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
