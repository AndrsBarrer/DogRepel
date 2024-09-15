const express = require("express");
const router = express.Router();
const mysql = require("mysql2");
const db = require("../db"); // Import the centralized DB connection

const isNumber = (value) => Number.isFinite(value);

const isFloat = (value) => {
  return Number(value) === value && value % 1 !== 0;
};

// Function to upload message to the MySQL database
const uploadMessage = async (message) => {
  var msg = message.split(":");
  console.log(msg);

  //DogRepel:DogID:StationID:Distance:Comment
  let query = `INSERT INTO dog_visits (dog_id, station_id, distance) VALUES (${
    msg[1]
  }, ${msg[2]}, ${parseFloat(msg[3])});`;

  db.query(query, (err, result) => {
    if (err) throw err;
    console.log(
      `Visit with Dog ID:${msg[1]}, Station ID: ${msg[2]}, and distance: ${msg[3]} inserted.`
    );
  });
};

// Define routes for handling API requests
router.get("/", async (req, res) => {
  try {
    const [results] = await db.query("SELECT * FROM dogs");
    res.status(200).json(results);
  } catch (err) {
    console.error("SQL Error:", err);
    res.status(500).json({ error: "Failed to fetch dogs" });
  }
});

router.post("/", async (req, res) => {
  try {
    const { name, breed, age } = req.body;
    console.log("api got: " + name, breed, age);
    // Convert age to a number and validate
    const ageNumber = Number(age);

    // Check for missing fields and validate age
    if (!name || !breed || !Number.isFinite(ageNumber)) {
      return res.status(400).json({
        error: "Valid name, breed, and age required.",
      });
    }

    // Proceed with the database insertion
    try {
      const query = "INSERT INTO dogs (name, breed, age) VALUES (?, ?, ?)";
      const [result] = await db.query(query, [name, breed, ageNumber]);

      res.status(200).json({
        message: "Success",
        name: name,
        breed: breed,
        age: ageNumber,
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

    // Proceed with the database insertion
    try {
      const query =
        "INSERT INTO dog_settings (dog_id, alert_distance) VALUES (?, ?)";
      const [result] = await db.query(query, [dog_id, alert_distance]);

      res.status(200).json({
        message: "Success",
        distance: alert_distance,
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

router.get("/dog_visits", async (req, res) => {
  try {
    const [results] = await db.query("SELECT * FROM dog_visits");
    res.json(results);
  } catch (err) {
    console.error("SQL Error:", err);
    res.status(500).json({ error: "Failed to fetch dog visits" });
  }
});

// Export the router and the uploadMessage function for UDP usage
module.exports = {
  router,
  uploadMessage,
};
