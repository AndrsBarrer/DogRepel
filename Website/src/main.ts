import { createApp } from 'vue';
import App from './App.vue';
import PrimeVue from 'primevue/config';
import router from './router'; // Import the router instance
import Chart from 'primevue/chart'; // Import Chart from PrimeVue

const app = createApp(App);

app.use(router); // Use the router instance with the app
app.use(PrimeVue);
app.component('Chart', Chart); // Register Chart component globally

app.mount('#app');
