"use strict";
// source from gabrieltanner.org/blog/webrtc-video-broadcast/

const PORT_NUMBER = 8088;

const path = require("path");
const express = require("express");
const app = express();

const http = require("http");
const server = http.createServer(app);

const io = require("socket.io")(server);

app.get('/', (req, res) => {
	res.sendFile(path.join(__dirname, '/public/index.html'));
});

app.get('/cctv_ui.js', (req, res) => {
	res.sendFile(path.join(__dirname, '/public/cctv_ui.js'));
});

setInterval(() => {
	// https://youtu.be/qexy4Ph66JE?si=jz2TREgDjcaS2TSF for am image version
	// tsh.io/blog/how-to-write-video-chat-app-using-webrtc-and-nodejs/ for video version
	io.emit('image', 'some data');
}, 1000);

server.listen(PORT_NUMBER);
console.log(`listening on ${PORT_NUMBER}`);

var procServer = require('./lib/cctv_server');
procServer.listen(server);