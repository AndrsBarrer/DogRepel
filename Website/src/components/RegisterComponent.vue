<template>
  <div class="registerContainer">
    <header>Register Dog</header>

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
import { ref } from "vue";
import axios from "axios";

const dog = ref({
  name: "",
  breed: "",
  age: null,
});

const message = ref("");

const registerDog = async () => {
  try {
    const response = await axios.post("/api/dogs", {
      name: dog.value.name,
      breed: dog.value.breed,
      age: dog.value.age,
    });
    message.value = response.data.message;
  } catch (error) {
    if (error.response) {
      message.value = error.response.data.error;
    } else {
      message.value = "An error occured.";
    }
  }
};
</script>

<style lang="scss" scoped>
$button-background-color: #007bff;
$button-background-hover-color: #005bbd;

.registerContainer {
  max-width: 600px;
  margin: 0 auto;
  padding: 20px;
  border: 1px solid #ccc;
  border-radius: 15px;
  background-color: #f9f9f9;
}

header {
  font-size: 24px;
  font-weight: bold;
  margin-bottom: 20px;
}

.form {
  display: flex;
  flex-direction: column; // make the elements extend the whole column
}

.details {
  margin-bottom: 20px;
}

.title {
  font-size: 18px;
  font-weight: bold;
  margin-bottom: 10px;
  display: block;
}

.fields {
  margin-bottom: 15px;
}

.input-field {
  margin-bottom: 10px;
}

.input-field label {
  display: block;
  margin-bottom: 5px;
  font-weight: 600;
}

.input-field input {
  width: 100%;
  padding: 10px;
  border: 1px solid #ccc;
  border-radius: 8px;
}

button {
  padding: 10px 15px;
  border: none;
  border-radius: 4px;
  background-color: $button-background-color;
  color: #fff;
  font-size: 16px;
  cursor: pointer;

  transition: background-color 0.3s ease, color 0.3s ease;
  &:hover {
    background-color: $button-background-hover-color;
  }
}
</style>
