var socketio = require('socket.io');
var io; 
var dgram    = require('dgram');
const child = require('child_process');

exports.listen = function(server) {
	io = socketio.listen(server);
	// io.set('log level 1');
	
	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function readVidPort(port, deviceNo){
	let ffmpeg = child.spawn("ffmpeg", [
		"-y",
		"-i",
		"udp://192.168.7.2:" + port,
		"-preset",
		"ultrafast",
		"-vcodec",
		"copy",
		"-f",
		"mjpeg",
		"pipe:1"
	]);

	ffmpeg.on('error', function (err) {
		console.log(err);
		throw err;
	});

	ffmpeg.on('close', function (code) {
		console.log('ffmpeg exited with code ' + code);
	});

	ffmpeg.stderr.on('data', function(data) {
		// Don't remove this
		// Child Process hangs when stderr exceed certain memory
		// console.error("something wrong:", data.toString());
	});

	ffmpeg.stdout.on('data', function(data) {
		// console.log("out:", data);
		io.sockets.emit('camStatus', deviceNo);
		var frame = Buffer.from(data).toString('base64'); //convert raw data to string
		io.sockets.emit('canvas' + deviceNo,frame); //send data to client
	});
}

function handleCommand(socket) {
	// console.log("Setting up socket handlers.");
	console.log('a user connected');
	readVidPort(12344, 0);
	readVidPort(12346, 1);
	
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
	socket.on('patrol', function(patrol) {
		console.log("Got patrol command: " + patrol);
		relayToLocalPort(socket, "patrol " + patrol, "patrol-reply");
	});
    socket.on('mute', function(mute) {
		console.log("Got mute command: " + mute);
		relayToLocalPort(socket, "mute " + mute, "mute-reply");
	});
	socket.on('talk', function(talk) {
		console.log("Got talk command: " + talk);
		relayToLocalPort(socket, "talk " + talk, "talk-reply");
	});
	socket.on('motion_light', function(motion_light) {
		console.log("Got motion_light command: " + motion_light);
		relayToLocalPort(socket, "motion_light " + motion_light, "motion_light-reply");
	});
	socket.on('stop', function(stop) {
		console.log("Got stop command: " + stop);
		relayToLocalPort(socket, "stop " + stop, "stop-reply");
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