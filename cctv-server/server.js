"use strict";
// source from gabrieltanner.org/blog/webrtc-video-broadcast/

const PORT_NUMBER = 8088;

const path = require("path");
const express = require("express");
const app = express();

const http = require("http");
const server = http.createServer(app);

app.get('/', (req, res) => {
	res.sendFile(path.join(__dirname, '/public/index.html'));
});

app.use(express.static(path.join(__dirname, '/public')));

server.listen(PORT_NUMBER);
console.log(`listening on ${PORT_NUMBER}`);

var procServer = require('./lib/cctv_server');
procServer.listen(server);