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
	setupHoldButton('#panLeft', () => sendCommandToServer('pan', "-1"));
	setupHoldButton('#panRight', () => sendCommandToServer('pan', "1"));
	setupHoldButton('#panUp', () => sendCommandToServer('tilt', "1"));
	setupHoldButton('#panDown', () => sendCommandToServer('tilt', "-1"));

	// Setup the button clicks:
	$('#zoomIn').click(function() {
		sendCommandToServer('zoom', "0");
	});
	$('#zoomOut').click(function() {
		sendCommandToServer('zoom', "1");
	});
	$('#mute').click(function() {
		mute = !mute;
		if (mute) {
			sendCommandToServer('talk', "0");	
		}
		else {
			sendCommandToServer('talk', "1");
		}
	});
	$('#pushToTalk').click(function() {
		toggleMic = !toggleMic
		if (toggleMic) {
			sendCommandToServer('talk', "1");	
		}
		else {
			sendCommandToServer('talk', "0");
		}
	});
	$('#stop').click(function() {
		console.log("Terminating program");
		sendCommandToServer('stop', "0");
	});
});

function setupServerMessageHandlers(socket) {
	// Hide error display:
	$('#error-box').hide(); 
	
	socket.on('cctv-error', errorHandler);
}

function sendCommandToServer(command, options) {
	if (communicationsTimeout == null) {
		communicationsTimeout = setTimeout(errorHandler, 1000, 
				"ERROR: Unable to communicate to HTTP server. Is nodeJS server running?");
	}
	console.log("sending to server");
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