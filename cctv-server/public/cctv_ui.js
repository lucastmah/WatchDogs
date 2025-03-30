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
	
	// Setup the button clicks:
	$('#panLeft').click(function() {
		console.log("pan left!");
		sendCommandToServer('pan', "0");
	});
	$('#panRight').click(function() {
		console.log("pan right!");
		sendCommandToServer('pan', "1");
	});
	$('#panUp').click(function() {
		console.log("pan up!");
		sendCommandToServer('pan', "2");
	});
	$('#panDown').click(function() {
		console.log("pan down!");
		sendCommandToServer('pan', "3");
	});

	// Set up button holds:
	setupHoldButton('#panLeft', () => sendCommandToServer('pan', "-1"));
	setupHoldButton('#panRight', () => sendCommandToServer('pan', "1"));
	setupHoldButton('#panUp', () => sendCommandToServer('tilt', "1"));
	setupHoldButton('#panDown', () => sendCommandToServer('tilt', "-1"));

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
			console.log("unmute!");
			sendCommandToServer('mute', "0");	
		}
		else {
			console.log("mute!");
			sendCommandToServer('mute', "1");
		}
	});
	$('#pushToTalk').click(function() {
		console.log("toggle mic!");
		toggleMic = !toggleMic;
		if (toggleMic) {
			sendCommandToServer('talk', "1");	
		}
		else {
			sendCommandToServer('talk', "0");
		}
	});
});

var hideErrorTimeout;
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
	
	$('#error-text').html(message);	
	$('#error-box').show();
	
	// Hide it after a few seconds:
	window.clearTimeout(hideErrorTimeout);
	hideErrorTimeout = window.setTimeout(function() {$('#error-box').hide();}, 5000);
	clearServerTimeout();
}