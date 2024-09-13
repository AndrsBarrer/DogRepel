const express = require('express');
const router = express.Router();
const mysql = require('mysql2');
const db = require('../db');  // Import the centralized DB connection

// Function to upload message to the MySQL database
const uploadMessage = async (message) => {
    var msg = message.split(':');
    console.log(msg);
    
    //DogRepel:DogID:StationID:Distance:Comment
    let query = `INSERT INTO dog_visits (dog_id, station_id, distance) VALUES (${msg[1]}, ${msg[2]}, ${parseFloat(msg[3])});`
    
    db.query(query, (err, result) => {
        if (err) throw err;
        console.log(`Visit with Dog ID:${msg[1]}, Station ID: ${msg[2]}, and distance: ${msg[3]} inserted.`);
    });
};


// Define routes for handling API requests
router.get('/', async (req, res) => {
    try {
        const [results] = await db.query('SELECT * FROM dogs');
        res.json(results);
    } catch (err) {
        console.error("SQL Error:", err);
        res.status(500).json({ error: 'Failed to fetch dogs' });
    }
});

router.get('/dog_settings', async (req, res) => {
    try {
        const [results] = await db.query('SELECT * FROM dog_settings');
        res.json(results);
    } catch (err) {
        console.error("SQL Error:", err);
        res.status(500).json({ error: 'Failed to fetch dog settings' });
    }
});

router.get('/dog_visits', async (req, res) => {
    try {
        const [results] = await db.query('SELECT * FROM dog_visits');
        res.json(results);
    } catch (err) {
        console.error("SQL Error:", err);
        res.status(500).json({ error: 'Failed to fetch dog visits' });
    }
});


// Export the router and the uploadMessage function for UDP usage
module.exports = {
    router,
    uploadMessage
};