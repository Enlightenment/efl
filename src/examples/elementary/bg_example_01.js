
var efl = require('efl');

win = new efl.Efl.Ui.Win(null, "Efl JS Example", efl.Efl.Ui.Win.Type.BASIC, "hw");
win.setText("Bg Plain");
win.setAutohide(true);

bg = new efl.Elm.Bg(win);
bg.setColor(255, 0,0,255);
bg.setHintWeight(1.0, 1.0);
win.setContent(bg);
bg.setVisible(true);

win.setSize(320,320);
win.setVisible(true);

