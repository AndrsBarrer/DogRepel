import { createApp } from "vue";
import App from "./App.vue";
import PrimeVue from "primevue/config";
import router from "./router"; // Import the router instance
import axios from "axios";
import VueAxios from "vue-axios";

import Chart from "primevue/chart"; // Import Chart from PrimeVue
import DataTable from "primevue/datatable";
import Column from "primevue/column";
import InputText from "primevue/inputtext";
import InputNumber from "primevue/inputnumber";

import Aura from "@primevue/themes/aura";
import Lara from "@primevue/themes/lara";
import Nora from "@primevue/themes/nora";
import "primeicons/primeicons.css"; // PrimeIcons CSS

const app = createApp(App);
axios.defaults.baseURL = "http://localhost:4000";

app.use(router); // Use the router instance with the app
app.use(VueAxios, axios);
app.component("Chart", Chart);
app.component("DataTable", DataTable);
app.component("Column", Column);
app.component("InputText", InputText);
app.component("InputNumber", InputNumber);

app.use(PrimeVue, {
  // Default theme configuration
  theme: {
    preset: Aura,
    // options: {
    //   prefix: "p",
    //   darkModeSelector: "true",
    //   cssLayer: false,
    // },
  },
});

app.mount("#app");
