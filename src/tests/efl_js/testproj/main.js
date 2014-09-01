var fs = require('fs');

text = fs.readFileSync('hello.txt', {encoding: 'utf8'});

console.log(text);