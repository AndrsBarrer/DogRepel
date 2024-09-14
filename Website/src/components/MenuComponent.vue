<template>
    <div class="menu-container">
      <div class="menu-list" v-for="menuItem in menuItems" :key="menuItem">
        <input type="radio" name="menu" />
        <div class=menu-item>
            <label @click="selectMenuItem(menuItem)"> {{ menuItem }}</label>
            <div v-if="selectedMenuItem===menuItem" class="blue-bar"></div>
        </div>
    </div>
    </div>
  </template>

<script setup lang="ts">

import { ref } from 'vue';

const menuItems = ref(['Dashboard', 'Profile', 'Settings']); // Selection of all available menu tabs
const selectedMenuItem = ref( menuItems.value[0]); // This will be the default item when the app starts


// Defining what type of event we want to emit to the parent
const emit = defineEmits(['sendMenuItem']);

// Updates selected variable with menuItem so that blue bar knows where it is supposed to be 
// Function that emits with the defined type, a variable
function selectMenuItem(menuItem: string) {
    selectedMenuItem.value = menuItem;
    emit('sendMenuItem', menuItem);
}
</script>



<style lang="scss" scoped>

.menu-container { 
    display: flex;
    position: relative;
    margin-bottom: 2rem;
    
    label {
        font-size: 1rem;
        display: block;
        padding-top: 1rem;
        padding-left: 1rem;
        padding-right: 1rem;
        padding-bottom: 0.2rem; // Padding is this way to make blue bar go under text
        text-align: center;
        cursor:pointer;
    }

    input[type="radio"] {
        display: none;
    }

    .menu-list {
        position: relative;
        transition: all 0.3s ease-in-out;
        &:hover {
            color: rgb(60, 60, 217);
        }

        .blue-bar {
            display: block;
            width: 80%;
            margin-left: auto;
            margin-right: auto;
            background-color: blue;
            height: 4px;
            bottom: 0;
            left: 0;
            border-radius: 10px;
        }
    }
}


</style>