<template>
  <div class="card">
    <DataTable :value="data" tableStyle="min-width: 50rem">
      <Column
        v-for="col of columns"
        :key="col.field"
        :field="col.field"
        :header="col.header"
      ></Column>
    </DataTable>
  </div>
</template>

<script setup lang="ts">
import { ref, watch } from "vue";

interface ColumnDef {
  field: string;
  header: string;
}

const props = defineProps<{
  columns: ColumnDef[];
  data: Array<any>;
}>();

// Create a local reactive state for data
const data = ref(props.data);

// Watch for changes in the data prop to update the local state
watch(
  () => props.data,
  (newData) => {
    data.value = newData;
  },
  { immediate: true }
);
</script>

<style scoped lang="scss">
// This needs to stretch the entirety of the container,
// so that columns can be made of the DataTableComponent
.card {
  display: flex;
  justify-content: center; /* Centers the DataTable horizontally */
  align-items: center; /* Centers the DataTable vertically */
  margin-top: 40px;
  width: 100%;
  text-align: center;
}
</style>
