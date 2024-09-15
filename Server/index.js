/*
The purpose of this is server is:
- receive packets from ESP32 
- send back an ACK to confirm reception
- process information and upload to database
*/

/*
Definition of message standards to know what has hexpress_appened or what should hexpress_appen.

Dogs will be registered on the website, unsure right now if the website backend will
register the dogs on the database or if the server should take care of everything.

Messages that server receives: 

Dog detected near station:
    DogRepel:DogID:StationID:Distance:Comment

*/

const express = require("express");
const dgram = require("dgram");
const cors = require("cors");
const os = require("os");
const express_app = express();

// Import dog API endpoints
const { router: dogRoutes, uploadMessage } = require("./routes/dogs");
const stationRoutes = require("./routes/stations"); // Import station routes
const expressPort = 4000;

const Express_tag = "Express: ";
const UDP_tag = "UDP: ";

express_app.use(express.json()); // Middleware to parse JSON
express_app.use(cors()); // Enable CORS for requests from the Vue frontend
express_app.use("/api/dogs", dogRoutes); // Mount dog routes
express_app.use("/api/stations", stationRoutes); // Mount station routes

// Create a UDP server
const server = dgram.createSocket("udp4");

// emits when any error occurs
server.on("error", (error) => {
  console.log("Error: " + error);
  server.close();
});

// emits on new datagram msg
server.on("message", (msg, info) => {
  console.log(UDP_tag + "Data received from client: " + msg.toString());
  console.log(
    UDP_tag + "Received %d bytes from %s:%d",
    msg.length,
    info.address,
    info.port
  );

  // Send acknowlegement back to ESP32 of message reception
  server.send("ACK", info.port, info.address, (error) => {
    if (error) {
      console.error(UDP_tag + "Error sending message:", error);
    } else {
      console.log(UDP_tag + "Verified packet sent to ESP32.");
    }
  });

  // Takes message, parses it, writes it to the database
  uploadMessage(msg.toString());
});

// Start UDP server
server.on("listening", () => {
  const address = server.address();
  const port = address.port;
  const family = address.family;
  const ipaddr = address.address;

  console.log(UDP_tag + "Server is listening at port " + port);
  console.log(UDP_tag + "Server IP address (from server.address()): " + ipaddr);
  console.log(UDP_tag + "Server is IP4/IP6: " + family);

  // Display all network interfaces' IP addresses (for now to know local IP address, should set server on static IP later)
  const networkInterfaces = os.networkInterfaces();
  for (const [interfaceName, addresses] of Object.entries(networkInterfaces)) {
    addresses.forEach((addressInfo) => {
      if (addressInfo.family === "IPv4" && !addressInfo.internal) {
        console.log(
          UDP_tag + `Network Interface ${interfaceName}: ${addressInfo.address}`
        );
      }
    });
  }
});

express_app.listen(expressPort, () => {
  console.log(
    Express_tag + `Server running on http://localhost:${expressPort}`
  );
});

// Bind to a port from environment variable or default to 2222
const port = process.env.UDP_PORT || 2222;
server.bind(port);
