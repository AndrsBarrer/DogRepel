import axios from "axios";

interface station {
  station_id: Number;
  location: String;
}

// Create an instance of Axios with a base URL if needed
const apiClient = axios.create({
  baseURL: import.meta.env.VITE_API_BASE_URL,
  headers: {
    "Content-Type": "application/json",
  },
});

const StationService = {
  async getStations() {
    try {
      const response = await apiClient.get("/stations");
      return response.data;
    } catch (error) {
      console.error("Error fetching stations:", error);
      throw error;
    }
  },

  async getStation(station_id) {
    try {
      const response = await apiClient.get("/stations/station", {
        params: { station_id },
      });
      return response.data;
    } catch (error) {
      console.error("Error fetching station:", error);
      throw error;
    }
  },

  /*
  router.get("/station", async (req, res) => {
  try {
    const { station_id } = req.body;
    let query = "SELECT * FROM stations WHERE station_id = ?";
    const [result] = await db.query(query, [station_id]);
    res.json(result);
  } catch (err) {
    console.error("SQL Error:", err);
    res.status(500).json({ error: "Failed to fetch station." });
  }
});

  */
  async getStationSettings() {
    try {
      const response = await apiClient.get("/stations/station_settings");
      return response.data;
    } catch (error) {
      console.error("Error fetching station settings:", error);
      throw error;
    }
  },

  async updateStationInfo(data: station) {
    try {
      await apiClient.put("/stations", data);
    } catch (error) {
      console.log("An error occurred updating station: ", error);
      throw error;
    }
  },
};

export default StationService;
