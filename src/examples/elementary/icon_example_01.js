
efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Icon Plain");
win.setAutohide(true);

icon = new efl.Efl.Ui.Image(win);
//icon.order_lookup_set(ELM_ICON_LOOKUP_THEME_FDO); Ja eh o default
icon.setIcon("home");

var path, group;
r = icon.getFile(path, group);
path = r[0];
gruop = r[1];
console.log("path = " + path + ", group = " + group);

var name;
name = icon.getIcon();
console.log("name = " + name);

icon.setScaleType(efl.Efl.Ui.Image.ScaleType.NONE);

icon.setHintWeight(1.0, 1.0);
win.setContent(icon);
icon.setVisible(true);

win.setSize(320, 320);
win.setVisible(true);
