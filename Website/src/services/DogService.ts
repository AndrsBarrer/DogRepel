import axios from "axios";

// Create an instance of Axios with a base URL if needed
const apiClient = axios.create({
  baseURL: "http://localhost:4000/api",
  headers: {
    "Content-Type": "application/json",
  },
});

const DogService = {
  async getDogs() {
    try {
      const response = await apiClient.get("/dogs");
      return response.data;
    } catch (error) {
      console.error("Error fetching dogs:", error);
      throw error; // Rethrow the error to handle it where the service is called
    }
  },
};

export default DogService;
