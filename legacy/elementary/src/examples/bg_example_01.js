
var elm = require('elm')

win = new elm.Elm.WinStandard(null);
win.setTitle("Bg Plain");
win.setAutohide(true);

bg = new elm.Elm.Bg(win);
bg.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bg);
bg.setVisible(true);

win.setSize(320,320);
win.setVisible(true);

