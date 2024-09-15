import { createApp } from "vue";
import App from "./App.vue";
import PrimeVue from "primevue/config";
import router from "./router"; // Import the router instance
import axios from "axios";
import VueAxios from "vue-axios";

import Chart from "primevue/chart"; // Import Chart from PrimeVue

import Aura from "@primevue/themes/aura";
import "primeicons/primeicons.css"; // PrimeIcons CSS

const app = createApp(App);
axios.defaults.baseURL = "http://localhost:4000";

app.use(router); // Use the router instance with the app
app.use(PrimeVue);
app.use(VueAxios, axios);
app.component("Chart", Chart);

app.use(PrimeVue, {
  // Default theme configuration
  theme: {
    preset: Aura,
    options: {
      prefix: "p",
      darkModeSelector: "system",
      cssLayer: false,
    },
  },
});

app.mount("#app");
