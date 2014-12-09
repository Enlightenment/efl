
efl = require('efl');
elm = require('elm');

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

win = new elm.Elm.WinStandard(null);
win.setTitle("Twitter App");
win.setAutohide(true);

box = new elm.Elm.Box(win);
box.setSizeHintWeight(1.0, 1.0);
win.resizeObjectAdd(box);
box.setVisible(true);

list = new elm.Elm.List(win);
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
                icon_array[i].setFile("/tmp/twitter_pic.jpg", null);
            }
        });
        if (tweets.length > 0) {
            request(tweets[0].user.profile_image_url).pipe(file);
        }

        for(i=0; i < tweets.length; i++){
            var layout = new elm.Elm.Layout(win);
            layout.setFile("twitter_example_01.edj", "tweet");

            var user_name = tweets[i].user.name;
            var screen_name = tweets[i].user.screen_name;
            var user_icon_url = tweets[i].user.profile_image_url;
            var text = tweets[i].text;

            layout.setText("user_name", screen_name);
            layout.setText("screen_name", " - @"+screen_name);
            var entry = new elm.Elm.Entry(win);
            entry.setText("elm.text", text);
            console.log(text);
            layout.contentSet("tweet_text", entry);

            layout.setSizeHintMin(127, 96);
            layout.setSizeHintWeight(1.0, 1.0);
            layout.setSizeHintAlign(-1.0, -1.0);

            var icon = new elm.Elm.Image(win);
            icon.setFillInside(true);
            icon_array.push(icon);
            layout.contentSet("user_icon", icon);
            item = list.itemAppend("", layout, null, null, null);
        }
        list.go();
    }
});

win.setSize(380,400);
win.setVisible(true);
