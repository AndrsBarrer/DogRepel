<template>
  <div class="card">
    <DataTable
      :value="visits"
      paginator
      :rows="5"
      :rowsPerPageOptions="[5, 10, 20, 50]"
      tableStyle="min-width: 50rem"
    >
      <Column field="dog_name" header="Dog" style="width: 25%"></Column>
      <Column field="location" header="Station" style="width: 25%"></Column>
      <Column field="formattedTime" header="Time" style="width: 25%"></Column>
    </DataTable>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted } from "vue";
import DogService from "../services/DogService";
let intervalId: NodeJS.Timeout;

const fetchVisits = async () => {
  DogService.getDogVisits().then((result) => {
    const formattedData = result.data.map((visit) => {
      const date = new Date(visit.visit_time);

      let timeOfDay = "AM";

      // Get the date first
      const day = date.getDay().toString().padStart(2, "0");
      const month = date.getMonth().toString().padStart(2, "0");
      const year = date.getFullYear().toString().padStart(2, "0");

      // Get the time next
      let hours = date.getHours();
      if (hours >= 12) {
        hours -= 12;
        timeOfDay = "PM";
      } else if (hours === 0) {
        hours = 12; // For midnight, display as 12 AM
      }

      toString().padStart(2, "0");
      const minutes = date.getMinutes().toString().padStart(2, "0");
      const seconds = date.getSeconds().toString().padStart(2, "0");

      return {
        ...visit,
        formattedTime: `${day}/${month}/${year} ${hours}:${minutes}:${seconds} ${timeOfDay}`,
      };
    });
    visits.value = formattedData;
  });
};

onMounted(() => {
  fetchVisits(); // Initial fetch
  intervalId = setInterval(fetchVisits, 5000); // Poll every 5 seconds
});

const visits = ref();
</script>

<style scoped lang="scss">
.title {
  text-align: center;
  margin-top: 2rem;
  font-size: 25px;
}
</style>
