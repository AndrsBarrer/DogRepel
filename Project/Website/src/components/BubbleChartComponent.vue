<template>
  <div class="chart-container">
    <canvas ref="chartCanvas"></canvas>
  </div>
</template>

<script setup lang="ts">
import { onMounted, onBeforeUnmount, ref, shallowRef } from "vue";
import DogService from "../services/DogService";
import StationService from "../services/StationService";
import Chart from "chart.js/auto";

const chartCanvas = ref(null);
// Use shallowRef for the chart instance to prevent deep reactivity
const chartInstance = shallowRef(null);
const updateInterval = ref(null);

const generateColor = (station_id: number) => {
  const hue = (station_id * 137.508) % 360;
  const rgb = hueToRGB(hue);
  return `rgba(${rgb.r}, ${rgb.g}, ${rgb.b}, 0.4)`;
};

const hueToRGB = (hue: number) => {
  const h = hue / 360;
  const r = Math.round(255 * (1 - Math.abs(2 * h - 1)));
  const g = Math.round(255 * (1 - Math.abs(2 * (h - 1 / 3) - 1)));
  const b = Math.round(255 * (1 - Math.abs(2 * (h - 2 / 3) - 1)));
  return { r, g, b };
};

const fetchData = async () => {
  try {
    const dogVisits = await DogService.getDogVisits().then(
      (result) => result.data
    );

    const stationVisitMap = dogVisits.reduce((acc, visit) => {
      const { location, distance } = visit;

      if (!acc[location]) {
        acc[location] = { count: 0, totalDistance: 0, visits: 0 };
      }
      acc[location].count += 1;
      acc[location].totalDistance += distance;
      acc[location].visits += 1;
      return acc;
    }, {});

    const chartData = await Promise.all(
      Object.entries(stationVisitMap).map(async ([location, info]) => {
        const station = await StationService.getStation(location);
        return {
          x: info.count,
          y: Math.abs(info.totalDistance / info.visits),
          r: Math.abs(info.totalDistance / info.visits),
          station_name: station[0].location,
          backgroundColor: generateColor(parseInt(location)),
          borderColor: generateColor(parseInt(location)),
        };
      })
    );
    return chartData;
  } catch (error) {
    console.error("Error fetching data: ", error);
    return [];
  }
};

const initChart = (data) => {
  const ctx = chartCanvas.value?.getContext("2d");
  if (!ctx) return null;

  // Convert the reactive data to plain objects
  const datasets = data.map((item) => ({
    label: String(item.station_name),
    data: [
      {
        x: Number(item.x),
        y: Number(item.y),
        r: Number(item.r),
      },
    ],
    backgroundColor: String(item.backgroundColor),
    borderColor: "white",
    borderWidth: 2,
  }));

  const chartConfig = {
    type: "bubble",
    data: { datasets },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: {
          type: "linear",
          grid: { color: "rgba(255, 255, 255, 0.5)" },
          title: { display: true, text: "Number of Visits (Frequency)" },
          min: 0,
          max: 300,
        },
        y: {
          type: "linear",
          grid: { color: "rgba(255, 255, 255, 0.1)" },
          title: { display: true, text: "Average abs. RSSI (Distance)" },
          min: 0,
          max: 100,
        },
      },
      plugins: {
        legend: {
          display: true,
          labels: { color: "white" },
        },
      },
    },
  };

  return new Chart(ctx, chartConfig);
};

const updateChartData = (chart, newData) => {
  if (!chart) return;

  // Convert the reactive data to plain objects
  const datasets = newData.map((item) => ({
    label: String(item.station_name),
    data: [
      {
        x: Number(item.x),
        y: Number(item.y),
        r: Number(item.r),
      },
    ],
    backgroundColor: String(item.backgroundColor),
    borderColor: "white",
    borderWidth: 2,
  }));

  chart.data.datasets = datasets;
  chart.update("none");
};

const fetchBubbleInfo = async () => {
  try {
    const fetchedData = await fetchData();
    if (!fetchedData.length) {
      console.error("No data available to plot.");
      return;
    }

    if (!chartInstance.value) {
      const newChart = initChart(fetchedData);
      if (newChart) {
        chartInstance.value = newChart;
      }
    } else {
      updateChartData(chartInstance.value, fetchedData);
    }
  } catch (error) {
    console.error("Error updating chart:", error);
  }
};

onMounted(async () => {
  await fetchBubbleInfo();

  updateInterval.value = setInterval(async () => {
    await fetchBubbleInfo();
  }, 3000);
});

onBeforeUnmount(() => {
  if (updateInterval.value) {
    clearInterval(updateInterval.value);
    updateInterval.value = null;
  }

  if (chartInstance.value) {
    chartInstance.value.destroy();
    chartInstance.value = null;
  }
});
</script>

<style scoped>
.chart-container {
  position: relative;
  height: 400px;
  width: 100%;
}
</style>
