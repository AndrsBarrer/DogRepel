<template>
  <div>
    <canvas id="bubble-chart"></canvas>
  </div>
</template>

<script setup>
import { onMounted, ref } from "vue";
import DogService from "../services/DogService";
import Chart from "chart.js/auto";

const chartInstance = ref(null);

// Function to generate a unique HSL color based on the station_id
const generateColor = (station_id) => {
  const hue = (station_id * 137.508) % 360; // Golden angle approximation for unique hues
  const rgb = hueToRGB(hue);
  return `rgba(${rgb.r}, ${rgb.g}, ${rgb.b}, 0.4)`; // Set opacity to 0.6
};

// Helper function to convert hue to RGB
const hueToRGB = (hue) => {
  const h = hue / 360; // Normalize hue
  const r = Math.round(255 * (1 - Math.abs(2 * h - 1)));
  const g = Math.round(255 * (1 - Math.abs(2 * (h - 1 / 3) - 1)));
  const b = Math.round(255 * (1 - Math.abs(2 * (h - 2 / 3) - 1)));
  return { r, g, b };
};

const fetchData = async () => {
  try {
    // Replace this with your actual service call
    const dogVisits = await DogService.getDogVisits();

    // Group the data by station_id and count the frequency of visits
    const stationVisitMap = dogVisits.reduce((acc, visit) => {
      const { station_id, distance } = visit;

      if (!acc[station_id]) {
        acc[station_id] = { count: 0, totalDistance: 0, visits: 0 };
      }

      acc[station_id].count += 1; // Count the number of visits
      acc[station_id].totalDistance += distance; // Sum the distances
      acc[station_id].visits += 1; // Increment the number of visits

      return acc;
    }, {});

    // Format the data for the bubble chart and generate color based on station_id
    const chartData = Object.entries(stationVisitMap).map(
      ([station_id, info]) => ({
        x: info.count, // Number of visits (frequency)
        y: Math.abs(info.totalDistance / info.visits),
        r: Math.abs(info.totalDistance / info.visits), // Average distance as bubble radius
        station_id: station_id,
        backgroundColor: generateColor(station_id),
        borderColor: generateColor(station_id),
      })
    );

    console.log(chartData);
    return chartData;
  } catch (error) {
    console.error("Error fetching data: ", error);
    return [];
  }
};

onMounted(async () => {
  // Clear previous chart instance if it exists
  if (chartInstance.value) {
    chartInstance.value.destroy();
  }

  const ctx = document.getElementById("bubble-chart").getContext("2d");

  const fetchedData = await fetchData();
  if (fetchedData.length === 0) {
    console.error("No data available to plot.");
    return;
  }

  // Separate datasets for each station, dynamically color-coded
  const data = {
    datasets: fetchedData.map((item) => ({
      label: `Station ${item.station_id}`, // Show the station ID in the legend
      data: [item], // Each station's data as a separate dataset
      backgroundColor: item.backgroundColor,
      borderColor: item.borderColor,
    })),
  };

  const options = {
    scales: {
      x: {
        type: "linear",
        grid: {
          color: "rgba(255, 255, 255, 0.1)", // White grid lines on the X-axis
        },
        title: {
          display: true,
          text: "Number of Visits (Frequency)",
        },
        min: 0,
        max: 300,
      },
      y: {
        type: "linear",
        grid: {
          color: "rgba(255, 255, 255, 0.1)", // White grid lines on the Y-axis
        },
        title: {
          display: true,
          text: "Average abs. RSSI (Distance)",
        },
        min: 0,
        max: 100,
      },
    },
    plugins: {
      legend: {
        display: true, // Show the legend with station colors
      },
    },
  };

  // Destroy previous chart instance if it exists
  if (chartInstance.value) {
    chartInstance.value.destroy();
  }

  chartInstance.value = new Chart(ctx, {
    type: "bubble",
    data: data,
    options: options,
  });
});
</script>

<style scoped>
canvas {
  max-width: 100%;
  max-height: 400px;
}
</style>
