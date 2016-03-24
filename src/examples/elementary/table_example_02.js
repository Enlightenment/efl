
efl = require('efl');
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Table");
win.setAutohide(true);

table = new elm.Elm.Table(win);
win.resizeObjectAdd(table);
table.setVisible(true);
table.setHomogeneous(false);

rect = new efl.Evas.Rectangle(win);
rect.setColor( 255, 0, 0, 255)
rect.setSizeHintMin( 100, 50);
rect.setVisible(true)
table.pack(rect, 0, 0, 2, 1);

rect2 = new efl.Evas.Rectangle(win);
rect2.setColor( 0, 255, 0, 255)
rect2.setSizeHintMin( 50, 100);
rect2.setVisible(true)
table.pack(rect2, 0, 1, 1, 2);

rect3 = new efl.Evas.Rectangle(win);
rect3.setColor( 0, 0, 255, 255)
rect3.setSizeHintMin( 50, 50);
rect3.setVisible(true)
table.pack(rect3, 1, 1, 1, 1);

rect4 = new efl.Evas.Rectangle(win);
rect4.setColor( 255, 255, 0, 255)
rect4.setSizeHintMin( 50, 50);
rect4.setVisible(true)
table.pack(rect4, 1, 2, 1, 1);

win.setVisible(true);
