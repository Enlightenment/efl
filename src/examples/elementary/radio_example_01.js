
efl = require('efl');

_cb = function(obj){
   console.log(obj.getText("elm.text"), "state value:", obj.getStateValue());
}

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Radio");
win.setAutohide(true);

bx = new efl.Efl.Ui.Box(win);
bx.cast("Efl.Orientation").setOrientation(efl.Efl.Orient.HORIZONTAL);
bx.setHintWeight(1.0, 1.0);
win.setContent(bx);
bx.setVisible(true);

radio_g = new efl.Elm.Radio(win);
radio_g.setText("elm.text", "Radio 1");
radio_g.setStateValue(1);
radio_g.setValue(1);
ic = new efl.Efl.Ui.Image(win);
ic.setIcon("home");
radio_g.part("icon").cast("Efl.Container").setContent(ic);
bx.packEnd(radio_g);
radio_g.setHintWeight(1.0, 1.0);
radio_g.setHintAlign(-1.0, -1.0);
radio_g.on('changed', _cb);
radio_g.setVisible(true);

radio = new efl.Elm.Radio(win);
radio.setText("elm.text", "Radio 2");
radio.setStateValue(2);
radio.addGroup(radio_g);
ic = new efl.Efl.Ui.Image(win);
ic.setIcon("file");
radio.part("icon").cast("Efl.Container").setContent(ic);
bx.packEnd(radio);
radio.setHintWeight(1.0, 1.0);
radio.setHintAlign(-1.0, -1.0);
radio.on('changed', _cb);
radio.setVisible(true);

radio = new efl.Elm.Radio(win);
radio.setText("elm.text", "Radio 3");
radio.setStateValue(3);
radio.addGroup(radio_g);
bx.packEnd(radio);
radio.setHintWeight(1.0, 1.0);
radio.setHintAlign(-1.0, -1.0);
radio.on('changed', _cb);
radio.setVisible(true);

radio = new efl.Elm.Radio(win);
radio.setText("elm.text", "Radio 4");
radio.setStateValue(4);
radio.addGroup(radio_g);
bx.packEnd(radio);
radio.setHintWeight(1.0, 1.0);
radio.setHintAlign(-1.0, -1.0);
radio.on('changed', _cb);
radio.setVisible(true);


win.setVisible(true);
