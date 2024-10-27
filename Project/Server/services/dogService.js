// dogService.js
const db = require("../db");

const getAllDogs = async () => {
  return await db.query("SELECT * FROM dogs");
};

const insertDog = async (name, breed, age) => {
  const query = "INSERT INTO dogs (name, breed, age) VALUES (?, ?, ?)";
  return await db.query(query, [name, breed, age]);
};

const updateDog = async (dog_id, name, breed, age) => {
  const query = "UPDATE dogs SET name = ?, breed = ?, age = ? WHERE dog_id = ?";
  return await db.query(query, [name, breed, age, dog_id]);
};

const getDogByMac = async (mac) => {
  try {
    const [results] = await db.query("SELECT * FROM dogs WHERE mac = ?", [mac]);
    return results;
  } catch (error) {
    console.log("Error: ", error);
  }
};

const createDogByMac = async (mac) => {
  try {
    await db.query("INSERT INTO dogs (mac) VALUES (?)", [mac]);
  } catch (error) {
    console.log("Error: ", error);
  }
};
module.exports = {
  getAllDogs,
  insertDog,
  updateDog,
  getDogByMac,
  createDogByMac,
};
