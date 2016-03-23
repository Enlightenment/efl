
efl = require('efl');
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Slider Example");
win.setAutohide(true);

bx = new elm.Elm.Box(win);
//bx.setHorizontal(true);
bx.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(bx);
bx.setVisible(true);

sl = new elm.Elm.Slider(win);
sl.setSizeHintAlign(-1.0, 0.5);
sl.setSizeHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* with icon, end and label */
sl = new elm.Elm.Slider(win);
sl.setText("elm.text", "Counter");

ic = new elm.Elm.Icon(win);
ic.setStandard("home");
ic.setResizable(false, false);
sl.contentSet("icon", ic);

ic = new elm.Elm.Icon(win);
ic.setStandard("folder");
ic.setResizable(false, false);
sl.contentSet("end", ic);

sl.setSizeHintAlign(-1.0, 0.5);
sl.setSizeHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* value set and span size */
sl = new elm.Elm.Slider(win);
sl.setValue(1);
sl.setSpanSize(200);
sl.setSizeHintAlign(-1.0, 0.5);
sl.setSizeHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* with unit label and min - max */
sl = new elm.Elm.Slider(win);
sl.setUnitFormat("%1.0f units");
sl.setMinMax(0, 100);
sl.setSizeHintAlign(-1.0, 0.5);
sl.setSizeHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* with indicator label and inverted */
sl = new elm.Elm.Slider(win);
sl.setIndicatorFormat("%1.2f");
sl.setInverted(true);
sl.setSizeHintAlign(-1.0, 0.5);
sl.setSizeHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* vertical with indicator format func */
sl = new elm.Elm.Slider(win);
sl.setHorizontal(false);
//indicator_format = function()
//                  {
//                      indicator = new char[32];
//                      nprintf(indicator, 32, "%1.2f u", val);
//                      return indicator;
//                  }
//indicator_free = function() {console.log("free")}
//sl.indicator_format_function_set(indicator_format, indicator_free);

sl.setSizeHintAlign(0.5, -1.0);
sl.setSizeHintWeight(0.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

/* callbacks */
sl = new elm.Elm.Slider(win);
sl.setUnitFormat("%1.3f units");
sl.setSizeHintAlign(-1.0, 0.5);
sl.setSizeHintWeight(1.0, 1.0);
bx.packEnd(sl);
sl.setVisible(true);

sl.on('changed', function(obj)
                  {
                     val = obj.getValue();
                     console.log("Changed to " + val);
                  });

sl.on('delay_changed', function(obj)
                 {
                     val = obj.getValue();
                     console.log("Delay changed to " + val);
                 });

win.setVisible(true);
