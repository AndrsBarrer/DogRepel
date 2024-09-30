// dogService.js
const db = require("../db");

const getAllStations = async () => {
  return await db.query("SELECT * FROM stations");
};

const insertStation = async (location) => {
  const query = "INSERT INTO stations (location) VALUES (?)";
  return await db.query(query, [location]);
};

const updateStation = async (location, station_id) => {
  const query = "UPDATE stations SET location = ? WHERE station_id = ?";
  return await db.query(query, [location, station_id]);
};

const getStationByMac = async (mac) => {
  try {
    const [results] = await db.query("SELECT * FROM stations WHERE mac = ?", [
      mac,
    ]);
    return results;
  } catch (error) {
    console.log("Dog does not exist.");
  }
};

const createStationByMac = async (mac) => {
  try {
    await db.query("INSERT INTO stations (mac) VALUES (?)", [mac]);
  } catch (error) {
    console.log("Could not create station: ", error);
  }
};

const getDogVisits = async () => {
  return await db.query("SELECT * FROM dog_visits");
};

const createDogVisit = async (station_mac, collar_mac) => {
  try {
    let [results] = await db.query("SELECT dog_id FROM dogs WHERE mac = ?", [
      collar_mac,
    ]);
    const dog_id = results[0].dog_id;

    [results] = await db.query(
      "SELECT station_id FROM stations WHERE mac = ?",
      [station_mac]
    );
    const station_id = results[0].station_id;

    await db.query("INSERT INTO dog_visits (dog_id, station_id) VALUES (?,?)", [
      dog_id,
      station_id,
    ]);
  } catch (error) {
    console.log("Could not create event: ", error);
  }
};
module.exports = {
  getAllStations,
  insertStation,
  updateStation,
  getStationByMac,
  createStationByMac,
  getDogVisits,
  createDogVisit,
};
