
efl = require('efl');
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Bg Plain");
win.setAutohide(true);

bg = new elm.Elm.Bg(win);
bg.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bg);
bg.setVisible(true);

label1 = new elm.Elm.Label(win);
label1.setText(null, "Bubble with no icon, info or label");
label1.setVisible(true);

console.log(efl);

icon = new efl.Evas.Rectangle(win);
icon.setColor( 0, 0, 255, 255);
icon.setVisible(true);

bubble1 = new elm.Elm.Bubble(win);
bubble1.contentSet("icon", icon);
bubble1.setText("info", "INFO");
bubble1.setText(null, "LABEL");
bubble1.contentSet(null, label1);
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

label2 = new elm.Elm.Label(win);
label2.setText(null, "Bubble with no icon, info or label");
label2.setVisible(true);

bubble2 = new elm.Elm.Bubble(win);
bubble2.contentSet(null, label2);
bubble2.setSize(200, 50);
bubble2.setPosition(0, 110);
bubble2.setVisible(true);

win.setSize(300, 200);
win.setVisible(true);
