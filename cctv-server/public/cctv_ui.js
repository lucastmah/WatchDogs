'use strict';

var socket = io.connect();

socket.on('image', (data) => {
	console.log('data', data);
	// const imageElm = document.getElementById('image');
	// image.src = `data:image/jpeg;base64,${image}`;
});

mute = true;
toggleMic = false;
var communicationsTimeout = null;

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
	setupServerMessageHandlers(socket);
	// Set up button holds:
	setupHoldButton('#panLeft', () => sendCommandToServer('pan', "0"));
	setupHoldButton('#panRight', () => sendCommandToServer('pan', "1"));
	setupHoldButton('#panUp', () => sendCommandToServer('tilt', "1"));
	setupHoldButton('#panDown', () => sendCommandToServer('tilt', "0"));

	// Setup the button clicks:
	$('#zoomIn').click(function() {
		console.log("zoom in!");
		sendCommandToServer('zoom', "0");
	});
	$('#zoomOut').click(function() {
		console.log("zoom out!");
		sendCommandToServer('zoom', "1");
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
	// $('#error-box').hide(); 
	
	
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

function errorHandler(message) {
	console.log("ERROR Handler: " + message);
	// Make linefeeds into <br> tag.
//	message = replaceAll(message, "\n", "<br/>");
	
	// $('#error-text').html(message);	
	// $('#error-box').show();
	
	// Hide it after a few seconds:
	window.clearTimeout(hideErrorTimeout);
	// hideErrorTimeout = window.setTimeout(function() {$('#error-box').hide();}, 5000);
	clearServerTimeout();
}