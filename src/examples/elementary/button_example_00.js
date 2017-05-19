
efl = require('efl');

win = new efl.Efl.Ui.Win(null, "Title", efl.Efl.Ui.Win.Type.BASIC, "hw");
win.setText("Hello, World!");
win.setAutohide(true);

btn = new efl.Efl.Ui.Button(win);
btn.setText("Good-Bye, World!");
btn.setSize(120, 30);
btn.setPosition(60, 15);
btn.setHintWeight(1.0, 1.0);
btn.setHintAlign(1.0, 1.0);
btn.setVisible(true);

btn.on('clicked', function () { console.log('clicked'); } );

win.setSize(240, 60);
win.setVisible(true);

