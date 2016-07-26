efl = require('efl');

win = new efl.Efl.Ui.Win.Standard(null);
win.setText("Popup");
win.setAutohide(true);

content = new efl.Elm.Label(win);
content.setText("elm.text", "<align=center>Content</align>");

popup = new efl.Elm.Popup(win);
popup.setTimeout(3);

popup.on('timeout', function()
			   {
			       console.log("timeout");
			       popup.setVisible(false);
			   });

content_container = popup.part("elm.swallow.content").cast("Efl.Container");
content_container.setContent(content);

popup.setText("title,text", "Title");
popup.setVisible(true);

popup.on('block_clicked', function()
			   {
			       console.log("clicked")
			       popup.setVisible(false);
 			   });

win.setSize(480, 800);
win.setVisible(true);
