const express = require('express');
const router = express.Router();
const mysql = require('mysql2');
const db = require('../db');  // Import the centralized DB connection

// Define routes for handling API requests
router.get('/', async (req, res) => {
    try {
        const [results] = await db.query('SELECT * FROM stations');
        res.json(results);
    } catch (err) {
        console.error("SQL Error:", err);
        res.status(500).json({ error: 'Failed to fetch stations' });
    }
});

router.get('/settings', async (req, res) => {
    try {
        const [results] = await db.query('SELECT * FROM station_settings');
        res.json(results);
    } catch (err) {
        console.error("SQL Error:", err);
        res.status(500).json({ error: 'Failed to fetch station settings' });
    }
});

// Export the router and the uploadMessage function for UDP usage
module.exports = router;