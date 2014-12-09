
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Spinner Example");
win.setAutohide(true);

bx = new elm.Elm.Box(win);
bx.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bx);
bx.setVisible(true);

sp = new elm.Elm.Spinner(win);
sp.setSizeHintWeight(1.0, 1.0);
sp.setSizeHintAlign(-1.0, 0.5);
bx.packEnd(sp);
sp.setVisible(true);

sp2 = new elm.Elm.Spinner(win);
sp2.setLabelFormat("Percentage %%%1.2f something");
sp2.setSizeHintWeight(1.0, 1.0);
sp2.setSizeHintAlign(-1.0, 0.5);
bx.packEnd(sp2);
sp2.setVisible(true);

sp3 = new elm.Elm.Spinner(win);
sp3.setLabelFormat("%1.1f units");
sp3.setStep(1.5);
sp3.setWrap(true);
sp3.setMinMax(-50.0, 250.0);
sp3.setSizeHintWeight(1.0, 1.0);
sp3.setSizeHintAlign(-1.0, 0.5);
bx.packEnd(sp3);
sp3.setVisible(true);

sp4 = new elm.Elm.Spinner(win);
sp4.setStyle("vertical");
sp4.setInterval(0.2);
sp4.setSizeHintWeight(1.0, 1.0);
sp4.setSizeHintAlign(-1.0, 0.5);
bx.packEnd(sp4);
sp4.setVisible(true);

sp5 = new elm.Elm.Spinner(win);
sp5.setEditable(false);
sp5.setSizeHintWeight(1.0, 1.0);
sp5.setSizeHintAlign(-1.0, 0.5);
bx.packEnd(sp5);
sp5.setVisible(true);

sp6 = new elm.Elm.Spinner(win);
sp6.setEditable(false);
sp6.setMinMax(1, 12);
sp6.specialValueAdd(1, "January");
sp6.specialValueAdd(2, "February");
sp6.specialValueAdd(3, "March");
sp6.specialValueAdd(4, "April");
sp6.specialValueAdd(5, "May");
sp6.specialValueAdd(6, "June");
sp6.specialValueAdd(7, "July");
sp6.specialValueAdd(8, "August");
sp6.specialValueAdd(9, "September");
sp6.specialValueAdd(10, "October");
sp6.specialValueAdd(11, "November");
sp6.specialValueAdd(12, "December");
sp6.setSizeHintWeight(1.0, 1.0);
sp6.setSizeHintAlign(-1.0, 0.5);
bx.packEnd(sp6);
sp6.setVisible(true);

sp7 = new elm.Elm.Spinner(win);
sp7.setSizeHintWeight(1.0, 1.0);
sp7.setSizeHintAlign(-1.0, 0.5);
bx.packEnd(sp7);
sp7.setVisible(true);
sp7.setEditable(true);

sp7.on('changed',
  function(obj)
  {
      console.log("Value changed to " + obj.value_get());
  });

sp7.on('delay_changed',
  function(obj)
  {
      console.log("Value delay changed to " + obj.value_get());
  });

win.setVisible(true);
