efl = require('efl');
path = require('path');

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

_client_connected_cb = function(obj)
{
   console.log("Connected client to ethumb daemon");
}

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Thumb example.");
win.setAutohide(true);

thumb = new efl.Efl.Ui.Image(win);

thumb.on('generate,start', _generation_started_cb);
thumb.on('generate,stop', _generation_finished_cb);
thumb.on('generate,error', _generation_error_cb);

thumb.setSize(160, 160);
// legacy
// thumb.setEditable(false);
filename = path.join(__dirname, "../../../data/elementary/images/plant_01.jpg");
filename = process.argv[2] || filename;
thumb.setFile(filename, null);
// legacy
// thumb.reload();

thumb.setHintWeight(1.0, 1.0);
win.setContent(thumb);

thumb.setVisible(true);
win.setSize(320, 320);
win.setVisible(true);
