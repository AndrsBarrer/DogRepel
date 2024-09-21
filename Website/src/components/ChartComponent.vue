<template>
  <div class="card">
    <Chart
      type="line"
      :data="chartData"
      :options="chartOptions"
      class="h-[30rem]"
    />
  </div>
</template>

<script setup>
import { ref, onMounted } from "vue";
import DogService from "../services/DogService";

const chartData = ref();
const chartOptions = ref();

const fetchDogVisits = async () => {
  try {
    // Fetch dog_visits data from the API
    const dogVisits = await DogService.getDogVisits();

    // Map visit data: separate x (time in hours) and y (distance)
    const visitTimes = dogVisits.map((visit) =>
      new Date(visit.visit_time).getHours()
    );
    const visitDistances = dogVisits.map((visit) => visit.distance);

    // Set the chart data based on fetched visits
    chartData.value = {
      labels: visitTimes, // Time in hours for x-axis
      datasets: [
        {
          label: "Dog Visits",
          data: visitDistances,
          fill: false,
          borderColor: getComputedStyle(
            document.documentElement
          ).getPropertyValue("--p-cyan-500"),
          tension: 0.4,
        },
      ],
    };
  } catch (error) {
    console.error("Error fetching dog visits:", error);
  }
};

onMounted(() => {
  // Initialize chart options
  chartOptions.value = setChartOptions();

  // Fetch and update chart data every minute
  fetchDogVisits(); // Initial fetch
  setInterval(fetchDogVisits, 60000); // Repeat every 60 seconds
});

const setChartOptions = () => {
  const documentStyle = getComputedStyle(document.documentElement);
  const textColor = documentStyle.getPropertyValue("--p-text-color");
  const textColorSecondary = documentStyle.getPropertyValue(
    "--p-text-muted-color"
  );
  const surfaceBorder = documentStyle.getPropertyValue(
    "--p-content-border-color"
  );

  return {
    maintainAspectRatio: false,
    aspectRatio: 0.6,
    plugins: {
      legend: {
        labels: {
          color: textColor,
        },
      },
    },
    scales: {
      x: {
        ticks: {
          color: textColorSecondary,
        },
        grid: {
          color: surfaceBorder,
        },
      },
      y: {
        ticks: {
          color: textColorSecondary,
        },
        grid: {
          color: surfaceBorder,
        },
      },
    },
  };
};
</script>
