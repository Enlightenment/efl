
var efl = require('efl');

var loop = new efl.Ecore_Mainloop(null);

var secondsToWait = parseFloat(process.argv[2]);

if (isNaN(secondsToWait))
  {
     secondsToWait = 2;
  }

console.log('Waiting ' + secondsToWait + ' seconds...');

if (false)
  {
    var timer = new efl.Timer(null, secondsToWait,
                            function(){
                               console.log("Timer cb called;");
                               loop.quit();
                            }, null);
  }
else
  {
     setTimeout(function(){
        console.log("Js callback called;");
        loop.quit();
     }, secondsToWait*1000);
  }

loop.begin();

process.exit(0);