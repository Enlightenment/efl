
efl = require('efl');
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Hello, World!");
win.setAutohide(true);

btn = new elm.Elm.Button(win);
btn.setText(null, "Good-Bye, World!");
btn.setSize(120, 30);
btn.setPosition(60, 15);
btn.setSizeHintWeight(1.0, 1.0);
btn.setSizeHintAlign(1.0, 1.0);
btn.setVisible(true);

btn.on('clicked', function () { console.log('clicked'); } );

win.setSize(240, 60);
win.setVisible(true);

