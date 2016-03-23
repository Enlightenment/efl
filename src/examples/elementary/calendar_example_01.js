
efl = require('efl');
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Calendar Creation Example");
win.setAutohide(true);

cal = new elm.Elm.Calendar(win);
cal.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(cal);
cal.setVisible(true);

win.setVisible(true);
