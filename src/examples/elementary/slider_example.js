
efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Slider Example");
win.setAutohide(true);

bx = new efl.Efl.Ui.Box(win);
bx.cast("Efl.Orientation").setOrientation(efl.Efl.Orient.HORIZONTAL);
bx.setHintWeight(1.0, 1.0);
win.setContent(bx);
bx.setVisible(true);

sl = new efl.Elm.Slider(win);
sl.setHintAlign(-1.0, 0.5);
sl.setHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* with icon, end and label */
sl = new efl.Elm.Slider(win);
sl.setText("elm.text", "Counter");

ic = new efl.Efl.Ui.Image(win);
ic.setIcon("home");
// ic.setResizable(false, false);
sl.part("icon").cast("Efl.Container").setContent(ic);

ic = new efl.Efl.Ui.Image(win);
ic.setIcon("folder");
// ic.setResizable(false, false);
sl.part("end").cast("Efl.Container").setContent(ic);

sl.setHintAlign(-1.0, 0.5);
sl.setHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* value set and span size */
sl = new efl.Elm.Slider(win);
sl.setProgressValue(1);
sl.setSpanSize(200);
sl.setHintAlign(-1.0, 0.5);
sl.setHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* with unit label and min - max */
sl = new efl.Elm.Slider(win);
sl.setUnitFormat("%1.0f units");
sl.setMinMax(0, 100);
sl.setHintAlign(-1.0, 0.5);
sl.setHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* with indicator label and inverted */
sl = new efl.Elm.Slider(win);
sl.setIndicatorFormat("%1.2f");
// legacy function, not supported.
// sl.setInverted(true);
sl.setHintAlign(-1.0, 0.5);
sl.setHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* vertical with indicator format func */
sl = new efl.Elm.Slider(win);
sl.cast("Efl.Orientation").setOrientation(efl.Efl.Orient.VERTICAL);
//indicator_format = function()
//                  {
//                      indicator = new char[32];
//                      nprintf(indicator, 32, "%1.2f u", val);
//                      return indicator;
//                  }
//indicator_free = function() {console.log("free")}
//sl.indicator_format_function_set(indicator_format, indicator_free);

sl.setHintAlign(0.5, -1.0);
sl.setHintWeight(0.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* callbacks */
sl = new efl.Elm.Slider(win);
sl.setUnitFormat("%1.3f units");
sl.setHintAlign(-1.0, 0.5);
sl.setHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

sl.on('changed', function(obj)
                  {
                     val = obj.getProgressValue();
                     console.log("Changed to " + val);
                  });

sl.on('delay_changed', function(obj)
                 {
                     val = obj.getProgressValue();
                     console.log("Delay changed to " + val);
                 });

win.setVisible(true);
