<template>
  <div class="card">
    <DataTable
      v-model:editingRows="editingRows"
      :value="dogs"
      editMode="row"
      dataKey="dog_id"
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
      <Column field="name" header="Name" style="width: 10%">
        <template #editor="{ data, field }">
          <InputText v-model="data[field]" fluid />
        </template>
      </Column>

      <Column field="breed" header="Breed" style="width: 10%">
        <template #editor="{ data, field }">
          <InputText v-model="data[field]" fluid />
        </template>
      </Column>

      <Column field="age" header="Age" style="width: 10%">
        <template #editor="{ data, field }">
          <InputNumber v-model="data[field]" fluid />
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
import DogService from "../services/DogService";
let intervalId: NodeJS.Timeout;

const dogs = ref([]);
const editingRows = ref([]);

// Function to fetch and update dogs data
const fetchDogs = async () => {
  try {
    const data = await DogService.getDogs();
    dogs.value = data.results;
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
  if (intervalId) {
    clearInterval(intervalId);
  }
});

const onRowEditSave = async (event) => {
  const { newData, index } = event;
  // Check if newData is correct and update
  if (newData) {
    dogs.value = [
      ...dogs.value.slice(0, index),
      { ...newData },
      ...dogs.value.slice(index + 1),
    ];
    await DogService.updateDogInfo(newData);
  }
};
</script>
