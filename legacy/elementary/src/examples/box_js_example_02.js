
var efl = require('efl')
var elm = require('elm')

_add_cb = function(){
    var btn = new elm.Elm.Button(win);
    btn.setText("elm.text", "I do nothing");
    bx.packEnd(btn);
    btn.setVisible(true);
}

_clear_cb = function(){
    bx.clear();
}

_unpack_cb = function(btn){
    bx.unpack(btn);
    btn.setColor(128, 64, 0, 128)
}

win = new elm.Elm.WinStandard(null);
win.setTitle("Box example");
win.setAutohide(true);

bg = new elm.Elm.Bg(win);
bg.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bg);
bg.setVisible(true);

bigbox = new elm.Elm.Box(win)
bigbox.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bigbox);

bx = new elm.Elm.Box(win)
bx.setSizeHintWeight(1.0, 1.0);
bx.setHorizontal(true);
bigbox.packEnd(bx);
bx.setVisible(true);

bt = new elm.Elm.Button(win);
bt.setText("elm.text", "Add");
bx.packEnd(bt);
bt.setVisible(true);
bt.on('clicked', _add_cb);

bt = new elm.Elm.Button(win);
bt.setText("elm.text", "Clear");
bx.packEnd(bt);
bt.setVisible(true);
bt.on('clicked', _clear_cb);

bx = new elm.Elm.Box(win)
bx.setSizeHintWeight(1.0, 1.0);
bx.setSizeHintAlign(-1.0, -1.0);
bx.setHorizontal(true);
bigbox.packEnd(bx);
bx.setVisible(true);

bt = new elm.Elm.Button(win);
bt.setText("elm.text", "Button 1");
bx.packEnd(bt);
bt.setSizeHintWeight(1.0, 1.0);
bt.setSizeHintAlign(-1.0, -1.0);
bt.setVisible(true);
bt.on('clicked', _unpack_cb);

bt = new elm.Elm.Button(win);
bt.setText("elm.text", "Button 2");
bx.packEnd(bt);
bt.setSizeHintWeight(1.0, 0.0);
bt.setSizeHintAlign(1.0, 0.5);
bt.setVisible(true);
bt.on('clicked', _unpack_cb);

bt = new elm.Elm.Button(win);
bt.setText("elm.text", "Button 3");
bx.packEnd(bt);
bt.setVisible(true);
bt.on('clicked', _unpack_cb);

bigbox.setVisible(true);

win.setSize(300, 320);
win.setVisible(true);

console.log("Going to wait now\n");
