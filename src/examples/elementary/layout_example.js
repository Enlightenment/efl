
Elm = require('elm').Elm;
efl = require('efl');
Evas = efl.Evas;

win = new Elm.WinStandard(null);
win.setTitle("Layout");
win.setAutohide(true);

box = new Elm.Box(win);
box.setSizeHintWeight(1.0, 1.0);
win.addResizeObject(box);
box.setVisible(true);

ly = new Elm.Layout(box);

if (!ly.setTheme("layout", "application", "titlebar"))
{
    console.log('Error setting layout');
}

ly.setPartText('elm.text', 'Some title');
ly.setSizeHintWeight(1.0, 1.0);
ly.setSizeHintAlign(1.0, 1.0);
box.packEnd(ly);
ly.setVisible(true);

bt = new Elm.Icon(ly);
bt.setStandard('chat');
bt.setSizeHintMin(20, 20);
// elm_layout_icon_set(ly, bt);
ly.setContent('elm.swallow.icon', bt);
ly.emitSignal('elm,state,icon,visible', 'elm');

bt = new Elm.Icon(ly);
bt.setStandard('close');
bt.setSizeHintMin(20, 20);
// elm_layout_end_set(ly, bt);
ly.setContent('elm.swallow.end', bt);
ly.emitSignal('elm,state,end,visible', 'elm');

ly = new Elm.Layout(box);
ly.setFile('/home/felipe/dev/samsung/upstream/elementary/build/data/objects/test.edj', "layout");
ly.setSizeHintWeight(1.0, 1.0);
box.packEnd(ly);
ly.setVisible(true);

//elm_layout_signal_callback_add(ly, "*", "*", _cb_signal, NULL);
// I hope to translate to:
//ly.on_layout('*', '*', function(emission, source)
//             { console.log("signal: '" + emission + "' '" + source + "'");});

bt = new Elm.Button(ly);
bt.setPartText(null, "Button 1");
ly.setContent("element1", bt);

bt.on("clicked", function() { console.log('button clicked'); });

ly.setPartCursor("text", 'watch');
ly.on('mouse,down', function() { console.log('layout mouse down') });
ly.on('mouse,up', function() { console.log('layout mouse up') });

win.setVisible(true);
