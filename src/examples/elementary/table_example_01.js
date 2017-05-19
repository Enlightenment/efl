
efl = require('efl');

win = new efl.Efl.Ui.Win(null, "Efl JS Example", efl.Efl.Ui.Win.Type.BASIC, "hw");
win.setText("Table");
win.setAutohide(true);

table = new efl.Elm.Table(win);
win.resizeObjectAdd(table);
table.setVisible(true);
table.setPadding(5, 5);
table.setHomogeneous(true);

label = new efl.Elm.Label(win);
label.setText("elm.text", "label 0");
label.setVisible(true);
table.pack(label, 0, 0, 1, 1);

label = new efl.Elm.Label(win);
label.setText("elm.text", "label 1");
label.setVisible(true);
table.pack(label, 1, 0, 1, 1);

label = new efl.Elm.Label(win);
label.setText("elm.text", "label 2");
label.setVisible(true);
table.pack(label, 0, 1, 1, 1);

label = new efl.Elm.Label(win);
label.setText("elm.text", "label 3");
label.setVisible(true);
table.pack(label, 1, 1, 1, 1);

win.setVisible(true);
