elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Clock Example");
win.setAutohide(true);

bx = new elm.Elm.Box(win);
bx.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bx);
bx.setVisible(true);

ck = new elm.Elm.Clock(win);
bx.packEnd(ck);
ck.setVisible(true);

ck2 = new elm.Elm.Clock(win);
ck2.setShowAmPm(true);
bx.packEnd(ck2);
ck2.setVisible(true);

ck3 = new elm.Elm.Clock(win);
ck3.setShowSeconds(true);
ck3.setTime(10, 11, 12);
bx.packEnd(ck3);
ck3.setVisible(true);

ck4 = new elm.Elm.Clock(win);
ck4.setEdit(true);
ck4.setShowSeconds(true);
ck4.setShowAmPm(true);
ck4.setTime(10, 11, 12);
bx.packEnd(ck4);
ck4.setVisible(true);

ck5 = new elm.Elm.Clock(win);
ck5.setShowSeconds(true);
ck5.setEdit(true);
digedit = elm.Elm.Clock.EditMode.HOUR_UNIT | elm.Elm.Clock.EditMode.MIN_UNIT | elm.Elm.Clock.EditMode.SEC_UNIT;
ck5.setEditMode(digedit);
bx.packEnd(ck5);
ck5.setVisible(true);

win.setVisible(true);
