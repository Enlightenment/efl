elm = require('elm');

_generation_started_cb = function(obj)
{
   console.log("thumbnail generation started.");
}

_generation_finished_cb = function(obj)
{
   console.log("thumbnail generation finished.");
}

_generation_error_cb = function(obj)
{
   console.log("thumbnail generation error.");
}

win = new elm.Elm.WinStandard(null);
win.setTitle("Table");
win.setAutohide(true);

thumb = new elm.Elm.Thumb(win);

thumb.on('generate_start', _generation_started_cb);
thumb.on('generate_stop', _generation_finished_cb);
thumb.on('generate_error', _generation_error_cb);

thumb.setSize(160, 160);
thumb.setEditable(false);
thumb.setFile("../../data/images/plant_01.jpg", null);
thumb.reload();

thumb.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(thumb);

thumb.setVisible(true);
win.setSize(320, 320);
win.setVisible(true);
