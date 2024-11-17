import axios from "axios";

export interface Dog {
  dog_id?: number;
  name: string;
  breed: string;
  age: number;
}

export interface Visit {
  distance: number;
  dog_id: number;
  dog_name: string;
  location: string;
  station_id: number;
  visit_id: number;
  visit_time: string;
}

// Create an instance of Axios
const apiClient = axios.create({
  baseURL: import.meta.env.VITE_API_BASE_URL,
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

  async registerDog(data: Dog) {
    try {
      await apiClient.post("/dogs", data);
    } catch (error) {
      console.log("An error occurred inserting dog: ", error);
      throw error;
    }
  },

  async getDogVisits() {
    try {
      const response = await apiClient.get("/dogs/dog_visits");
      return response;
    } catch (error) {
      console.error("Error fetching dog visits:", error);
      throw error;
    }
  },

  async updateDogInfo(data: Dog) {
    try {
      await apiClient.put("/dogs", data);
    } catch (error) {
      console.log("An error occurred updating dog: ", error);
      throw error;
    }
  },
};

export default DogService;
