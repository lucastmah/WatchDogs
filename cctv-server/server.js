"use strict";
// source from gabrieltanner.org/blog/webrtc-video-broadcast/

const PORT_NUMBER = 8088;

const path = require("path");
const express = require("express");
const app = express();

const http = require("http");
const child = require('child_process');
const server = http.createServer(app);

app.get('/', (req, res) => {
	res.sendFile(path.join(__dirname, '/public/index.html'));
});

app.get('/audio', (req, res) => {
	
	res.writeHead(200, {"Content-Type": "audio/mp3"});

	function startNewFfm(){
		var ffm = child.spawn(
			"ffmpeg", 
			// "-f alsa -i default:CARD=C920 -fflags nobuffer -flags low_delay -probesize 32 -analyzeduration 0 -b:a 128k -f mp3 -".split(
			// "-fflags +nobuffer -flags low_delay -hide_banner -loglevel error -ar 44100 -i udp://192.168.7.2:12343 -c:a libmp3lame -b:a 128k -f mp3 -".split(
			"-f mp3 -i udp://192.168.7.2:12343 -t 60 -f mp3 -".split(
				" "
			)
		);
	
		ffm.stdout.on("data", (data) => {
			res.write(data)
		});
	
		ffm.on('close', () => {
			startNewFfm()
		})
	}

	startNewFfm();
});

app.use(express.static(path.join(__dirname, '/public')));

server.listen(PORT_NUMBER);
console.log(`listening on ${PORT_NUMBER}`);

var procServer = require('./lib/cctv_server');
procServer.listen(server);