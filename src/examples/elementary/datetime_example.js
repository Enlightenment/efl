
efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Datetime");
win.setAutohide(true);
win.setSize(320,240);

bg = new efl.Elm.Bg(win);
bg.setHintWeight(1.0, 1.0);
win.pack(bg);
bg.setVisible(true);

bx = new efl.Efl.Ui.Box(win);
bx.setHintWeight(1.0, 1.0);
win.pack(bx);
bx.setVisible(true);

datetime = new efl.Elm.Datetime(bx);
datetime.setHintWeight(1.0, 1.0);
datetime.setHintAlign(-1.0, 0.5);
datetime.setFieldVisible(efl.Elm.Datetime.FieldType.HOUR, false);
datetime.setFieldVisible(efl.Elm.Datetime.FieldType.MINUTE, false);
datetime.setFieldVisible(efl.Elm.Datetime.FieldType.AMPM, false);
bx.packEnd(datetime);
datetime.setVisible(true);

datetime = new efl.Elm.Datetime(bx);
datetime.setHintWeight(1.0, 1.0);
datetime.setHintAlign(-1.0, 0.5);
datetime.setFieldVisible(efl.Elm.Datetime.FieldType.YEAR, false);
datetime.setFieldVisible(efl.Elm.Datetime.FieldType.MONTH, false);
datetime.setFieldVisible(efl.Elm.Datetime.FieldType.DATE, false);
bx.packEnd(datetime);
datetime.setVisible(true);

datetime = new efl.Elm.Datetime(bx);
datetime.setHintWeight(1.0, 1.0);
datetime.setHintAlign(-1.0, 0.5);
bx.packEnd(datetime);
datetime.setVisible(true);

win.setVisible(true);
