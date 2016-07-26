
efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Bg Plain");
win.setAutohide(true);

bg = new efl.Elm.Bg(win);
bg.setHintWeight(1.0, 1.0);
win.pack(bg);
bg.setVisible(true);

label1 = new efl.Elm.Label(win);
label1.setText(null, "Bubble with icon, info and label");
label1.setVisible(true);

console.log(efl);

icon = new efl.Efl.Canvas.Rectangle(win);
icon.setColor( 0, 0, 255, 255);
icon.setVisible(true);

bubble1 = new efl.Elm.Bubble(win);
// bubble1.setContent(icon);
bubble1.setText("info", "INFO");
bubble1.setText(null, "LABEL");
bubble1.setContent(label1);
bubble1.setSize(300, 100);
bubble1.setVisible(true);

corner = 0;
bubble1.on('clicked',
  function()
  {
      ++corner;
      if (corner > 3)
          bubble1.setPos(corner = 0);
      else
          bubble1.setPos(corner);
  });

label2 = new efl.Elm.Label(win);
label2.setText(null, "Bubble with no icon, info or label");
label2.setVisible(true);

bubble2 = new efl.Elm.Bubble(win);
bubble2.setContent(label2);
bubble2.setSize(200, 50);
bubble2.setPosition(0, 110);
bubble2.setVisible(true);

win.setSize(300, 200);
win.setVisible(true);
