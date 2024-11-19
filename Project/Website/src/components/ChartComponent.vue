<template>
  <div class="card">
    <Chart
      type="line"
      :data="chartData"
      :options="chartOptions"
      class="h-[30rem]"
      :style="{
        height: '20rem',
        margin: '1rem 4rem 1rem',
      }"
    />
  </div>
</template>

<script setup lang="ts">
import { ref, onMounted, reactive } from "vue";
import DogService from "../services/DogService";
import { Dog, Visit } from "../services/DogService";
import StationService from "../services/StationService";
import { Station } from "../services/StationService";

let legendVisibility = reactive({}); // To store visibility state

const chartData = ref();
const chartOptions = ref();
let reactiveChartOptions = reactive({
  options: {
    maintainAspectRatio: false,
    aspectRatio: 0.6,
    plugins: {
      legend: {
        onClick: function (event, legendItem) {
          // Toggle visibility and save state
          const datasetIndex = legendItem.datasetIndex;

          const isVisible = !this.chart.isDatasetVisible(datasetIndex);
          legendVisibility[datasetIndex] = isVisible;

          // Default legend behavior
          this.chart.setDatasetVisibility(datasetIndex, isVisible);
          this.chart.update();
        },
        labels: {
          color: "white", // Set legend text color to white
        },
      },
    },
    scales: {
      x: {
        title: {
          display: true,
          text: "Hour of Day",
          color: "white", // Set x-axis title color to white
        },
        ticks: {
          color: "white", // Set x-axis tick color to white
        },
        grid: {
          color: "rgba(255, 255, 255, 0.2)", // Set x-axis grid line color to white with slight transparency
        },
      },
      y: {
        min: -90,
        max: -10,
        title: {
          display: true,
          text: "Proximity",
          color: "white", // Set y-axis title color to white
        },
        ticks: {
          color: "white", // Set y-axis tick color to white
          callback: function (value: number) {
            if (value === -80) return "Low";
            if (value === -40) return "Medium";
            if (value === 0) return "High";
            return null;
          },
          stepSize: 40,
        },
        grid: {
          color: "rgba(255, 255, 255, 0.2)", // Set y-axis grid line color to white with slight transparency
        },
      },
    },
  },
});

// Used to know if the chart should be updated or not, stores the gotten value to compare to new value
const oldDogVisits = ref({});

async function fetchDogVisits() {
  try {
    const dogVisits = await DogService.getDogVisits("dynamic").then(
      (result) => result.data
    );

    if (JSON.stringify(dogVisits) !== JSON.stringify(oldDogVisits.value)) {
      oldDogVisits.value = dogVisits;

      const groupedVisits = {};
      dogVisits.forEach((visit) => {
        const date = new Date(visit.visit_time);
        const minuteKey = `${date.getHours()}:${date
          .getMinutes()
          .toString()
          .padStart(2, "0")}`;
        const key = `${visit.dog_name}-${visit.location}`;

        if (!groupedVisits[key]) groupedVisits[key] = {};
        if (!groupedVisits[key][minuteKey]) groupedVisits[key][minuteKey] = [];

        groupedVisits[key][minuteKey].push(visit.distance);
      });

      const datasets = Object.keys(groupedVisits).map((key, index) => {
        const [dogName, location] = key.split("-");
        const dataPoints = Object.keys(groupedVisits[key])
          .sort()
          .map((minute) => {
            const distances = groupedVisits[key][minute];
            const avgDistance =
              distances.reduce((sum, dist) => sum + dist, 0) / distances.length;
            return { x: minute, y: avgDistance };
          });

        // Reapply saved visibility state or default to visible
        const isVisible = legendVisibility[index] !== false;

        return {
          label: `${dogName} at ${location}`,
          data: dataPoints,
          borderColor: getColorForDogStation(dogName, location),
          tension: 0.4,
          hidden: !isVisible, // Apply saved visibility state
        };
      });

      const allMinutes = [
        ...new Set(
          Object.values(groupedVisits).flatMap((minutes) =>
            Object.keys(minutes)
          )
        ),
      ].sort();

      chartData.value = {
        labels: allMinutes,
        datasets: datasets,
      };
    }
  } catch (error) {
    console.error("Error fetching dog visits:", error);
  }
}

// Function to generate colors based on Dog Name and Station Name
function getColorForDogStation(dogName: string, stationName: string) {
  // Concatenate the dog's name and station's name
  const combinedName = `${dogName}-${stationName}`;

  // Hash the combined string to get a base value
  const hash = combinedName
    .split("")
    .reduce((acc, char) => acc + char.charCodeAt(0), 0);

  // Use hash to set hue, keeping saturation and lightness constant
  const hue = hash % 360; // Hue can range from 0 to 360 degrees
  const saturation = 50; // Set saturation to 70%
  const lightness = 40; // Set lightness to 50%

  return `hsl(${hue}, ${saturation}%, ${lightness}%)`;
}

onMounted(() => {
  // Initialize chart options
  chartOptions.value = reactiveChartOptions.options;

  // Fetch and update chart data every minute
  fetchDogVisits(); // Initial fetch
  setInterval(fetchDogVisits, 2000); // Repeat every 2 seconds

  // Turn off the page animation if the page already loaded once, its annoying
  setTimeout(() => {
    // any is used to bypass strict type checks, animation doesnt exist on the object yet so it yells at us
    (reactiveChartOptions.options as any).animation = {
      duration: 0,
    };
  }, 1500); // Wait for initial animation to finish
});

console.clear();
console.log(
  "Welcome to Dog Repel! \n" +
    "   / \\__ \n" +
    "  (    @\\___ \n" +
    "  /         O \n" +
    " /   (_____/ \n" +
    "/_____/   U"
);
</script>
