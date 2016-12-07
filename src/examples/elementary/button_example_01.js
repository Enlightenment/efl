efl = require('efl');

function setIcon(widget, icon) {
    container = widget.part("icon").cast("Efl.Container");
    container.setContent(icon);
}

_btn_options_cb = function(_btn) {
     var text = _btn.getText("elm.text");
     var lbl = text.split(": ");

     var t = parseFloat(lbl[1]);
     if (lbl[0] === "Initial")
     {
         console.log ("Initial: " + lbl[1]);
         up.setAutorepeatInitialTimeout(t);
         down.setAutorepeatInitialTimeout(t);
         left.setAutorepeatInitialTimeout(t);
         right.setAutorepeatInitialTimeout(t);
     }
     else if (lbl[0] === "Gap")
     {
         console.log ("Gap: " + lbl[1]);
         up.setAutorepeatGapTimeout(t);
         down.setAutorepeatGapTimeout(t);
         left.setAutorepeatGapTimeout(t);
         right.setAutorepeatGapTimeout(t);
     }
}

_btn_cursors_move = function(_btn) {
   var size = mid.getSizeHintAlign();
   var ax = size[0];
   var ay = size[1];
   console.log ("cursor move: ", size[0], size[1]);

   if (!icon_still) {
       var icon = new efl.Efl.Ui.Image(mid);
       var icon_still_p = mid.contentUnset("icon");
       if (icon_still_p) {
           icon_still_p.setVisible(false);
        }
      icon.setIcon("chat");
      setIcon(mid, icon);
   }

   if (_btn.getPosition()[1] == up.getPosition()[1]) {
        ay -= 0.05;
        if (ay < 0.0) ay = 0.0;
   }
   else if (_btn.getPosition()[1] == down.getPosition()[1]) {
       ay += 0.05;
       if (ay > 1.0) ay = 1.0;
   }
   else if (_btn.getPosition()[0] == left.getPosition()[0]) {
        ax -= 0.05;
        if (ax < 0.0) ax = 0.0;
   }
   else if (_btn.getPosition()[0] == right.getPosition()[0]) {
        ax += 0.05;
        if (ax > 1.0) ax = 1.0;
   }
   mid.setHintAlign(ax, ay);
}


win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Button example");
win.setAutohide(true);

icon_still = new efl.Efl.Ui.Image(win);
mid = new efl.Elm.Button(win);
up = new efl.Elm.Button(win);
down = new efl.Elm.Button(win);
left = new efl.Elm.Button(win);
right = new efl.Elm.Button(win);

_btn_cursors_release = function() {
    if (icon_still)
    {
       setIcon(mid, icon_still);
       icon_still = null;
    }
}

box = new efl.Efl.Ui.Box(win);
box.setHintWeight(1.0, 1.0);
win.setContent(box);
box.setVisible(true);

box_initial = new efl.Efl.Ui.Box(win);
box_initial.cast("Efl.Orientation").setOrientation(efl.Efl.Orient.HORIZONTAL);
box_initial.setHintWeight(1.0, 0.0);
box.packEnd(box_initial);
box_initial.setVisible(true);

btn = new efl.Elm.Button(win);
btn.setText("elm.text", "Initial: 0.0");
box_initial.packEnd(btn);
btn.setVisible(true);

btn.on('clicked', _btn_options_cb);

btn2 = new efl.Elm.Button(win);
btn2.setText("elm.text", "Initial: 1.0");
box_initial.packEnd(btn2);
btn2.setVisible(true);
btn2.on('clicked', _btn_options_cb);

btn3 = new efl.Elm.Button(win);
btn3.setText("elm.text", "Initial: 5.0");
box_initial.packEnd(btn3);
btn3.setVisible(true);
btn3.on('clicked', _btn_options_cb);

box_gap = new efl.Efl.Ui.Box(win);
box_gap.cast("Efl.Orientation").setOrientation(efl.Efl.Orient.HORIZONTAL);
box_gap.setHintWeight(1.0, 0.0);
box.packEnd(box_gap);
box_gap.setVisible(true);

btn4 = new efl.Elm.Button(win);
btn4.setText("elm.text", "Gap: 0.1");
box_gap.packEnd(btn4);
btn4.setVisible(true);
btn4.on('clicked', _btn_options_cb);

btn5 = new efl.Elm.Button(win);
btn5.setText("elm.text", "Gap: 0.5");
box_gap.packEnd(btn5);
btn5.setVisible(true);
btn5.on('clicked', _btn_options_cb);

btn6 = new efl.Elm.Button(win);
btn6.setText("elm.text", "Gap: 1.0");
box_gap.packEnd(btn6);
btn6.setVisible(true);
btn6.on('clicked', _btn_options_cb);

up.setAutorepeat(true);
up.setAutorepeatInitialTimeout(1.0);
up.setAutorepeatGapTimeout(0.5);
up.setHintWeight(1.0, 0.0);
up.setHintAlign(-1.0, 0.0);
box.packEnd(up);
up.setVisible(true);
up.on('repeated', _btn_cursors_move)
up.on('unpressed', _btn_cursors_release);
icon_up = new efl.Efl.Ui.Image(win);
icon_up.setIcon("arrow_up");
setIcon(up, icon_up);

box_inferior = new efl.Efl.Ui.Box(win);
box_inferior.cast("Efl.Orientation").setOrientation(efl.Efl.Orient.HORIZONTAL);
box_inferior.setHintWeight(1.0, 1.0);
box_inferior.setHintAlign(-1.0, -1.0);
box.packEnd(box_inferior);
box_inferior.setVisible(true);

left.setAutorepeat(true);
left.setAutorepeatInitialTimeout(1.0);
left.setAutorepeatGapTimeout(0.5);
left.setHintWeight(0.0, 1.0);
left.setHintAlign(0.0, -1.0);
box_inferior.packEnd(left);
left.setVisible(true);
left.on('repeated', _btn_cursors_move)
left.on('unpressed', _btn_cursors_release);

icon_left = new efl.Efl.Ui.Image(win);
icon_left.setIcon("arrow_left");
setIcon(left, icon_left);

mid.setHintWeight(1.0, 1.0);
box_inferior.packEnd(mid);
mid.setVisible(true);

icon_mid = new efl.Efl.Ui.Image(win);
icon_mid.setIcon("close");
setIcon(mid, icon_mid);

right.setAutorepeat(true);
right.setAutorepeatInitialTimeout(1.0);
right.setAutorepeatGapTimeout(0.5);
right.setHintWeight(0.0, 1.0);
right.setHintAlign(0.0, -1.0);
box_inferior.packEnd(right);
right.setVisible(true);
right.on('repeated', _btn_cursors_move);
right.on('unpressed', _btn_cursors_release);

icon_right = new efl.Efl.Ui.Image(win);
icon_right.setIcon("arrow_right");
setIcon(right, icon_right);

down.setAutorepeat(true);
down.setAutorepeatInitialTimeout(1.0);
down.setAutorepeatGapTimeout(0.5);
down.setHintWeight(1.0, 0.0);
down.setHintAlign(-1.0, 0.0);
box.packEnd(down);
down.setVisible(true);
down.on('repeated', _btn_cursors_move);
down.on('unpressed', _btn_cursors_release);

icon_down = new efl.Efl.Ui.Image(win);
icon_down.setIcon("arrow_down");
setIcon(down, icon_down);

win.setSize(300, 320);
win.setVisible(true);

