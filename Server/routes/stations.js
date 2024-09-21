const express = require("express");
const router = express.Router();
const mysql = require("mysql2");
const db = require("../db"); // Import the centralized DB connection

router.get("/", async (req, res) => {
  try {
    const [results] = await db.query("SELECT * FROM stations");
    res.json(results);
  } catch (err) {
    console.error("SQL Error:", err);
    res.status(500).json({ error: "Failed to fetch stations" });
  }
});

router.post("/", async (req, res) => {
  try {
    const { location } = req.body;
    let query = "INSERT INTO stations (location) VALUES (?)";
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
    const { station_id, location } = req.body;
    let query = "UPDATE stations SET location = ? WHERE station_id = ?";
    const [result] = await db.query(query, [location, station_id]);
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

router.get("/station_settings", async (req, res) => {
  try {
    const [results] = await db.query("SELECT * FROM station_settings");
    res.json(results);
  } catch (err) {
    console.error("SQL Error:", err);
    res.status(500).json({ error: "Failed to fetch station settings" });
  }
});

router.post("/station_settings", async (req, res) => {
  try {
    const { station_id, sensitivity, camera_activation } = req.body;
    let query =
      "INSERT INTO station_settings (station_id, sensitivity, camera_activation) VALUES (?, ?, ?)";
    const [result] = await db.query(query, [
      station_id,
      sensitivity,
      camera_activation,
    ]);
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

router.put("/station_settings", async (req, res) => {
  try {
    const { station_id, sensitivity, camera_activation } = req.body;
    let query =
      "UPDATE station_settings SET sensitivity = ?, camera_activation = ? WHERE station_id = ?";
    const [result] = await db.query(query, [
      sensitivity,
      camera_activation,
      station_id,
    ]);
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

// Export the router and the uploadMessage function for UDP usage
module.exports = router;
