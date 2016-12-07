path = require('path');
efl = require('efl');
Evas = efl.Evas;
Elm = efl.Elm;

win = new efl.Efl.Ui.Win.Standard(null);
win.setText('Layout');
win.setAutohide(true);

box = new efl.Efl.Ui.Box(win);
box.setHintWeight(1.0, 1.0);
win.setContent(box);
box.setVisible(true);

ly = new Elm.Layout(box);

if (!ly.setTheme("layout", "application", "titlebar"))
{
    console.log('Error setting layout');
}

ly.setPartText('elm.text', 'Some title');
ly.setHintWeight(1.0, 1.0);
ly.setHintAlign(1.0, 1.0);
box.packEnd(ly);
ly.setVisible(true);

bt = new efl.Efl.Ui.Image(ly);
bt.setIcon('chat');
bt.setHintMin(20, 20);
// elm_layout_icon_set(ly, bt);
icon_container = ly.part('elm.swallow.icon').cast('Efl.Container');
icon_container.setContent(bt);
ly.emitSignal('elm,state,icon,visible', 'elm');

bt = new efl.Efl.Ui.Image(ly);
bt.setIcon('close');
bt.setHintMin(20, 20);
// elm_layout_end_set(ly, bt);
end_container = ly.part('elm.swallow.end').cast('Efl.Container');
end_container.setContent(bt);
ly.emitSignal('elm,state,end,visible', 'elm');

ly = new Elm.Layout(box);
filename = path.join(__dirname, 'layout_example.edj');
ly.setFile(filename, 'example/mylayout');
ly.setHintWeight(1.0, 1.0);
box.packEnd(ly);
ly.setVisible(true);

//elm_layout_signal_callback_add(ly, "*", "*", _cb_signal, NULL);
// I hope to translate to:
//ly.on_layout('*', '*', function(emission, source)
//             { console.log("signal: '" + emission + "' '" + source + "'");});

bt = new Elm.Button(ly);
bt.setPartText(null, "Button 1");
console.log("Will get part");
element_container = ly.part('example/custom').cast('Efl.Container');
console.log("Will setcontent on part");
element_container.setContent(bt);

bt.on("clicked", function() { console.log('button clicked'); });

ly.setPartCursor("example/title", 'watch');
ly.on('mouse,down', function() { console.log('layout mouse down') });
ly.on('mouse,up', function() { console.log('layout mouse up') });

win.setVisible(true);
