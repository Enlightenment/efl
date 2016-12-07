efl = require('efl');

//var weekdays = ["S", "M", "T", "W", "T", "F", "S"];

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Calendar Layout Formatting Example");
win.setAutohide(true);

cal = new efl.Elm.Calendar(win);
cal.setHintWeight(1.0, 1.0);

win.setContent(cal);
win.setSize(240, 240);
//cal.format_function_set(funtion);
//cal.weekdays_names_set(weekdays);

cal.setVisible(true);
win.setVisible(true);
