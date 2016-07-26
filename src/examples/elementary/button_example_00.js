
efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Hello, World!");
win.setAutohide(true);

btn = new efl.Elm.Button(win);
btn.setText(null, "Good-Bye, World!");
btn.setSize(120, 30);
btn.setPosition(60, 15);
btn.setHintWeight(1.0, 1.0);
btn.setHintAlign(1.0, 1.0);
btn.setVisible(true);

btn.on('clicked', function () { console.log('clicked'); } );

win.setSize(240, 60);
win.setVisible(true);

