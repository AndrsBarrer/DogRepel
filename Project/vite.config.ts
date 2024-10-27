import { fileURLToPath, URL } from "node:url";

import { defineConfig } from "vite";
import vue from "@vitejs/plugin-vue";
import vueJsx from "@vitejs/plugin-vue-jsx";

// // https://vitejs.dev/config/
// export default defineConfig({
//   plugins: [vue(), vueJsx()],
//   resolve: {
//     alias: {
//       "@": fileURLToPath(new URL("./src", import.meta.url)),
//     },
//   },
// });

import path from "path";
export default defineConfig({
  root: path.resolve(__dirname, "Website"), // Set Website as the root
  plugins: [vue()],
  resolve: {
    alias: {
      "@": path.resolve(__dirname, "Website/src"), // Alias for Website's src directory
    },
    dedupe: ["vue"], // Ensures Vue is loaded from the root node_modules
  },
});
