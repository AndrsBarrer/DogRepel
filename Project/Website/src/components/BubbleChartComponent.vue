<template>
  <div class="chart-container">
    <canvas ref="chartCanvas"></canvas>
  </div>
</template>

<script setup lang="ts">
import { onMounted, onBeforeUnmount, ref, shallowRef, reactive } from "vue";
import DogService, { Visit } from "../services/DogService";
import StationService from "../services/StationService";
import Chart from "chart.js/auto";

const chartCanvas = ref(null);
// Use shallowRef for the chart instance to prevent deep reactivity
const chartInstance = shallowRef(null);
const updateInterval = ref(null);
const legendVisibility = reactive({});

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
    const dogVisits = await DogService.getDogVisits("dynamic").then(
      (result) => result.data
    );
    console.log(dogVisits);
    const stationVisitMap = dogVisits.reduce((acc, visit: Visit) => {
      const { station_id, distance } = visit;

      if (!acc[station_id]) {
        acc[station_id] = { count: 0, totalDistance: 0, visits: 0 };
      }
      acc[station_id].count += 1;
      acc[station_id].totalDistance += distance;
      acc[station_id].visits += 1;
      return acc;
    }, {});

    return await Promise.all(
      Object.entries(stationVisitMap).map(async ([station_id, info]) => {
        const station_id_num = Number(station_id);
        let station = await StationService.getStation(station_id_num);

        return {
          station_id: station_id_num,
          station_name: station[0].location,
          x: info.count,
          y: Math.abs(info.totalDistance / info.visits),
          r: Math.abs(info.totalDistance / info.visits),
          backgroundColor: generateColor(station_id_num),
          borderColor: generateColor(station_id_num),
        };
      })
    );
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
        station_id: item.station_id,
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
          max: 1000,
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
          onClick: function (event, legendItem) {
            const datasetIndex = legendItem.datasetIndex;
            const dataset = this.chart.data.datasets[datasetIndex];
            const station_id = dataset.data[0].station_id;

            const isVisible = !this.chart.isDatasetVisible(datasetIndex);
            // Update visibility state
            legendVisibility[station_id] = isVisible;

            // Toggle visibility in the chart
            this.chart.setDatasetVisibility(datasetIndex, isVisible);
            this.chart.update();
          },
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
  const datasets = newData.map((item) => {
    const isVisible = legendVisibility[item.station_id] !== false; // Respect visibility state
    return {
      label: item.station_name,
      data: [
        {
          station_id: item.station_id,
          x: item.x,
          y: item.y,
          r: item.r,
        },
      ],
      backgroundColor: item.backgroundColor,
      borderColor: "white",
      borderWidth: 2,
      hidden: !isVisible, // Apply visibility state
    };
  });

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
