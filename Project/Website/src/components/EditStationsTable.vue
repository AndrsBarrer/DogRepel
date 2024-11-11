<template>
  <div class="card">
    <DataTable
      v-model:editingRows="editingRows"
      :value="stations"
      editMode="row"
      dataKey="station_id"
      @row-edit-save="onRowEditSave"
      :pt="{
        table: { style: 'min-width: 15rem' },
        column: {
          bodycell: ({ state }) => ({
            style: state['d_editing']
              ? 'overflow: hidden;'
              : 'overflow: hidden;',
          }),
        },
      }"
    >
      <Column field="location" header="Location" style="width: 100px">
        <template #editor="{ data, field }">
          <InputText v-model="data[field]" class="edit-input" />
        </template>
      </Column>

      <Column
        :rowEditor="true"
        style="width: 5%; min-width: 8rem"
        bodyStyle="text-align:center"
      ></Column>
    </DataTable>
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from "vue";
import StationService from "../services/StationService";
let intervalId: NodeJS.Timeout;

const stations = ref([]);
const editingRows = ref([]);

// Used to know whether interface should be updated or not,
// to not interrupt user when they are editing the table
const stationReference = ref({});

// Function to fetch and update dogs data
const fetchStations = async () => {
  try {
    const data = await StationService.getStations();
    // Check if the data received is different than what was there before (used to not refresh UI)
    if (
      JSON.stringify(stationReference.value) !== JSON.stringify(data.results)
    ) {
      stations.value = data.results;
      stationReference.value = stations.value;
    }
  } catch (error) {
    console.error("Error fetching stations:", error);
  }
};

// Fetch data when component is mounted
onMounted(() => {
  fetchStations(); // Initial fetch
  intervalId = setInterval(fetchStations, 5000); // Poll every 5 seconds
});

// Cleanup interval on component unmount
onUnmounted(() => {
  if (intervalId) {
    clearInterval(intervalId);
  }
});

const onRowEditSave = async (event) => {
  const { newData, index } = event;
  // Check if newData is correct and update
  if (newData) {
    stations.value = [
      ...stations.value.slice(0, index),
      { ...newData },
      ...stations.value.slice(index + 1),
    ];
    await StationService.updateStationInfo(newData);
  }
};
</script>

<style scoped>
:deep(.edit-input) {
  width: 60%;
  max-width: 100%;
  box-sizing: border-box;
  background-color: #1e2530;
  border-radius: 6px;
  border: 1px solid #2a323d;
  color: white;
  padding: 0.5rem;
}

:deep(.p-inputnumber-input) {
  width: 100% !important;
  max-width: 100% !important;
  background-color: #1e2530;
  border-radius: 6px;
  border: 1px solid #2a323d;
  color: white;
  padding: 0.5rem;
  outline: none;
}

:deep(.p-inputnumber) {
  border: none;
  background: none;
}
</style>
