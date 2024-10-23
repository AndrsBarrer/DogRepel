<template>
  <div class="registerContainer">
    <header>Register Station</header>

    <form>
      <div class="form first">
        <div class="details personal">
          <div class="fields">
            <div class="input-field">
              <label for="dogName">Dog's Name</label>
              <input
                id="dogName"
                type="text"
                placeholder="Enter Name"
                required
                v-model="dog.name"
              />
            </div>
          </div>

          <div class="fields">
            <div class="input-field">
              <label for="breed">Breed</label>
              <input
                id="breed"
                type="text"
                placeholder="Enter Breed"
                required
                v-model="dog.breed"
              />
            </div>
          </div>

          <div class="fields">
            <div class="input-field">
              <label for="age">Age</label>
              <input
                id="age"
                type="number"
                placeholder="Enter Age"
                required
                v-model="dog.age"
              />
            </div>
          </div>
        </div>
      </div>

      <button @click="registerDog" type="button">Register</button>
      <p v-if="message">{{ message }}</p>
    </form>
  </div>
</template>

<script setup lang="ts">
import DogService from "../services/DogService";
import { ref } from "vue";

const dog = ref({
  name: "",
  breed: "",
  age: null,
});

const message = ref("");

const registerDog = async () => {
  try {
    await DogService.registerDog({
      name: dog.value.name,
      breed: dog.value.breed,
      age: dog.value.age,
    });
    message.value = "Success!";
  } catch (error) {
    message.value = "Failed.";
  }
};
</script>

<style lang="scss" scoped>
$button-background-color: #007bff;
$button-background-hover-color: #005bbd;

.registerContainer {
  width: 40%;
  max-width: 40%;
  margin: 0 5rem;
  padding: 20px;
  border: 1px solid #424141;
  border-radius: 10px;
  background-color: #18181a;

  * {
    font-family: "IBM Plex Mono", monospace;
  }

  header {
    display: flex;
    flex-direction: row;
    font-size: 24px;
    margin-bottom: 20px;
    justify-content: center;
  }

  .form {
    display: flex;
    flex-direction: column;

    .details {
      margin-bottom: 20px;

      .fields {
        margin-bottom: 15px;

        .input-field {
          label {
            display: block;
            margin-bottom: 5px;
          }

          input {
            width: 100%;
            padding: 10px;
            border: 1px solid #ccc;
            border-radius: 8px;
          }
        }
      }
    }
  }

  button {
    padding: 10px 15px;
    border: none;
    border-radius: 4px;
    background-color: $button-background-color;
    color: #fff;
    font-size: 16px;
    cursor: pointer;
    transition: background-color 0.3s ease;

    &:hover {
      background-color: $button-background-hover-color;
    }
  }

  p {
    margin-top: 10px;
    color: #ff6f61; // Customize color for the message
  }
}
</style>
