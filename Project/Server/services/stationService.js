// dogService.js
const db = require("../db");

const getAllStations = async () => {
  return await db.query("SELECT * FROM stations");
};

const insertStation = async (location) => {
  const query = "INSERT INTO stations (location) VALUES (?)";
  return await db.query(query, [location]);
};

const updateStation = async (station_id, location, category) => {
  const query = `
      UPDATE stations 
      SET location = ?, category = ?
      WHERE station_id = ?
    `;
  const values = [location, category, station_id];

  const [result] = await db.query(query, values);
  return result;
};

const getStationByMac = async (mac) => {
  try {
    const [rows] = await db.query(
      `SELECT 
         s.station_id,
         s.mac,
         s.location,
         rc.category AS label,
         rc.threshold AS value
       FROM 
         stations s
       JOIN 
         rssi_categories rc ON s.category = rc.category
       WHERE 
         s.mac = ?`,
      [mac]
    );

    if (rows.length === 0) {
      return { error: "No station found with the given MAC address." };
    }

    const result = rows.map((row) => ({
      station_id: row.station_id,
      mac: row.mac,
      location: row.location,
      category: {
        label: row.label,
        value: row.value,
      },
    }));

    return result[0];
  } catch (error) {
    console.log("Error: ", error);
  }
};

const createStationByMac = async (mac) => {
  try {
    await db.query("INSERT INTO stations (mac, category) VALUES (?, ?)", [
      mac,
      "MEDIUM",
    ]);
  } catch (error) {
    console.log("Could not create station: ", error);
  }
};

const getDogVisits = async () => {
  return await db.query("SELECT * FROM dog_visits");
};

const createDogVisit = async (station_mac, collar_mac, distance) => {
  try {
    let [results] = await db.query("SELECT name FROM dogs WHERE mac = ?", [
      collar_mac,
    ]);
    const dog_name = results[0].name;

    // Get the station's location and station_id based on the station_mac
    [results] = await db.query(
      "SELECT location, station_id FROM stations WHERE mac = ?",
      [station_mac]
    );
    const location = results[0].location;
    const station_id = results[0].station_id;

    // Insert the dog visit, including the station_id
    await db.query(
      "INSERT INTO dog_visits (dog_name, location, station_id, distance) VALUES (?,?,?,?)",
      [dog_name, location, station_id, distance]
    );
  } catch (error) {
    console.log("Could not create event: ", error);
  }
};

const getDefaultDistance = async () => {
  const [result] = await db.query(
    "SELECT threshold FROM `rssi_categories` WHERE category = 'MEDIUM'"
  );
  return result[0].threshold;
};
module.exports = {
  getAllStations,
  insertStation,
  updateStation,
  getStationByMac,
  createStationByMac,
  getDogVisits,
  createDogVisit,
  getDefaultDistance,
};
