
elm = require('efl');
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Calendar Day Selection Example");
win.setAutohide(true);

box = new elm.Elm.Box(win);
box.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(box);
box.setVisible(true);

cal = new elm.Elm.Calendar(win);
cal.setSizeHintWeight(1.0, 1.0);
cal.setSizeHintAlign(-1.0, -1.0);
cal.setSelectMode(elm.Elm.Calendar.Select.Mode.NONE);
win.resizeObjectAdd(cal);
cal.setVisible(true);
box.packEnd(cal);

cal2 = new elm.Elm.Calendar(win);
cal2.setSizeHintWeight(1.0, 1.0);
cal2.setSizeHintAlign(-1.0, -1.0);
// selected_time_set uses tm* struct
//dateCurrent = new Date();
//dateSelected = dateCurrent.getSeconds() + 2 * 3600 * 24;
//cal2.selected_time_set(dateSelected);
cal2.setVisible(true);
box.packEnd(cal2);

win.setVisible(true);
