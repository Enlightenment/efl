
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Table");
win.setAutohide(true);

table = new elm.Elm.Table(win);
win.resizeObjectAdd(table);
table.setVisible(true);
table.setPadding(5, 5);
table.setHomogeneous(true);

label = new elm.Elm.Label(win);
label.setText("elm.text", "label 0");
label.setVisible(true);
table.pack(label, 0, 0, 1, 1);

label = new elm.Elm.Label(win);
label.setText("elm.text", "label 1");
label.setVisible(true);
table.pack(label, 1, 0, 1, 1);

label = new elm.Elm.Label(win);
label.setText("elm.text", "label 2");
label.setVisible(true);
table.pack(label, 0, 1, 1, 1);

label = new elm.Elm.Label(win);
label.setText("elm.text", "label 3");
label.setVisible(true);
table.pack(label, 1, 1, 1, 1);

win.setVisible(true);
