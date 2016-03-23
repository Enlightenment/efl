
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Bg Image");
win.setAutohide(true);

bg = new elm.Elm.Bg(win);
bg.setLoadSize(20,20);
bg.setOption(elm.Elm.Bg.Option.CENTER);

//TODO: elm_app_data_dir_get
bg.setFile('../../data/images/plant_01.jpg', null);
bg.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bg);
bg.setVisible(true);

win.setSize(320, 320);
win.setVisible(true);
