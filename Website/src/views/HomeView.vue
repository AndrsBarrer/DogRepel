<!-- src/views/HomeView.vue -->
<template>
  <main>
    <MenuComponent @selectedMenuItem="changeMenu"></MenuComponent>

    <div v-if="currentMenuItem === 'Dashboard'">
      <ChartComponent />

      <EditDogsTable />
    </div>

    <RegisterComponent v-if="currentMenuItem === 'Register'" />

    <h1 v-if="currentMenuItem === 'Settings'">Settings will show here</h1>
  </main>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from "vue";
import DogService from "../services/DogService";

import ChartComponent from "../components/ChartComponent.vue";
import MenuComponent from "../components/MenuComponent.vue";
import RegisterComponent from "../components/RegisterComponent.vue";
import DataTableComponent from "../components/DataTableComponent.vue";
import EditDogsTable from "../components/EditDogsTable.vue";

const currentMenuItem = ref("Dashboard");

const changeMenu = (payload) => {
  currentMenuItem.value = payload;
};

const dogColumns = [
  { field: "name", header: "Name" },
  { field: "breed", header: "Breed" },
  { field: "age", header: "Age" },
];

const dogs = ref<any[]>([]); // Adjust type if necessary
let intervalId: NodeJS.Timeout;

// Function to fetch and update dogs data
const fetchDogs = async () => {
  try {
    const data = await DogService.getDogs();
    dogs.value = data.results; // Adjust if the data is nested
  } catch (error) {
    console.error("Error fetching dogs:", error);
  }
};

// Fetch data when component is mounted
onMounted(() => {
  fetchDogs(); // Initial fetch
  intervalId = setInterval(fetchDogs, 5000); // Poll every 5 seconds
});

// Cleanup interval on component unmount
onUnmounted(() => {
  if (intervalId) clearInterval(intervalId);
});
</script>

<style scoped>
main {
  padding: 20px;
}

.datatable-components {
  display: flex;
  flex-direction: row;
  flex-wrap: wrap;
  gap: 20px;

  .data-table {
    flex: 1 1 45%;
    min-width: 0;
    max-width: 100%;
  }
}
</style>
