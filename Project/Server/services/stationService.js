// dogService.js
const db = require("../db");

const getAllStations = async () => {
  return await db.query("SELECT * FROM stations");
};

const insertStation = async (location) => {
  const query = "INSERT INTO stations (location) VALUES (?)";
  return await db.query(query, [location]);
};

const updateStation = async (station_id, location) => {
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
    console.log("Error: ", error);
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

const createDogVisit = async (station_mac, collar_mac, distance) => {
  try {
    let [results] = await db.query("SELECT name FROM dogs WHERE mac = ?", [
      collar_mac,
    ]);
    const dog_name = results[0].name;

    [results] = await db.query("SELECT location FROM stations WHERE mac = ?", [
      station_mac,
    ]);
    const location = results[0].location;

    await db.query(
      "INSERT INTO dog_visits (dog_name, location, distance) VALUES (?,?,?)",
      [dog_name, location, distance]
    );
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
