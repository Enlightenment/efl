efl = require('efl');
elm = require('elm');

win = new elm.Elm.WinStandard(null);
win.setTitle("Popup");
win.setAutohide(true);

content = new elm.Elm.Label(win);
content.setText("elm.text", "<align=center>Content</align>");

popup = new elm.Elm.Popup(win);
popup.setTimeout(3);

popup.on('timeout', function()
			   {
			       console.log("timeout");
			       popup.setVisible(false);
			   });

popup.contentSet("elm.swallow.content", content);

popup.setText("title,text", "Title");
popup.setVisible(true);

popup.on('block_clicked', function()
			   {
			       console.log("clicked")
			       popup.setVisible(false);
 			   });

win.setSize(480, 800);
win.setVisible(true);
