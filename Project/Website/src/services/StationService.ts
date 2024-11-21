import axios from "axios";

export interface Station {
  station_id: number;
  location: string;
  category: string;
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

  async getStation(station_id: number) {
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

  async getStationByLocation(location: string) {
    try {
      const response = await apiClient.get("/stations/stationLocation", {
        params: { location },
      });
      return response.data;
    } catch (error) {
      console.error("Error fetching location:", error);
      throw error;
    }
  },

  async getStationSettings() {
    try {
      const response = await apiClient.get("/stations/station_settings");
      return response.data;
    } catch (error) {
      console.error("Error fetching station settings:", error);
      throw error;
    }
  },

  async updateStationInfo(data: Station) {
    try {
      const response = await apiClient.put("/stations", data);
      return response;
    } catch (error) {
      console.log("An error occurred updating station: ", error);
      throw error;
    }
  },
};

export default StationService;
