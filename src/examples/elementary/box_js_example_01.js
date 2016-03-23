
var elm = require('elm')

win = new elm.Elm.WinStandard(null);
win.setTitle('title');
win.setAutohide(true);

bg = new elm.Elm.Bg(win);
bg.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bg);
bg.setVisible(true);

bx = new elm.Elm.Box(win);
bx.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bx);

entry = new elm.Elm.Label(win);
entry.setSize(100, 100);
entry.setText(null, "Texto");

console.log('Texto: ', entry.getText(null));

entry.setSizeHintWeight(1.0, 1.0);
entry.setSizeHintAlign(-1.0, -1.0);
entry.setWrapWidth(50);
bx.packEnd(entry);
entry.setVisible(true);

bx.setVisible(true);

win.setSize(300, 320);
win.setVisible(true);

console.log("Going to wait now\n");
