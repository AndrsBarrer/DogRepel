<template>
  <div class="card">
    <DataTable
      v-model:editingRows="editingRows"
      :value="stations"
      editMode="row"
      dataKey="station_id"
      @row-edit-save="onRowEditSave"
      :pt="{
        table: { style: 'max-width: 100%' },
        column: {
          bodycell: ({ state }) => ({
            style: state['d_editing'] && 'max-width: 100%',
          }),
        },
      }"
    >
      <Column field="location" header="Location" style="width: 10%">
        <template #editor="{ data, field }">
          <InputText v-model="data[field]" fluid />
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

<script setup>
import { ref, onMounted } from "vue";
import StationService from "../services/StationService";

const stations = ref([]);
const editingRows = ref([]);

onMounted(async () => {
  try {
    const response = await StationService.getStations();
    stations.value = response.results;
  } catch (error) {
    console.error("Error fetching stations: ", error);
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
