
efl = require('efl');

// Standard node modules
util = require('util');
path = require('path');
fs = require('fs');

// External modules, use 'npm install request twitter'
request = require('request');
Twitter = require('twitter');

user_acount = 'EnlightenmentKo';

var twit = new Twitter({
  consumer_key: '', // replace with consumer_key
  consumer_secret: '', // replace with consumer_secret
  access_token_key: '', // replace with access_token_key
  access_token_secret: '' // replace with access_token_secret
});

win = new efl.Efl.Ui.Win(null, "Efl JS Example", efl.Efl.Ui.Win.Type.BASIC, "hw");
win.setText("Twitter App");
win.setAutohide(true);

box = new efl.Efl.Ui.Box(win);
box.setHintWeight(1.0, 1.0);
win.setContent(box);
box.setVisible(true);

tweet_box = new efl.Efl.Ui.Box(win);
tweet_box.setHintWeight(1.0, 1.0);
tweet_box.setHintAlign(-1.0, -1.0);
tweet_box.setPackPadding(0.0, 30.0, true);
box.packEnd(tweet_box);

tweet_box_orient = tweet_box.cast("Efl.Ui.Direction");
tweet_box_orient.setOrientation(efl.Efl.Ui.Dir.VERTICAL);

icon_array = new Array();

twit.get('statuses/user_timeline', {screen_name: user_acount, count:10}, function(error, tweets, response) {
    if (error)
        return;

    for(var i=0; i < tweets.length; i++){
        var user_name = tweets[i].user.name;
        var screen_name = tweets[i].user.screen_name;
        var text = tweets[i].text;

        var layout = new efl.Efl.Ui.Layout(win);
        var filename = path.join(__dirname, 'twitter_example_01.edj');
        layout.setFile(filename, "tweet");

        layout.setPartText("user_name", user_name);
        layout.setPartText("screen_name", " - @"+screen_name);

        var entry = new efl.Elm.Entry(win);
        entry.setPartText("elm.text", text);
        entry.setEditable(false);
        var part = layout.part("tweet_text").cast("Efl.Content");
        part.setContent(entry);

        layout.setHintMin(127, 96);
        layout.setHintWeight(1.0, 1.0);
        layout.setHintAlign(-1.0, -1.0);

        var icon = new efl.Efl.Ui.Image(win);
        icon.fillInside = true;
        icon_array.push(icon);
        var user_icon = layout.part("user_icon").cast("Efl.Content");
        user_icon.setContent(icon);
        item = tweet_box.packEnd(layout);
        layout.setVisible(true);
    }

    var icon_filename = '/tmp/twitter_pic.jpg';
    var file = fs.createWriteStream(icon_filename);
    file.on('finish', function() {
        console.log("finished loading the icon file.");
        for (var i=0; i < icon_array.length; i++) {
            icon_array[i].setFile(icon_filename, null);
        }
    });

    // Load the tweet icons
    if (tweets.length > 0) {
        request(tweets[0].user.profile_image_url).pipe(file);
    }

    tweet_box.setVisible(true);
});

win.setSize(380, 400);
win.setVisible(true);
