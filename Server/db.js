const mysql = require('mysql2');
require('dotenv').config();

// Create a MySQL connection pool (recommended for handling multiple connections)
const pool = mysql.createPool({
    host: process.env.DB_HOST,
    user: process.env.DB_USER,
    password: process.env.DB_PASSWORD,
    database: process.env.DB_NAME,
    waitForConnections: true,
    connectionLimit: 10,
    queueLimit: 0
});

// Export the pool to be used in other files
module.exports = pool.promise();  // Export as a promise for async/await
