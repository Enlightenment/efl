
var efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText('title');
win.setAutohide(true);

bg = new efl.Elm.Bg(win);
bg.setHintWeight(1.0, 1.0);
win.setContent(bg);
bg.setVisible(true);

bx = new efl.Efl.Ui.Box(win);
bx.setHintWeight(1.0, 1.0);
win.setContent(bx);

entry = new efl.Elm.Label(win);
entry.setSize(100, 100);
entry.setText(null, "Texto");

console.log('Texto: ', entry.getText(null));

entry.setHintWeight(1.0, 1.0);
entry.setHintAlign(-1.0, -1.0);
entry.setWrapWidth(50);
bx.packEnd(entry);
entry.setVisible(true);

bx.setVisible(true);

win.setSize(300, 320);
win.setVisible(true);

console.log("Going to wait now\n");
