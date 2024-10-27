import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import path from 'path'

export default defineConfig({
  plugins: [vue()],
  resolve: {
    alias: {
      // This helps Vite find modules in the root node_modules
      '@': path.resolve(__dirname, './src'),
      'node_modules': path.resolve(__dirname, '../node_modules')
    }
  }
})
