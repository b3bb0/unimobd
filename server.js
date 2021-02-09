var path = require('path');
var sp = require("serialport");
var express = require('express');

var port = new sp("/dev/ttyACM0", { baudRate: 115200 });
// var port = new sp("/dev/tty.usbmodem14114101", { baudRate: 115200 });

const Readline = sp.parsers.Readline;
const parser = new Readline();
port.pipe(parser);

if (!Object.prototype.forEach) {
	Object.defineProperty(Object.prototype, 'forEach', {
		value: function (callback, thisArg) {
			if (this == null) {
				throw new TypeError('Not an object');
			}
			for (var key in this) {
				if (this.hasOwnProperty(key)) {
					callback.call(thisArg, this[key], key, this);
				}
			}
		}
	});
}

// Server part
var app = express();

app.use('/', express.static(path.join(__dirname, 'public')));

var server = app.listen(8090,"0.0.0.0");
console.log('Server listening on port 8090');

// Socket.IO part
var io = require('socket.io')(server);

var connected = {};

io.on('connection', function (socket) {
    var id = generateId();
    connected[id] = socket;
    console.log(id, "connected");
    socket.on('disconnect', () => {
        delete connected[socket];
        console.log(id, "disconnected");
    });
});


port.on("open",function() { console.log("\nPort open\n"); });
port.on("close", function() { console.log("\nConnection lost\n"); });
port.on("error", function(err) { console.log("\nConnection error\n",err); });

parser.on("data",function(data) {
    var params = data.split(':');

    var res = {};
    res[params[0]] = params[1];
    
    connected.forEach( (socket) => {
        socket.emit('ecuData', res);
    })
});

function generateId(length) {
    var result           = '';
    var characters       = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
    var charactersLength = characters.length;
    for ( var i = 0; i < length; i++ ) {
       result += characters.charAt(Math.floor(Math.random() * charactersLength));
    }
    return result;
 }


 