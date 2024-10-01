<!-- src/views/HomeView.vue -->
<template>
  <main>
    <div class="page-container">
      <ChartComponent />
      <div class="tables">
        <EditDogsTable />
        <EditStationsTable />
      </div>
    </div>
  </main>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from "vue";
import DogService from "../services/DogService";

import ChartComponent from "../components/ChartComponent.vue";
import MenuComponent from "../components/MenuComponent.vue";
import RegisterDog from "../components/RegisterDog.vue";
import RegisterStation from "../components/RegisterStation.vue";
import DataTableComponent from "../components/DataTableComponent.vue";
import EditDogsTable from "../components/EditDogsTable.vue";
import EditStationsTable from "../components/EditStationsTable.vue";
</script>

<style scoped>
main {
  padding: 20px;
}
.page-container {
  display: flex;
  flex-direction: column;

  .tables {
    display: flex;
    width: 100%;
    gap: 4rem;
    flex-direction: row;
    justify-content: center;

    /* Ensure each child div (e.g., EditDogsTable and EditStationsTable) takes up 50% of the container */
    & > div {
      /* flex: 1 1 50%:
         - 1 (grow): allows the div to grow if there is extra space
         - 1 (shrink): allows the div to shrink if space is limited
         - 50% (basis): initially sets the width of each div to 50% of the row2 container */
      flex: 1 1 50%;

      /* max-width: 50% ensures that even if the div grows, it will not exceed 50% of the parent width */
      max-width: 50%;
    }
  }

  .row2-register {
    display: flex;
    flex-direction: row;
    justify-content: center;
    width: 100%;
  }
}
</style>
