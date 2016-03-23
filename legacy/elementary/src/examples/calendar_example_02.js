elm = require('elm');

//var weekdays = ["S", "M", "T", "W", "T", "F", "S"];

win = new elm.Elm.WinStandard(null);
win.setTitle("Calendar Layout Formatting Example");
win.setAutohide(true);

cal = new elm.Elm.Calendar(win);
cal.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(cal);

//cal.format_function_set(funtion);
//cal.weekdays_names_set(weekdays);

cal.setVisible(true);
win.setVisible(true);
