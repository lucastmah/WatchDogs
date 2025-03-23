var socket = io.connect();

socket.on('image', (data) => {
	console.log('data', data);
	// const imageElm = document.getElementById('image');
	// image.src = `data:image/jpeg;base64,${image}`;
});

mute = true;
toggleMic = false;

$(document).ready(function() {
	// Setup the button clicks:
	$('#panLeft').click(function() {
		sendCommandToServer('pan', "0");
	});
	$('#panRight').click(function() {
		sendCommandToServer('pan', "1");
	});
	$('#panUp').click(function() {
		sendCommandToServer('pan', "2");
	});
	$('#panDown').click(function() {
		sendCommandToServer('pan', "3");
	});

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
});