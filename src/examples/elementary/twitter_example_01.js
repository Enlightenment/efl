
efl = require('efl');

util = require('util');
fs = require('fs');

user_acount = 'EnlightenmentKo'

request = require('request'); //need "npm install request"
Twitter = require('twitter'); //need "npm install twitter"

var twit = new Twitter({
  consumer_key: 'ZbSM93w5Sp2cyZ2SG0XuCvoHV',
  consumer_secret: 'g8N7EEQLpdKPnAsS9hWuQV29FYjBkhH62jhZzXyYymDw87DKye',
  access_token_key: '222611263-pPhKKjYh59uuNLP0b86sP7aAtLhdecjVQaEsCDCv',
  access_token_secret: 'l7ccNKXTVv6cymfSD1gQH61tmfixkdna2QmOjPtpVxSHD'
});

win = new efl.ui.WinStandard(null);
win.title = "Twitter App";
win.autohide = true;

box = new efl.ui.Box(win);
box.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(box);
box.setVisible(true);

list = new efl.List(win);
list.setSizeHintWeight(1.0, 1.0);
list.setSizeHintAlign(-1.0, -1.0);
box.packEnd(list);
list.setVisible(true);

icon_array = new Array();

twit.get('statuses/user_timeline', {screen_name: user_acount, count:10}, function(error, tweets, response) {
    if (!error){
        file = fs.createWriteStream('/tmp/twitter_pic.jpg');
        file.on('finish', function() {
            console.log("finished");
            for (i=0; i < icon_array.length; i++) {
                console.log(i);
                icon_array[i].file = "/tmp/twitter_pic.jpg";
            }
        });
        if (tweets.length > 0) {
            request(tweets[0].user.profile_image_url).pipe(file);
        }

        for(i=0; i < tweets.length; i++){
            var layout = new efl.ui.Layout(win);
            layout.file["tweet"] = "twitter_example_01.edj";

            var user_name = tweets[i].user.name;
            var screen_name = tweets[i].user.screen_name;
            var user_icon_url = tweets[i].user.profile_image_url;
            var text = tweets[i].text;

            layout.setText("user_name", screen_name);
            layout.setText("screen_name", " - @"+screen_name);
            var entry = new efl.ui.Entry(win);
            entry.text["elm.text"] = text;
            console.log(text);
            layout.content["tweet_text"] = entry;

            layout.sizeHintMin = {127, 96};
            layout.sizeHintWeight = {1.0, 1.0};
            layout.sizeHintAlign = {-1.0, -1.0};

            var icon = new efl.ui.Image(win);
            icon.fillInside = true;
            icon_array.push(icon);
            layout.content["user_icon"] = icon;
            item = list.itemAppend("", layout, null, null, null);
        }
        list.go();
    }
});

win.size = {380,400};
win.visible = {true};
