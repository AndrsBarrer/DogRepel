/*
The purpose of this is server is:
- receive packets from ESP32 
- send back an ACK to confirm reception
- process information and upload to database
*/

/*
Definition of message standards to know what has happened or what should happen.

Dogs will be registered on the website, unsure right now if the website backend will
register the dogs on the database or if the server should take care of everything.

Messages that server receives: 

Dog detected near station:
    DogRepel:DogID:StationID:Distance:Comment

*/


const dgram = require('dgram');
const os = require('os');
const mysql = require('mysql2')
require('dotenv').config();

// Create a UDP server
const server = dgram.createSocket('udp4');

const db = mysql.createConnection({
    host: process.env.DB_HOST,
    user: process.env.DB_USER,
    password: process.env.DB_PASSWORD,
    database: process.env.DB_NAME
});

// Connect to MySQL database to write received messages
db.connect(err => {
    if (err) {
        console.log("Could not connect to database:", err);
    } else {
        console.log("Connected to database.");
    }
})


// emits when any error occurs
server.on('error', (error) => {
    console.log('Error: ' + error);
    server.close();
});

// emits on new datagram msg
server.on('message', (msg, info) => {
    console.log('Data received from client: ' + msg.toString());
    console.log('Received %d bytes from %s:%d', msg.length, info.address, info.port);

    // Send acknowlegement back to ESP32 of message reception 
    server.send("ACK", info.port, info.address, (error) => {
        if (error) {
            console.error('Error sending message:', error);
        } else {
            console.log('Verified packet sent to ESP32.');
        }
    });

    // Takes message, parses it, writes it to the database
    uploadMessage(msg.toString());

});

// Uploads message to MySQL database
const uploadMessage = (message) => {
    var msg = message.split(':');
    console.log(msg);
    
    //DogRepel:DogID:StationID:Distance:Comment
    // dog_id station_id
    let query = `INSERT INTO dog_visits (dog_id, station_id, distance) VALUES (${msg[1]}, ${msg[2]}, ${parseFloat(msg[3])});`
    
    //DogRepel:1:1:2.4:TestingFirstInsert
    db.query(query, (err, result) => {
        if (err) throw err;
        console.log(`Visit with Dog ID:${msg[1]}, Station ID: ${msg[2]} and distance: ${msg[3]} inserted.`)
    });
}

// Start UDP server
server.on('listening', () => {
    const address = server.address();
    const port = address.port;
    const family = address.family;
    const ipaddr = address.address;
    
    console.log('Server is listening at port ' + port);
    console.log('Server IP address (from server.address()): ' + ipaddr);
    console.log('Server is IP4/IP6: ' + family);

    // Display all network interfaces' IP addresses (for now to know local IP address, should set server on static IP later)
    const networkInterfaces = os.networkInterfaces();
    for (const [interfaceName, addresses] of Object.entries(networkInterfaces)) {
        addresses.forEach((addressInfo) => {
            if (addressInfo.family === 'IPv4' && !addressInfo.internal) {
                console.log(`Network Interface ${interfaceName}: ${addressInfo.address}`);
            }
        });
    }
});

// Bind to a port from environment variable or default to 2222
const port = process.env.UDP_PORT || 2222;
server.bind(port);
