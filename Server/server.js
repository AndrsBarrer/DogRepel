const express = require("express");
const net = require("net");
const cors = require("cors");
const os = require("os");
const express_app = express();

// Import dog API endpoints
const { router: dogRoutes } = require("./routes/dogs");
const stationRoutes = require("./routes/stations");

const dogService = require("./services/dogService");
const stationService = require("./services/stationService");
const { measureMemory } = require("vm");

port = 2222;
const host = "192.168.0.28";

const Express_tag = "Express: ";
const expressPort = 4000;

express_app.use(express.json()); // Middleware to parse JSON
express_app.use(cors()); // Enable CORS for requests from the Vue frontend
express_app.use("/api/dogs", dogRoutes); // Mount dog routes
express_app.use("/api/stations", stationRoutes); // Mount station routes

const server = net.createServer();
server.listen(port, host, () => {
  console.log("[✓] TCP Server is running on port " + port + ".");
});

let sockets = [];

server.on("connection", function (sock) {
  console.log("[!] CONNECTED: " + sock.remoteAddress + ":" + sock.remotePort);
  sockets.push(sock);

  sock.on("data", function (data) {
    //console.log("DATA " + sock.remoteAddress + ": " + data);

    // Upload the event to the database
    uploadMessage(data);
    // Write the data back to all the connected, the client will receive it as data from the server
    // sockets.forEach(function (sock, index, array) {
    //   sock.write(
    //     sock.remoteAddress + ":" + sock.remotePort + " said " + data + "\n"
    //   );
    // });
  });

  // Add a 'close' event handler to this instance of socket
  sock.on("close", function () {
    let index = sockets.findIndex(function (o) {
      return (
        o.remoteAddress === sock.remoteAddress &&
        o.remotePort === sock.remotePort
      );
    });
    if (index !== -1) sockets.splice(index, 1);
    console.log("[!] CLOSED: " + sock.remoteAddress + " " + sock.remotePort);
  });
});

const uploadMessage = async (message) => {
  try {
    console.log("[+] Uploading new event.");
    // EVENT-CC:7B:5C:35:6D:B4-CC:DB:A7:5A:56:D4
    var msg = String(message).split("/");
    const station_mac = msg[1];
    const collar_mac = msg[2];
    const rssi = Number(msg[3]);

    let stationResults = await stationService.getStationByMac(station_mac);
    if (stationResults.length == 0) {
      await stationService.createStationByMac(station_mac);
    }

    // Check if the MAC address of the station is already registered (GET /stations)
    let collarResults = await dogService.getDogByMac(collar_mac);
    if (collarResults.length == 0) {
      await dogService.createDogByMac(collar_mac);
    }

    // Add the event to db
    await stationService.createDogVisit(station_mac, collar_mac, rssi);
  } catch (error) {
    console.log(error);
  }
};

express_app.listen(expressPort, () => {
  console.log(`[✓] Express Server running on http://localhost:${expressPort}`);
});
