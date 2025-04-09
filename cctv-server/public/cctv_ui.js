'use strict';

var socket = io.connect();

var mute = true;
var toggleMic = false;
var communicationsTimeout = null;
var patrolling = false;
var motion_light = false;
var currentZoom = 0;
var zoomSettings = {
	increment: 1,
	min: 0,
	max: 30,
	default: 0,
	x: 40,
	y: 30
}

var current_email = "lucastmah@gmail.com";

var currentCam = 0;
var cams = [
	{
		"id": "src1",
		"lastCheckin": 0,
		"audioSrc": "/audio",
		"iconId": "src1Icon"
	},
	{
		"id": "src2",
		"lastCheckin": 0,
		"audioSrc": "/audio1",
		"iconId": "src2Icon"
	}
]

function getTimeMs(){
	let date = new Date();
	return date.getTime()
}

function checkin(id){
	cams[id].lastCheckin = getTimeMs()
}

function updateZoom(increase){
	let change = 0;
	if(increase && currentZoom < zoomSettings.max){
		change = zoomSettings.increment;
	}else if(!increase && currentZoom > zoomSettings.min){
		change = -zoomSettings.increment;
	}
	currentZoom += change;
}

function resetZoom(){
	currentZoom = zoomSettings.default;
}

function setCam(cam) {
	currentCam = cam;
	cams.forEach(c => {
		$(`#${c.id}`).removeClass('active')
	});
	$(`#${cams[cam].id}`).addClass('active')
	resetZoom();
}

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

const setEditorVis = (mode) => {
	if(mode == true){
		$("#editorPopup").show();
	}else{
		$("#editorPopup").hide();
	}
}

$(document).ready(function() {
	setInterval(() => {
		cams.forEach(iCam => {
			if(getTimeMs() - iCam.lastCheckin > 5000){
				$(`#${iCam.id}`).addClass('offline')
			}else{
				$(`#${iCam.id}`).removeClass('offline')
			}
		});
	}, 5000);

	setEditorVis(false);

	$("#emailDisplay").html(current_email)

	$("#openEditor").on("click", () => {
		$("#emailInput").val(current_email);
		setEditorVis(true);
	})

	$("#saveEmail").on("click", () => {
		let input = $("#emailInput").val().trim()
		if(input == ""){
			return
		}
		console.log(input)
		current_email = input
  		sendCommandToServer('email', current_email)
		setEditorVis(false);
		$("#emailDisplay").html(current_email)
	})

	$("#cancelEdit").on("click", () => {
		setEditorVis(false);
	})

	cams.forEach((c, i) => {
		$(`#${c.id}`).on("click", () => {
			setCam(i)
		})
	});

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

	cams.forEach((c, i) => {
		socket.on('canvas' + i, function(data) {
			if(currentCam != i){
				return
			}
			const canvas = $("#videostream");
			const context = canvas[0].getContext('2d');
			const image = new Image();
			image.src = "data:image/jpeg;base64,"+data;
			let sX = zoomSettings.x * currentZoom
			let sY = zoomSettings.y * currentZoom
			image.onload = function() {
				context.height = image.height;
				context.width = image.width;
				context.drawImage(image,0 - sX / 2,0 - sY / 2,context.width + sX, context.height + sY);
			}
		});
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
		updateZoom(true)
	});
	$('#zoomOutBtn').click(function() {
		updateZoom(false)
	});
	$('#speakerBtn').click(function() {
		mute = !mute;
		let aPlayer = document.getElementById("audioPlayer");
		// console.log(aPlayer.currentTime);
		// console.log(aPlayer.duration);
		
		if (mute) {
			$('#speakerIcon').attr("src", "./speakerMute.svg")
			console.log("mute!");
			// sendCommandToServer('mute', "1");
			aPlayer.pause();
		}
		else {
			$('#speakerIcon').attr("src", "./speaker.svg")
			console.log("unmute!");
			// sendCommandToServer('mute', "0");
			aPlayer.play();
			if(aPlayer.duration != Infinity && aPlayer.duration != NaN && aPlayer.duration > 0){
				aPlayer.currentTime = aPlayer.duration - (2 * (aPlayer.duration > 2)? 1 : 0);
			}
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
	
	socket.on('camStatus', (message) => {
		checkin(message);
		console.log(message)
	})

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