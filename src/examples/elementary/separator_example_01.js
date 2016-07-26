
efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Separator");
win.setAutohide(true);

bg = new efl.Elm.Bg(win);
bg.setHintWeight(1.0, 1.0);
win.setSize(640, 480);
win.pack(bg);
bg.setVisible(true);

bx = new efl.Efl.Ui.Box(win);
bx.cast("Efl.Orientation").setOrientation(efl.Efl.Orient.HORIZONTAL);
bx.setHintWeight(1.0, 1.0);
win.pack(bx);
bx.setVisible(true);

rect = new efl.Efl.Canvas.Rectangle(win);
rect.setColor( 0, 255, 0, 255)
rect.setHintMin( 90, 200);
rect.setHintWeight(1.0, 1.0);
rect.setHintAlign(-1.0, -1.0);
rect.setVisible(true);
bx.packEnd(rect);

separator = new efl.Elm.Separator(win);
separator.cast("Efl.Orientation").setOrientation(efl.Efl.Orient.HORIZONTAL);
separator.setVisible(true);
bx.packEnd(separator);

rect2 = new efl.Efl.Canvas.Rectangle(win);
rect2.setColor( 0, 0, 255, 255);
rect2.setHintMin( 90, 200);
rect2.setHintWeight(1.0, 1.0);
rect2.setHintAlign(-1.0, -1.0);
rect2.setVisible(true);
bx.packEnd(rect2);

win.setVisible(true);
