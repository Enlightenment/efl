
efl = require('efl');
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Separator");
win.setAutohide(true);

bg = new elm.Elm.Bg(win);
bg.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bg);
bg.setVisible(true);

bx = new elm.Elm.Box(win);
bx.setHorizontal(true);
bx.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bx);
bx.setVisible(true);

rect = new efl.Evas.Rectangle(win);
rect.setColor( 0, 255, 0, 255)
rect.setSizeHintMin( 90, 200);
rect.setSizeHintWeight(1.0, 1.0);
rect.setSizeHintAlign(-1.0, -1.0);
rect.setVisible(true);
bx.packEnd(rect);

separator = new elm.Elm.Separator(win);
separator.setHorizontal(true);
separator.setVisible(true);
bx.packEnd(separator);

rect2 = new efl.Evas.Rectangle(win);
rect2.setColor( 0, 0, 255, 255);
rect2.setSizeHintMin( 90, 200);
rect2.setSizeHintWeight(1.0, 1.0);
rect2.setSizeHintAlign(-1.0, -1.0);
rect2.setVisible(true);
bx.packEnd(rect2);

win.setVisible(true);
