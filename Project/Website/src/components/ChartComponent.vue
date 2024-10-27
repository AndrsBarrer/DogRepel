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

<script setup>
import { ref, onMounted } from "vue";
import DogService from "../services/DogService";
import StationService from "../services/StationService";

const chartData = ref();
const chartOptions = ref();

async function fetchDogVisits() {
  try {
    // Fetch dog_visits data from the API
    const dogVisits = await DogService.getDogVisits();

    // Fetch dog details (names) from the API
    const dogs = await DogService.getDogs();

    // Fetch station details (names) from the API
    const stations = await StationService.getStations();

    // Map dog names to dog_ids
    const dogNamesMap = dogs.results.reduce((map, dog) => {
      map[dog.dog_id] = dog.name;
      return map;
    }, {});

    // Map station names to station_ids
    const stationNamesMap = stations.results.reduce((map, station) => {
      map[station.station_id] = station.location;
      return map;
    }, {});

    // Group visits by dog_id and station_id
    const groupedVisits = {};
    dogVisits.forEach((visit) => {
      const key = `${visit.dog_id}-${visit.station_id}`;
      if (!groupedVisits[key]) {
        groupedVisits[key] = {
          dog_id: visit.dog_id,
          station_id: visit.station_id,
          visits: [],
        };
      }
      groupedVisits[key].visits.push(visit);
    });

    // Prepare datasets for each dog-station combination
    const datasets = Object.keys(groupedVisits).map((key) => {
      const group = groupedVisits[key];

      // Map visit times to "HH:MM" format
      const visitTimes = group.visits.map((visit) => {
        const date = new Date(visit.visit_time);
        const hours = String(date.getHours()).padStart(2, "0"); // Ensure two-digit format
        const minutes = String(date.getMinutes()).padStart(2, "0"); // Ensure two-digit format
        return `${hours}:${minutes}`; // Combine hours and minutes
      });

      const visitDistances = group.visits.map((visit) => visit.distance);

      // Fetch the dog's name and station's name
      const dogName = dogNamesMap[group.dog_id] || `Dog ${group.dog_id}`;
      const stationName =
        stationNamesMap[group.station_id] || `Station ${group.station_id}`;

      return {
        label: `${dogName} at ${stationName}`, // Use dog's name and station's name
        data: visitDistances,
        fill: false,
        borderColor: getColorForDogStation(dogName, stationName), // Color based on names
        tension: 0.4,
      };
    });

    // Set the chart data based on grouped visits
    chartData.value = {
      labels: [
        ...new Set(
          dogVisits.map((visit) => {
            const date = new Date(visit.visit_time);
            const hours = String(date.getHours()).padStart(2, "0");
            const minutes = String(date.getMinutes()).padStart(2, "0");
            return `${hours}:${minutes}`; // Unique time points in "HH:MM"
          })
        ),
      ], // Unique time points for x-axis
      datasets: datasets,
    };
  } catch (error) {
    console.error("Error fetching dog visits:", error);
  }
}

// Function to generate colors based on Dog Name and Station Name
function getColorForDogStation(dogName, stationName) {
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
  chartOptions.value = setChartOptions();

  // Fetch and update chart data every minute
  fetchDogVisits(); // Initial fetch
  setInterval(fetchDogVisits, 60000); // Repeat every 60 seconds
});

// const setChartOptions = () => {
//   const documentStyle = getComputedStyle(document.documentElement);
//   const textColor = documentStyle.getPropertyValue("--p-text-color");
//   const textColorSecondary = documentStyle.getPropertyValue(
//     "--p-text-muted-color"
//   );
//   const surfaceBorder = documentStyle.getPropertyValue(
//     "--p-content-border-color"
//   );

//   return {
//     maintainAspectRatio: false,
//     aspectRatio: 0.6,
//     plugins: {
//       legend: {
//         labels: {
//           color: textColor,
//         },
//       },
//     },
//     scales: {
//       x: {
//         title: {
//           display: true,
//           text: "Hour of Day", // Label for the x-axis
//           color: textColor,
//         },
//         ticks: {
//           color: textColorSecondary,
//         },
//         grid: {
//           color: surfaceBorder,
//         },
//       },
//       y: {
//         min: -60, // Set the fixed minimum value for the y-axis
//         max: -20, // Set the fixed maximum value for the y-axis
//         title: {
//           display: true,
//           text: "Proximity", // Label for the y-axis
//           color: textColor,
//         },
//         ticks: {
//           color: textColorSecondary,
//           // Define the exact positions for the labels
//           callback: function (value) {
//             if (value === -80) return "Low"; // Lowest value
//             if (value === -40) return "Medium"; // Mid value
//             if (value === 0) return "High"; // Highest value
//             return null; // Hide other values
//           },
//           // Force display of ticks at specific values
//           stepSize: 40, // Ensure consistent spacing
//         },
//         grid: {
//           color: surfaceBorder,
//         },
//       },
//     },
//   };
// };
const setChartOptions = () => {
  return {
    maintainAspectRatio: false,
    aspectRatio: 0.6,
    plugins: {
      legend: {
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
        min: -60,
        max: -20,
        title: {
          display: true,
          text: "Proximity",
          color: "white", // Set y-axis title color to white
        },
        ticks: {
          color: "white", // Set y-axis tick color to white
          callback: function (value) {
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
  };
};
</script>
