<!-- src/views/HomeView.vue -->
<template>
  <main>
    <MenuComponent @selectedMenuItem="changeMenu"></MenuComponent>

    <ChartComponent v-if="currentMenuItem === 'Dashboard'" />

    <h1 v-if="currentMenuItem === 'Profile'">profile will show here</h1>

    <h1 v-if="currentMenuItem === 'Settings'">Settings will show here</h1>

    <RegisterComponent
      v-if="currentMenuItem === 'Register'"
    ></RegisterComponent>
  </main>
</template>

<script setup lang="ts">
import { ref } from "vue";

import ChartComponent from "../components/ChartComponent.vue";
import MenuComponent from "../components/MenuComponent.vue";
import RegisterComponent from "../components/RegisterComponent.vue";
const currentMenuItem = ref("Dashboard");

const changeMenu = (payload) => {
  currentMenuItem.value = payload;
};
</script>

<style scoped>
main {
  padding: 20px;
}
</style>
