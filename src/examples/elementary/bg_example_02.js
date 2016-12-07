
var efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Bg Plain");
win.setAutohide(true);

bg = new efl.Elm.Bg(win);
bg.setLoadSize(20,20);
bg.setOption(efl.Elm.Bg.Option.CENTER);

//TODO: elm_app_data_dir_get
bg.setFile(__dirname + '/../../../data/elementary/images/plant_01.jpg', null);
bg.setHintWeight(1.0, 1.0);
win.setContent(bg);
bg.setVisible(true);

win.setSize(320, 320);
win.setVisible(true);
