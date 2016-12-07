
efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Calendar Day Selection Example");
win.setAutohide(true);

box = new efl.Efl.Ui.Box(win);
box.setHintWeight(1.0, 1.0);
win.setContent(box);
box.setVisible(true);

cal = new efl.Elm.Calendar(win);
cal.setHintWeight(1.0, 1.0);
cal.setHintAlign(-1.0, -1.0);
cal.setSelectMode(efl.Elm.Calendar.Select.Mode.NONE);
cal.setVisible(true);
box.packEnd(cal);

cal2 = new efl.Elm.Calendar(win);
cal2.setHintWeight(1.0, 1.0);
cal2.setHintAlign(-1.0, -1.0);
// selected_time_set uses tm* struct
//dateCurrent = new Date();
//dateSelected = dateCurrent.getSeconds() + 2 * 3600 * 24;
//cal2.selected_time_set(dateSelected);
cal2.setVisible(true);
box.packEnd(cal2);

win.setSize(240, 480);
win.setVisible(true);
