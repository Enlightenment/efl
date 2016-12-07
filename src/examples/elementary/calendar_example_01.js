
efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Calendar Creation Example");
win.setAutohide(true);

cal = new efl.Elm.Calendar(win);
cal.setHintWeight(1.0, 1.0);
win.setContent(cal);
win.setSize(240, 240);
cal.setVisible(true);

win.setVisible(true);
