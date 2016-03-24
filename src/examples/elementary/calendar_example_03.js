
efl = require('efl');
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Calendar Min/Max Year Example");
win.setAutohide(true);

cal = new elm.Elm.Calendar(win);
cal.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(cal);
cal.setMinMaxYear(2020,2022);
cal.setVisible(true);

win.setVisible(true);
