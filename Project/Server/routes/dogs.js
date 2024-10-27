const express = require("express");
const router = express.Router();
const mysql = require("mysql2");
const db = require("../db"); // Import the centralized DB connection
const dogService = require("../services/dogService");

const isNumber = (value) => Number.isFinite(value);

// Define routes for handling API requests
router.get("/", async (req, res) => {
  try {
    const [results] = await dogService.getAllDogs();
    res.status(200).json({
      message: "Succesfully returned results.",
      results: results,
    });
  } catch (error) {
    res.status(500).json({
      message: "Failed to fetch dogs",
      error: error,
    });
  }
});

router.post("/", async (req, res) => {
  try {
    const { name, breed, age } = req.body;
    // Convert age to a number and validate
    const ageNumber = Number(age);

    // Check for missing fields and validate age
    if (!name || !breed || !Number.isFinite(ageNumber)) {
      return res.status(400).json({
        error: "Valid name, breed, and age required.",
      });
    }

    try {
      const [result] = await dogService.insertDog(name, breed, ageNumber);

      res.status(200).json({
        message: "Successfully inserted values.",
        result: result,
      });
    } catch (error) {
      res.status(500).json({
        message: "Error occurred during insertion.",
        error: error,
      });
    }
  } catch (error) {
    console.error("General error:", err); // Log other errors
    res.status(500).json({
      message: "An unexpected error occurred.",
      error: error,
    });
  }
});

router.put("/", async (req, res) => {
  const { dog_id, name, breed, age } = req.body;
  try {
    const [result] = await dogService.updateDog(dog_id, name, breed, age);
    res.status(200).json({
      message: "Succesfully updated values.",
      result: result,
    });
  } catch (err) {
    console.log(err);
    res.status(500).json({
      message: "Unable to update values.",
    });
  }
});

router.get("/dog_settings", async (req, res) => {
  try {
    const [results] = await db.query("SELECT * FROM dog_settings");
    res.status(200).json(results);
  } catch (err) {
    console.error("SQL Error:", err);
    res.status(500).json({ error: "Failed to fetch dog settings" });
  }
});

router.post("/dog_settings", async (req, res) => {
  try {
    const { dog_id, alert_distance } = req.body;

    // Check for missing fields and validate age
    if (!isNumber(dog_id) || !isNumber(alert_distance)) {
      return res.status(400).json({
        error: "Valid Dog ID and distance required.",
      });
    }

    try {
      const query =
        "INSERT INTO dog_settings (dog_id, alert_distance) VALUES (?, ?)";
      const [result] = await db.query(query, [dog_id, alert_distance]);

      res.status(200).json({
        message: "Success",
        distance: alert_distance,
      });
    } catch (error) {
      res.status(500).json({
        message: "Error occurred during insertion.",
        error: error,
      });
    }
  } catch (error) {
    res.status(500).json({
      message: "An unexpected error occurred.",
      error: error,
    });
  }
});

router.put("/dog_settings", async (req, res) => {
  const { dog_id, alert_distance } = req.body;
  let query = "UPDATE dog_settings SET alert_distance = ? WHERE dog_id = ?";
  const [result] = await db.query(query, [alert_distance, dog_id]);
  res.status(200).json({
    message: "Succesfully updated values.",
    result: result,
  });
  try {
  } catch (error) {
    res.status(500).json({
      error: error,
    });
  }
});

router.get("/dog_visits", async (req, res) => {
  try {
    const [results] = await db.query("SELECT * FROM dog_visits");
    res.status(200).json(results);
  } catch (err) {
    console.error("SQL Error:", err);
    res.status(500).json({ error: "Failed to fetch dog visits" });
  }
});

router.post("/dog_visits", async (req, res) => {
  try {
    const { dog_id, station_id, distance } = req.body;

    // Check for missing fields and validate age
    // TODO: validate stationID
    if (!isNumber(dog_id) || !isNumber(distance)) {
      return res.status(400).json({
        error: "Valid Dog ID and distance required.",
      });
    }

    // Proceed with the database insertion
    try {
      const query =
        "INSERT INTO dog_visits (dog_id, station_id, distance) VALUES (?, ?, ?)";
      const [result] = await db.query(query, [dog_id, station_id, distance]);

      res.status(200).json({
        message: "Success",
        result: result,
      });
    } catch (queryError) {
      console.error("Database query error:", queryError); // Log the database query error
      res.status(500).json({
        error: "Error occurred during insertion.",
      });
    }
  } catch (err) {
    console.error("General error:", err); // Log other errors
    res.status(500).json({
      error: "An unexpected error occurred.",
    });
  }
});

router.put("/dog_visits", async (req, res) => {
  const { visit_id, dog_id, station_id, visit_time, distance } = req.body;
  try {
    let query =
      "UPDATE dog_visits SET dog_id = ?, station_id = ?, visit_time = ?, distance = ? WHERE visit_id = ?";
    const [result] = await db.query(query, [
      dog_id,
      station_id,
      visit_time,
      distance,
      visit_id,
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
module.exports = {
  router,
};
