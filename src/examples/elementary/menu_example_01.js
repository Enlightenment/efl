efl = require('efl');
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Menu");
win.setAutohide(true);

rect = new efl.Evas.Rectangle(win);
win.resizeObjectAdd(rect);
rect.setSizeHintMin( 0, 0);
rect.setColor( 0, 0, 0, 0);
rect.setVisible(true);

menu = new elm.Elm.Menu(win);
menu.itemAdd(null, null, "first item", null, null); // item_add nao implementado
menu_it = menu.itemAdd(null, "mail-reply-all", "second intem", null, null);

menu.itemAdd(menu_it, "object-rotate-left", "menu 1", null, null);
button = new elm.Elm.Button(win);
button.setText("elm.text", "button - delete items");
menu_it1 = menu.itemAdd(menu_it, null, null, null, null);
menu_it1.setPartContent(null, button);

//button.event_clicked(del_it);

menu.itemSeparatorAdd(menu_it);
menu.itemAdd(menu_it, null, "third item", null, null);
menu.itemAdd(menu_it, null, "fourth item", null, null);
menu.itemAdd(menu_it, "window-new", "sub menu", null, null);

menu_it = menu.itemAdd(null, null, "third item", null, null);
menu_it.setDisabled(true);

menu.setVisible(true);
rect.on('mouse_down', function(){menu.setVisible(true)});
win.setSize(250, 350);
win.setVisible(true);
