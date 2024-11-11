<template>
  <div class="card">
    <DataTable
      v-model:editingRows="editingRows"
      :value="dogs"
      editMode="row"
      dataKey="dog_id"
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
      <Column field="name" header="Name" style="width: 80px">
        <template #editor="{ data, field }">
          <InputText v-model="data[field]" class="edit-input" />
        </template>
      </Column>

      <Column field="breed" header="Breed" style="width: 100px">
        <template #editor="{ data, field }">
          <InputText v-model="data[field]" class="edit-input" />
        </template>
      </Column>

      <Column field="age" header="Age" style="width: 40px">
        <template #editor="{ data, field }">
          <InputNumber v-model="data[field]" class="edit-input" />
        </template>
      </Column>

      <Column
        :rowEditor="true"
        style="width: 80px"
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

// Used to know whether interface should be updated or not,
// to not interrupt user when they are editing the table
const dogsReference = ref({});

// Function to fetch and update dogs data
const fetchDogs = async () => {
  try {
    const data = await DogService.getDogs();
    if (JSON.stringify(dogsReference.value) !== JSON.stringify(data.results)) {
      dogs.value = data.results;
      dogsReference.value = dogs.value;
    }
  } catch (error) {
    console.error("Error fetching dogs:", error);
  }
};

// Fetch data when component is mounted
onMounted(() => {
  fetchDogs(); // Initial fetch
  intervalId = setInterval(fetchDogs, 2000); // Poll every 5 seconds
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

<style scoped>
:deep(.edit-input) {
  width: 100%;
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
