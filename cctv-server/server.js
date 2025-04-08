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
	var ffm = child.spawn(
		"ffmpeg", 
		"-hide_banner -loglevel error -ar 44100 -f alsa -i default:CARD=C920 -b:a 128k -f webm -".split(
			" "
		)
	);
	
	res.writeHead(200, {"Content-Type": "audio/webm;codecs=vorbis"});
	ffm.stdout.on("data", (data) => {
		res.write(data)
	});
});

app.use(express.static(path.join(__dirname, '/public')));

server.listen(PORT_NUMBER);
console.log(`listening on ${PORT_NUMBER}`);

var procServer = require('./lib/cctv_server');
procServer.listen(server);