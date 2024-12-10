const express = require("express");
const net = require("net");
const cors = require("cors");
const os = require("os");
const express_app = express();
const dgram = require("dgram"); // Import dgram for UDP

// Import dog API endpoints
const { router: dogRoutes } = require("./routes/dogs");
const stationRoutes = require("./routes/stations");

const dogService = require("./services/dogService");
const stationService = require("./services/stationService");

port = 2222;

const Express_tag = "Express: ";
const expressPort = 4000;
const udpPort = 2222; // UDP port to listen for broadcast messages
const host = "0.0.0.0"; // Listen on all interfaces

express_app.use(express.json()); // Middleware to parse JSON
express_app.use(cors()); // Enable CORS for requests from the Vue frontend
express_app.use("/api/dogs", dogRoutes); // Mount dog routes
express_app.use("/api/stations", stationRoutes); // Mount station routes

// Create UDP server
const socket = dgram.createSocket("udp4");

const macToSocket = new Map(); // Store MAC-to-socket mappings
const lastRssiValues = new Map(); // Store last known RSSI values for each MAC

socket.on("listening", function () {
  const address = socket.address();
  console.log(
    "[UDP] UDP socket listening on " + address.address + ":" + address.port
  );
});

socket.on("message", function (message, remote) {
  console.log(
    "[UDP] UDP Server received:",
    remote.address + ":" + remote.port + " - " + message
  );

  if (message.toString() === "DISCOVER_SERVER") {
    const response = "Server ACK.";
    socket.send(response, 0, response.length, remote.port, remote.address);
  }
});
socket.bind("12345");
let sockets = [];

const server = net.createServer();
server.listen(port, host, () => {
  console.log("[TCP] TCP Server is running on port " + port + ".");
});

server.on("connection", async function (sock) {
  console.log("[TCP] CONNECTED: " + sock.remoteAddress + ":" + sock.remotePort);
  sockets.push(sock);

  // This message is used to initialize the default distance that the esp should be set at (default setting)
  // Every certain amount of time, get default value from database, if value has changed,
  // then send a message to the ESP so that it can save the value in its NVS for distance tolerated

  // On the server side, it a value changes in the database, send a message to esp
  // ESP saves value into NVS
  // On bootup, ESP gets value for distance tolerance from its NVS
  // On the website, changes can be made to the database
  // This value is a default setting
  const defaultDistance = await stationService.getDefaultDistance();
  sock.write(`DogRepel/Write/${defaultDistance}`);

  sock.on("data", async function (data) {
    // Check if what was received was the MAC from the device
    if (String(data).split("/")[0] === "MAC") {
      // Save the gotten MAC from the connected device
      const saved_mac = String(data).split("/")[1].substring(0, 17);

      macToSocket.set(saved_mac, sock);

      let stationResults = await stationService.getStationByMac(saved_mac);

      if (stationResults.length === 0) {
        await stationService.createStationByMac(saved_mac);
      }

      // Start periodic checking for this device
      startPeriodicCheck(saved_mac);
    } else {
      // Upload the event to the database
      uploadMessage(data);
    }
  });

  // Add a 'close' event handler to this instance of socket
  sock.on("close", function () {
    let index = sockets.findIndex(function (o) {
      return (
        o.remoteAddress === sock.remoteAddress &&
        o.remotePort === sock.remotePort
      );
    });

    // Remove from macToSocket map
    for (let [mac, socket] of macToSocket.entries()) {
      if (socket === sock) {
        macToSocket.delete(mac);
        // Stop periodic checking for this device
        stopPeriodicCheck(mac);
        break;
      }
    }

    if (index !== -1) sockets.splice(index, 1);
    console.log("[TCP] CLOSED: " + sock.remoteAddress + " " + sock.remotePort);
  });

  // Handle connection errors
  sock.on("error", (err) => {
    console.error("Socket error:", err.message);
  });
});

// Store intervals for each MAC
const checkIntervals = new Map();

function startPeriodicCheck(mac) {
  const interval = setInterval(async () => {
    try {
      // Retrieve the station's allowed RSSI value from the database
      const stationConnectionInfo = await stationService
        .getStationByMac(mac)
        .then((station) => {
          return station[0].category; // Ensure station exists and has a value for its category
        });

      if (stationConnectionInfo.value === undefined) {
        //console.warn(`No RSSI value found for MAC ${mac}. Skipping check.`);
        return;
      }

      const socket = macToSocket.get(mac);
      if (socket) {
        const lastRssi = lastRssiValues.get(mac);

        // If the RSSI value has changed, send an update
        if (lastRssi !== stationConnectionInfo.value) {
          console.log(
            `[!] RSSI changed for MAC ${mac}: ${lastRssi} -> ${stationConnectionInfo.value}`
          );
          socket.write(`DogRepel/Update/${stationConnectionInfo.value}`);

          // Update the last known RSSI value
          lastRssiValues.set(mac, stationConnectionInfo.value);
        }
      } else {
        console.warn(`No active socket found for MAC ${mac}.`);
      }
    } catch (error) {
      console.error(`Error during periodic check for MAC ${mac}:`, error);
    }
  }, 5000); // Check every 5 seconds

  checkIntervals.set(mac, interval);
}

function stopPeriodicCheck(mac) {
  const interval = checkIntervals.get(mac);
  if (interval) {
    clearInterval(interval);
    checkIntervals.delete(mac);
  }
}

const uploadMessage = async (message) => {
  try {
    var msg = String(message).split("/");
    const station_mac = msg[1];
    const collar_mac = msg[2];
    const rssi = Number(msg[3]);
    // console.log("[+] Uploading new event:");
    // console.log("    Station MAC:", station_mac);
    // console.log("    Collar MAC:", collar_mac);
    // console.log("    RSSI:", rssi, "\n");

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

express_app.listen(expressPort, host, () => {
  console.log(`[✓] Express Server running on http://${host}:${expressPort}`);
});
