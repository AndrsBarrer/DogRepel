const express = require("express");
const router = express.Router();
const mysql = require("mysql2");
const db = require("../db"); // Import the centralized DB connection
const stationService = require("../services/stationService");

router.get("/", async (req, res) => {
  try {
    const [results] = await stationService.getAllStations();
    res.status(200).json({
      message: "Succesfully returned results.",
      results: results,
    });
  } catch (err) {
    console.error("SQL Error:", err);
    res.status(500).json({ error: "Failed to fetch stations" });
  }
});

router.post("/", async (req, res) => {
  try {
    const { location } = req.body;
    let query = await stationService.insertStation();
    const [result] = await db.query(query, [location]);
    res.status(200).json({
      message: "Succesfully inserted values.",
      result: result,
    });
  } catch (error) {
    res.status(500).json({
      error: error,
    });
  }
});

router.put("/", async (req, res) => {
  try {
    const { station_id, location, allowedDistance, category } = req.body;

    const result = await stationService.updateStation(
      station_id,
      location,
      allowedDistance,
      category
    );
    res.status(200).json({
      message: "Succesfully updated values.",
      result: result,
    });
  } catch (error) {
    res.status(500).json({
      error: error,
    });
  }
});

router.get("/station", async (req, res) => {
  try {
    const { station_id } = req.query;
    let query = "SELECT * FROM stations WHERE station_id = ?";
    const [result] = await db.query(query, [station_id]);
    res.json(result);
  } catch (err) {
    console.error("SQL Error:", err);
    res.status(500).json({ error: "Failed to fetch station." });
  }
});

router.get("/stationLocation", async (req, res) => {
  try {
    const { location } = req.query;
    let query = "SELECT * FROM stations WHERE location = ?";
    const [result] = await db.query(query, [location]);
    res.json(result);
  } catch (err) {
    console.error("SQL Error:", err);
    res.status(500).json({ error: "Failed to fetch station location." });
  }
});

// Export the router and the uploadMessage function for UDP usage
module.exports = router;
