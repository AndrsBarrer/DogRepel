import { createApp } from "vue";
import App from "./App.vue";
import PrimeVue from "primevue/config";
import router from "./router";
import axios from "axios";
import VueAxios from "vue-axios";

import Chart from "primevue/chart";
import DataTable from "primevue/datatable";
import Column from "primevue/column";
import InputText from "primevue/inputtext";
import InputNumber from "primevue/inputnumber";
import Tooltip from "primevue/tooltip";

// import Select from "../../node_modules/@primevue/themes/types/select";

// Theme imports - order is important
//import "primevue/resources/themes/lara-dark-blue/theme.css";
import "primevue/resources/themes/lara-light-blue/theme.css";
import "primeicons/primeicons.css";

const app = createApp(App);
axios.defaults.baseURL = "http://localhost:4000";

// Use PrimeVue first before registering components
app.use(PrimeVue, {
  ripple: true,
  unstyled: false,
  dark: false,
});

app.use(router); // Use the router instance with the app
app.use(VueAxios, axios);
app.component("Chart", Chart);
app.component("DataTable", DataTable);
app.component("Column", Column);
app.component("InputText", InputText);
app.component("InputNumber", InputNumber);
app.directive("tooltip", Tooltip);

app.mount("#app");
