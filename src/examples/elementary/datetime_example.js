
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Datetime");
win.setAutohide(true);

bg = new elm.Elm.Bg(win);
bg.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bg);
bg.setVisible(true);

bx = new elm.Elm.Box(win);
bx.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bx);
bx.setVisible(true);

datetime = new elm.Elm.Datetime(bx);
datetime.setSizeHintWeight(1.0, 1.0);
datetime.setSizeHintAlign(-1.0, 0.5);
datetime.fieldVisibleSet(elm.Elm.Datetime.FieldType.HOUR, false);
datetime.fieldVisibleSet(elm.Elm.Datetime.FieldType.MINUTE, false);
datetime.fieldVisibleSet(elm.Elm.Datetime.FieldType.AMPM, false);
bx.packEnd(datetime);
datetime.setVisible(true);

datetime = new elm.Elm.Datetime(bx);
datetime.setSizeHintWeight(1.0, 1.0);
datetime.setSizeHintAlign(-1.0, 0.5);
datetime.fieldVisibleSet(elm.Elm.Datetime.FieldType.YEAR, false);
datetime.fieldVisibleSet(elm.Elm.Datetime.FieldType.MONTH, false);
datetime.fieldVisibleSet(elm.Elm.Datetime.FieldType.DATE, false);
bx.packEnd(datetime);
datetime.setVisible(true);

datetime = new elm.Elm.Datetime(bx);
datetime.setSizeHintWeight(1.0, 1.0);
datetime.setSizeHintAlign(-1.0, 0.5);
bx.packEnd(datetime);
datetime.setVisible(true);

win.setVisible(true);
