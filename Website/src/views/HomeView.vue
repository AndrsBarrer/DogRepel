<!-- src/views/HomeView.vue -->
<template>
  <main>
    <MenuComponent></MenuComponent>
    
    <ChartComponent v-if="currentMenuItem==='Dashboard'"/>

    <h1 v-if="currentMenuItem==='Profile'"> profile will show here</h1>
    
    <h1 v-if="currentMenuItem==='Settings'"> Settings will show here</h1>
  </main>
</template>

<script setup lang="ts">
import { ref } from 'vue';

import ChartComponent from '../components/ChartComponent.vue';
import MenuComponent from '../components/MenuComponent.vue';

const currentMenuItem = ref('Dashboard');

</script>

<style scoped>
/* Your styles here */
</style>
