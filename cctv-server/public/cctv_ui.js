'use strict';

var socket = io.connect();

socket.on('image', (data) => {
	console.log('data', data);
	// const imageElm = document.getElementById('image');
	// image.src = `data:image/jpeg;base64,${image}`;
});

var mute = true;
var toggleMic = false;
var communicationsTimeout = null;
var patrolling = false;

function setupHoldButton(selector, onHold, interval = 50) {
    let intervalId = null;

    const start = () => {
        if (!intervalId) {
            onHold();
            intervalId = setInterval(onHold, interval);
        }
    };

    const stop = () => {
        clearInterval(intervalId);
        intervalId = null;
    };

    $(selector).on('mousedown touchstart', start);
    $(selector).on('mouseup mouseleave touchend', stop);
}

$(document).ready(function() {

	socket.on('canvas', function(data) {
		const canvas = $("#videostream");
		const context = canvas[0].getContext('2d');
		const image = new Image();
		image.src = "data:image/jpeg;base64,"+data;
		image.onload = function() {
			context.height = image.height;
			context.width = image.width;
			context.drawImage(image,0,0,context.width, context.height);
		}
	});

	setupServerMessageHandlers(socket);
	// Set up button holds:
	setupHoldButton('#panLeft', () => {
		patrolling = false;
		sendCommandToServer('pan', "-1")
	});
	setupHoldButton('#panRight', () => {
		patrolling = false;
		sendCommandToServer('pan', "1")
	});
	setupHoldButton('#panUp', () => {
		patrolling = false;
		sendCommandToServer('tilt', "1")
	});
	setupHoldButton('#panDown', () => {
		patrolling = false;
		sendCommandToServer('tilt', "-1")
	});

	// Setup the button clicks:
	$('#zoomIn').click(function() {
		console.log("zoom in!");
		sendCommandToServer('zoom', "0");
	});
	$('#zoomOut').click(function() {
		console.log("zoom out!");
		sendCommandToServer('zoom', "1");
	});
	$('#patrol').click(function() {
		patrolling = !patrolling;

		if (patrolling) {
			console.log("starting patrol mode!");
			sendCommandToServer('patrol', "1");	
		}
		else {
			console.log("stopping patrol mode!");
			sendCommandToServer('patrol', "0");
		}
	});
	$('#mute').click(function() {
		mute = !mute;
		if (mute) {
			console.log("mute!");
			sendCommandToServer('mute', "1");	
		}
		else {
			console.log("unmute!");
			sendCommandToServer('mute', "0");
		}
	});
	$('#toggleMic').click(function() {
		console.log("toggle mic!");
		toggleMic = !toggleMic;
		if (toggleMic) {
			console.log("talk on!");
			sendCommandToServer('talk', "1");	
		}
		else {
			console.log("talk off!");
			sendCommandToServer('talk', "0");
		}
	});
	$('#stop').click(function() {
		console.log("stopping!");
		sendCommandToServer('stop');
	});
});

var hideErrorTimeout;

function setupServerMessageHandlers(socket) {
	// Hide error display:
	$('#error-box').hide(); 
	
	
	socket.on('talk-reply', function(message) {
		console.log("Receive Reply: talk-reply " + message);
		clearServerTimeout();
	});
	
	socket.on('zoom-reply', function(message) {
		console.log("Receive Reply: zoom-reply " + message);
		clearServerTimeout();
	});
	
	socket.on('mute-reply', function(message) {
		console.log("Receive Reply: mute-reply " + message);
		clearServerTimeout();
	});
	
	socket.on('pan-reply', function(message) {
		console.log("Receive Reply: pan-reply " + message);
		clearServerTimeout();
	});
	
	socket.on('tilt-reply', function(message) {
		console.log("Receive Reply: tilt-reply " + message);
		clearServerTimeout();
	});

	socket.on('patrol-reply', function(message) {
		console.log("Receive Reply: patrol-reply " + message);
		patrolling = Number(message) === 1;
		clearServerTimeout();
	});

	socket.on('stop-reply', function(message) {
		console.log("Receive Reply: stop-reply " + message);
		clearServerTimeout();
	});

	socket.on('server-error', errorHandler);
}

function sendCommandToServer(command, options) {
	if (communicationsTimeout == null) {
		communicationsTimeout = setTimeout(errorHandler, 1000, 
				"ERROR: Unable to communicate to HTTP server. Is nodeJS server running?");
	}
	socket.emit(command, options);
}

function clearServerTimeout() {
	clearTimeout(communicationsTimeout);
	communicationsTimeout = null;
}

var hideErrorTimeout;

function errorHandler(message) {
	console.log("ERROR Handler: " + message);
	// Make linefeeds into <br> tag.
//	message = replaceAll(message, "\n", "<br/>");
	
	$('#error-text').html(message);	
	$('#error-box').show();
	
	// Hide it after a few seconds:
	window.clearTimeout(hideErrorTimeout);
	hideErrorTimeout = window.setTimeout(function() {$('#error-box').hide();}, 5000);
	clearServerTimeout();
}