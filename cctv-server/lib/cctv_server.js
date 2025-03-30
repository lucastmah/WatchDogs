
// var fs       = require('fs');
var socketio = require('socket.io');
var io; 
var dgram    = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	// io.set('log level 1');
	
	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function handleCommand(socket) {
	// console.log("Setting up socket handlers.");

	// Zoom in, out
	socket.on('zoom', function(zoom) {
        console.log("Got zoom command: " + zoom);
		relayToLocalPort(socket, "zoom " + zoom, "zoom-reply");
	});
    // Pan left, right, up, down, stop
	socket.on('pan', function(pan) {
		console.log("Got pan command: " + pan);
		relayToLocalPort(socket, "pan " + pan, "pan-reply");
	});
	socket.on('tilt', function(tilt) {
		console.log("Got tilt command: " + tilt);
		relayToLocalPort(socket, "tilt " + tilt, "tilt-reply");
	});
    socket.on('stop', function(notUsed) {
		console.log("Got stop command: ");
		relayToLocalPort(socket, "stop", "stop-reply");
	});
};

function relayToLocalPort(socket, data, replyCommandName) {
	console.log('relaying to local port command: ' + data);
	
	// Info for connecting to the local process via UDP
	var PORT = 12345;	// Port of local application
	var HOST = '127.0.0.1';
	var buffer = new Buffer(data);

	// Send an error if we have not got a reply in a second
    var errorTimer = setTimeout(function() {
    	console.log("ERROR: No reply from local application.");
    	socket.emit("server-error", "SERVER ERROR: No response from beagleY-AI. Is it running?");
    }, 1000);

	
	var client = dgram.createSocket('udp4');
	client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
	    if (err) 
	    	throw err;
	    console.log('UDP message sent to ' + HOST +':'+ PORT);
	});
	
	client.on('listening', function () {
	    var address = client.address();
	    console.log('UDP Client: listening on ' + address.address + ":" + address.port);
	});
	// Handle an incoming message over the UDP from the local application.
	client.on('message', function (message, remote) {
	    console.log("UDP Client: message Rx" + remote.address + ':' + remote.port +' - ' + message);
	    
	    var reply = message.toString('utf8')
	    socket.emit(replyCommandName, reply);
	    clearTimeout(errorTimer);
	    client.close();
	});
	
	client.on("UDP Client: close", function() {
	    console.log("closed");
	});
	client.on("UDP Client: error", function(err) {
	    console.log("error: ",err);
	});	
}