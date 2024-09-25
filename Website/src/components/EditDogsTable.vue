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
          <InputText v-model="data[field]" />
        </template>
      </Column>

      <Column field="breed" header="Breed" style="width: 10%">
        <template #editor="{ data, field }">
          <InputText v-model="data[field]" />
        </template>
      </Column>

      <Column field="age" header="Age" style="width: 10%">
        <template #editor="{ data, field }">
          <InputNumber v-model="data[field]" />
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
import DogService from "../services/DogService";

const dogs = ref([]);
const editingRows = ref([]);

onMounted(async () => {
  try {
    const response = await DogService.getDogs();
    dogs.value = response.results;
    console.log(response);
  } catch (error) {
    console.error("Error fetching dogs: ", error);
  }
});

const onRowEditSave = (event) => {
  const { newData, index } = event;

  // Check if newData is correct and update
  if (newData) {
    dogs.value = [
      ...dogs.value.slice(0, index),
      { ...newData },
      ...dogs.value.slice(index + 1),
    ];
  }
};
</script>
