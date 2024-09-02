/* Main server code
- Creates a UDP server that will receive UDP packets from ESP32
- Stores into a MySQL database
- Has an API that can be used by the website to request information
*/
const dgram = require('dgram');
const mysql = require('mysql2');
require('dotenv').config();

// Create UDP server
const server = dgram.createSocket('udp4');

// Database connection
const db = mysql.createConnection({
    host: process.env.DB_HOST,
    user: process.env.DB_USER,
    password: process.env.DB_PASSWORD,
    database: process.env.DB_NAME
});

db.connect(err => {
    if (err) {
        console.error('Database connection failed:', err.stack);
        return;
    }
    console.log('Connected to database.');
});

// Handle incoming UDP messages
server.on('message', (msg, rinfo) => {
    console.log(`Received message: ${msg} from ${rinfo.address}:${rinfo.port}`);
    
    // Assuming the message is in JSON format
    const data = JSON.parse(msg);
    
    const query = 'INSERT INTO events (device_name, location, timestamp) VALUES (?, ?,
