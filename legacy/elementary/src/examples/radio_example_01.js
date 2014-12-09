
elm = require('elm');

_cb = function(obj){
   console.log(obj.textGet("elm.text"), "state value:", obj.getStateValue());
}

win = new elm.Elm.WinStandard(null);
win.setTitle("Radio");
win.setAutohide(true);

bx = new elm.Elm.Box(win);
bx.setHorizontal(true);
bx.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bx);
bx.setVisible(true);

radio_g = new elm.Elm.Radio(win);
radio_g.textSet("elm.text", "Radio 1");
radio_g.setStateValue(1);
radio_g.setValue(1);
ic = new elm.Elm.Icon(win);
ic.setStandard("home");
radio_g.contentSet("icon", ic);
bx.packEnd(radio_g);
radio_g.setSizeHintWeight(1.0, 1.0);
radio_g.setSizeHintAlign(-1.0, -1.0);
radio_g.on('changed', _cb);
radio_g.setVisible(true);

radio = new elm.Elm.Radio(win);
radio.textSet("elm.text", "Radio 2");
radio.setStateValue(2);
radio.groupAdd(radio_g);
ic = new elm.Elm.Icon(win);
ic.setStandard("file");
radio.contentSet("icon", ic);
bx.packEnd(radio);
radio.setSizeHintWeight(1.0, 1.0);
radio.setSizeHintAlign(-1.0, -1.0);
radio.on('changed', _cb);
radio.setVisible(true);

radio = new elm.Elm.Radio(win);
radio.textSet("elm.text", "Radio 3");
radio.setStateValue(3);
radio.groupAdd(radio_g);
bx.packEnd(radio);
radio.setSizeHintWeight(1.0, 1.0);
radio.setSizeHintAlign(-1.0, -1.0);
radio.on('changed', _cb);
radio.setVisible(true);

radio = new elm.Elm.Radio(win);
radio.textSet("elm.text", "Radio 4");
radio.setStateValue(4);
radio.groupAdd(radio_g);
bx.packEnd(radio);
radio.setSizeHintWeight(1.0, 1.0);
radio.setSizeHintAlign(-1.0, -1.0);
radio.on('changed', _cb);
radio.setVisible(true);


win.setVisible(true);
