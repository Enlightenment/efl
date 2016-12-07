
var efl = require('efl')

_add_cb = function(){
    var btn = new efl.Elm.Button(win);
    btn.setText("elm.text", "I do nothing");
    bx.packEnd(btn);
    btn.setVisible(true);
}

_clear_cb = function(){
    bx.clearPack();
}

_unpack_cb = function(btn){
    bx.unpack(btn);
    btn.setColor(128, 64, 0, 128)
}

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Box example");
win.setAutohide(true);

bigbox = new efl.Efl.Ui.Box(win)
bigbox.setHintWeight(1.0, 1.0);
win.setContent(bigbox);

bx = new efl.Efl.Ui.Box(win);
bx.setHintWeight(1.0, 1.0);
bx_orient = bx.cast("Efl.Orientation");
bx_orient.setOrientation(efl.Efl.Orient.VERTICAL);
bigbox.packEnd(bx);
bx.setVisible(true);

bt = new efl.Elm.Button(win);
bt.setText("elm.text", "Add");
bx.packEnd(bt);
bt.setVisible(true);
bt.on('clicked', _add_cb);

bt = new efl.Elm.Button(win);
bt.setText("elm.text", "Clear");
bx.packEnd(bt);
bt.setVisible(true);
bt.on('clicked', _clear_cb);

bx = new efl.Efl.Ui.Box(win)
bx.setHintWeight(1.0, 1.0);
bx.setHintAlign(-1.0, -1.0);
bx_orient = bx.cast("Efl.Orientation");
bx_orient.setOrientation(efl.Efl.Orient.HORIZONTAL);
bigbox.packEnd(bx);
bx.setVisible(true);

bt = new efl.Elm.Button(win);
bt.setText("elm.text", "Button 1");
bx.packEnd(bt);
bt.setHintWeight(1.0, 1.0);
bt.setHintAlign(-1.0, -1.0);
bt.setVisible(true);
bt.on('clicked', _unpack_cb);

bt = new efl.Elm.Button(win);
bt.setText("elm.text", "Button 2");
bx.packEnd(bt);
bt.setHintWeight(1.0, 0.0);
bt.setHintAlign(1.0, 0.5);
bt.setVisible(true);
bt.on('clicked', _unpack_cb);

bt = new efl.Elm.Button(win);
bt.setText("elm.text", "Button 3");
bx.packEnd(bt);
bt.setVisible(true);
bt.on('clicked', _unpack_cb);

bigbox.setVisible(true);

win.setSize(300, 320);
win.setVisible(true);

console.log("Going to wait now\n");
