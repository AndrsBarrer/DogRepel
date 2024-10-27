<!-- src/views/HomeView.vue -->
<template>
  <main>
    <div class="page-container">
      <ChartComponent />
      <BubbleChartComponent />

      <div class="tables-wrapper">
        <div class="tables">
          <EditDogsTable />
          <EditStationsTable />
        </div>
      </div>
    </div>
  </main>
</template>

<script setup lang="ts">
import ChartComponent from "../components/ChartComponent.vue";
import BubbleChartComponent from "../components/BubbleChartComponent.vue";
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

  .tables-wrapper {
    margin: 0 2rem; /* Apply margin to the wrapper */
  }

  .tables {
    display: flex;
    width: 100%;
    gap: 2rem;
    flex-direction: row;
    justify-content: center;

    /* Ensure each child div (e.g., EditDogsTable and EditStationsTable) takes up 50% of the container */
    & > div {
      flex: 1 1 40%; /* Allow flexibility in width */
      max-width: 50%; /* Maximum width constraint */
    }
  }
}
</style>
