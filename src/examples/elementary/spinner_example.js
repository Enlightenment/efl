
efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Spinner Example");
win.setAutohide(true);

bx = new efl.Efl.Ui.Box(win);
bx.setHintWeight(1.0, 1.0);
win.setContent(bx);
bx.setVisible(true);

sp = new efl.Elm.Spinner(win);
sp.setHintWeight(1.0, 1.0);
sp.setHintAlign(-1.0, 0.5);
bx.packEnd(sp);
sp.setVisible(true);

sp2 = new efl.Elm.Spinner(win);
sp2.setLabelFormat("Percentage %%%1.2f something");
sp2.setHintWeight(1.0, 1.0);
sp2.setHintAlign(-1.0, 0.5);
bx.packEnd(sp2);
sp2.setVisible(true);

sp3 = new efl.Elm.Spinner(win);
sp3.setLabelFormat("%1.1f units");
sp3.setStep(1.5);
sp3.setWrap(true);
sp3.setMinMax(-50.0, 250.0);
sp3.setHintWeight(1.0, 1.0);
sp3.setHintAlign(-1.0, 0.5);
bx.packEnd(sp3);
sp3.setVisible(true);

sp4 = new efl.Elm.Spinner(win);
sp4.setStyle("vertical");
sp4.setInterval(0.2);
sp4.setHintWeight(1.0, 1.0);
sp4.setHintAlign(-1.0, 0.5);
bx.packEnd(sp4);
sp4.setVisible(true);

sp5 = new efl.Elm.Spinner(win);
sp5.setEditable(false);
sp5.setHintWeight(1.0, 1.0);
sp5.setHintAlign(-1.0, 0.5);
bx.packEnd(sp5);
sp5.setVisible(true);

sp6 = new efl.Elm.Spinner(win);
sp6.setEditable(false);
sp6.setMinMax(1, 12);
sp6.addSpecialValue(1, "January");
sp6.addSpecialValue(2, "February");
sp6.addSpecialValue(3, "March");
sp6.addSpecialValue(4, "April");
sp6.addSpecialValue(5, "May");
sp6.addSpecialValue(6, "June");
sp6.addSpecialValue(7, "July");
sp6.addSpecialValue(8, "August");
sp6.addSpecialValue(9, "September");
sp6.addSpecialValue(10, "October");
sp6.addSpecialValue(11, "November");
sp6.addSpecialValue(12, "December");
sp6.setHintWeight(1.0, 1.0);
sp6.setHintAlign(-1.0, 0.5);
bx.packEnd(sp6);
sp6.setVisible(true);

sp7 = new efl.Elm.Spinner(win);
sp7.setHintWeight(1.0, 1.0);
sp7.setHintAlign(-1.0, 0.5);
bx.packEnd(sp7);
sp7.setVisible(true);
sp7.setEditable(true);

sp7.on('changed',
  function(obj)
  {
      console.log("Value changed to " + obj.getValue());
  });

sp7.on('delay_changed',
  function(obj)
  {
      console.log("Value delay changed to " + obj.getValue());
  });

win.setVisible(true);
