import { createApp } from 'vue';
import App from './App.vue';
import PrimeVue from 'primevue/config';
import router from './router'; // Import the router instance


import Chart from 'primevue/chart'; // Import Chart from PrimeVue

import Aura from '@primevue/themes/aura';
import 'primeicons/primeicons.css';                     // PrimeIcons CSS

const app = createApp(App);

app.use(router); // Use the router instance with the app
app.use(PrimeVue);
app.component('Chart', Chart);

app.use(PrimeVue, {
    // Default theme configuration
    theme: {
        preset: Aura,
        options: {
            prefix: 'p',
            darkModeSelector: 'system',
            cssLayer: false
        }
    }
 });



app.mount('#app');
