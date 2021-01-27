var path = require('path');
var express = require('express');

// All the values we are getting from the ECU
var rpm = 0, kph = 0, coolantTemp = 0, oilTemp = 0;

// Server part
var app = express();

app.use('/', express.static(path.join(__dirname, 'public')));

var server = app.listen(8090);
console.log('Server listening on port 8090');

// Socket.IO part
var io = require('socket.io')(server);

io.on('connection', function (socket) {
  console.log('New client connected!');

    //send data to client
    setInterval(function(){

        if(rpm < 30){
            rpm += 1
        } else{
            rpm = 0
        }
        if(kph < 130){
            kph += 1
        } else{
            kph = 0
        }
        if (coolantTemp < 70) ++coolantTemp;

        if (oilTemp < 80) oilTemp = oilTemp + 0.4;

      socket.emit('ecuData', {
          'rpm':Math.floor(rpm),
          'kph':Math.floor(kph),
          'coolantTemp':Math.floor(coolantTemp),
          'oilTemp':Math.floor(oilTemp)
        });

    }, 300);
});