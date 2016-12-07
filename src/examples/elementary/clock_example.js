efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Clock Example");
win.setAutohide(true);
win.setSize(480, 240);

bx = new efl.Efl.Ui.Box(win);
bx.setHintWeight(1.0, 1.0);
win.setContent(bx);
bx.setVisible(true);
bx.setOrientation(efl.Efl.Orient.VERTICAL);

ck = new efl.Elm.Clock(win);
bx.packEnd(ck);
ck.setVisible(true);

ck2 = new efl.Elm.Clock(win);
ck2.setShowAmPm(true);
bx.packEnd(ck2);
ck2.setVisible(true);

ck3 = new efl.Elm.Clock(win);
ck3.setShowSeconds(true);
ck3.setTime(10, 11, 12);
bx.packEnd(ck3);
ck3.setVisible(true);

ck4 = new efl.Elm.Clock(win);
ck4.setEdit(true);
ck4.setShowSeconds(true);
ck4.setShowAmPm(true);
ck4.setTime(10, 11, 12);
bx.packEnd(ck4);
ck4.setVisible(true);

ck5 = new efl.Elm.Clock(win);
ck5.setShowSeconds(true);
ck5.setEdit(true);
digedit = efl.Elm.Clock.EditMode.HOUR_UNIT | efl.Elm.Clock.EditMode.MIN_UNIT | efl.Elm.Clock.EditMode.SEC_UNIT;
ck5.setEditMode(digedit);
bx.packEnd(ck5);
ck5.setVisible(true);


win.setVisible(true);
