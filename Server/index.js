/*
The purpose of this is server is:
- receive packets from ESP32 
- send back an ACK to confirm reception
- process information and upload to database
*/

const dgram = require('dgram');
const os = require('os');
require('dotenv').config();

// Create a UDP server
const server = dgram.createSocket('udp4');

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
});

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
