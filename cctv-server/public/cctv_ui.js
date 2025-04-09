'use strict';

var socket = io.connect();

var mute = true;
var toggleMic = false;
var communicationsTimeout = null;
var patrolling = false;
var motion_light = false;

var lastCheckin = [
	{
		"lastCheckin": 0,
		"audioSrc": "/audio",
		"iconId": "src1Icon"
	},
	{

	}
]


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

function updatePatrolButton() {
	$("#patrolToggle").toggleClass("active", patrolling);
}

function updateNightLightButton() {
	$("#nightLightToggle").toggleClass("active", motion_light);
}

const PatrolSwitchToggle = (enable) => {
	if (enable && !patrolling) {
		console.log("starting patrol mode!");
		sendCommandToServer('patrol', "1");	
	}
	else if (!enable && patrolling){
		console.log("stopping patrol mode!");
		sendCommandToServer('patrol', "0");
	}
	updatePatrolButton();
}

const NightLightToggle = () => {
	if (motion_light){
		console.log("turn off motion light!");
		sendCommandToServer('motion_light', "0");
	} else {
		console.log("turn on motion light!");
		sendCommandToServer('motion_light', "1");	
	}
	motion_light = !motion_light;
	updateNightLightButton();
}

$(document).ready(function() {

	$("#patrolToggle").on("click", () => {
		let mode = !patrolling;
		PatrolSwitchToggle(mode);
	})

	$("#nightLightToggle").on("click", () => {
		NightLightToggle();
	})

	// Setup a repeating function (every 1s)
	window.setInterval(function() {
		sendCommandToServer('patrol', "null")
		updatePatrolButton();
	}, 1000);
	window.setInterval(function() {
		sendCommandToServer('motion_light', "null")
		updateNightLightButton();
	}, 1000);

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
	setupHoldButton('#camLeft', () => {
		PatrolSwitchToggle(false);
		sendCommandToServer('pan', "-1")
	});
	setupHoldButton('#camRight', () => {
		PatrolSwitchToggle(false);
		sendCommandToServer('pan', "1")
	});
	setupHoldButton('#camUp', () => {
		PatrolSwitchToggle(false);
		sendCommandToServer('tilt', "1")
	});
	setupHoldButton('#camDown', () => {
		PatrolSwitchToggle(false);
		sendCommandToServer('tilt', "-1")
	});

	// Setup the button clicks:
	$('#zoomInBtn').click(function() {
		console.log("zoom in!");
		sendCommandToServer('zoom', "0");
	});
	$('#zoomOutBtn').click(function() {
		console.log("zoom out!");
		sendCommandToServer('zoom', "1");
	});
	$('#speakerBtn').click(function() {
		mute = !mute;
		if (mute) {
			$('#speakerIcon').attr("src", "./speakerMute.svg")
			console.log("mute!");
			sendCommandToServer('mute', "1");	
		}
		else {
			$('#speakerIcon').attr("src", "./speaker.svg")
			console.log("unmute!");
			sendCommandToServer('mute', "0");
		}
	});
	$('#micBtn').click(function() {
		console.log("toggle mic!");
		toggleMic = !toggleMic;
		if (toggleMic) {
			$('#micIcon').attr("src", "./mic.svg")
			console.log("talk on!");
			sendCommandToServer('talk', "1");	
		}
		else {
			$('#micIcon').attr("src", "./micMute.svg")
			console.log("talk off!");
			sendCommandToServer('talk', "0");
		}
	});
	$('#terminateProgram').click(function() {
		console.log("stopping!");
		sendCommandToServer('stop');
	});
});

var hideErrorTimeout;

function setupServerMessageHandlers(socket) {
	// Hide error display:
	$('#error-box').addClass("clearError");
	
	
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
		if (message !== undefined) {
			patrolling = Number(message) === 1;
		}
		clearServerTimeout();
	});

	socket.on('motion_light-reply', function(message) {
		console.log("Receive Reply: motion_light-reply " + message);
		if (message !== undefined) {
			motion_light = Number(message) === 1;
			console.log(motion_light);
		}
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
	$('#error-box').removeClass("clearError");
	
	// Hide it after a few seconds:
	window.clearTimeout(hideErrorTimeout);
	hideErrorTimeout = window.setTimeout(function() {$('#error-box').addClass("clearError");}, 3000);
	clearServerTimeout();
}