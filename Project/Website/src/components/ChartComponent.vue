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
import { ref, onMounted } from "vue";
import DogService from "../services/DogService";
import { Dog, Visit } from "../services/DogService";
import StationService from "../services/StationService";
import { Station } from "../services/StationService";

const chartData = ref();
const chartOptions = ref();

// Used to know if the chart should be updated or not, stores the gotten value to compare to new value
const oldDogVisits = ref({});

// Define type for grouped visits structure
type GroupedVisits = {
  [key: string]: {
    // Key is the "dog_id-station_id" pair
    [minuteKey: string]: number[]; // Each minuteKey maps to an array of distances
  };
};

async function fetchDogVisits() {
  try {
    // Fetch dog visits data from the API
    const dogVisits = await DogService.getDogVisits().then(
      (result) => result.data
    );

    if (JSON.stringify(dogVisits) !== JSON.stringify(oldDogVisits.value)) {
      oldDogVisits.value = dogVisits;

      // Fetch dog and station details
      const dogs = await DogService.getDogs();
      const stations = await StationService.getStations();

      // Map dog names and station names
      const dogNamesMap = dogs.results.reduce((acc: string[], dog: Dog) => {
        if (dog.dog_id) acc[dog.dog_id] = dog.name;
        return acc;
      }, {});

      const stationNamesMap = stations.results.reduce(
        (acc: string[], station: Station) => {
          acc[station.station_id] = station.location;
          return acc;
        },
        {}
      );

      // Group visits by dog_id, station_id, and minute, storing the distances for each group
      const groupedVisits: GroupedVisits = {}; // Initialize with the specified type
      dogVisits.forEach((visit: Visit) => {
        const date = new Date(visit.visit_time);
        const minuteKey = `${date.getHours()}:${date.getMinutes()}`;
        const key = `${visit.dog_id}-${visit.station_id}`;

        if (!groupedVisits[key]) groupedVisits[key] = {};
        if (!groupedVisits[key][minuteKey]) groupedVisits[key][minuteKey] = [];

        groupedVisits[key][minuteKey].push(visit.distance);
      });

      // Prepare datasets with averaged distances per minute for each dog-station combination
      const datasets = Object.keys(groupedVisits).map((key) => {
        const [dog_id, station_id] = key.split("-");
        const dogName = dogNamesMap[dog_id] || `Dog ${dog_id}`;
        const stationName =
          stationNamesMap[station_id] || `Station ${station_id}`;

        // Calculate average distance per minute and prepare data points for each minute
        const dataPoints = Object.keys(groupedVisits[key])
          .sort() // Ensure data points are in chronological order
          .map((minute) => {
            const distances = groupedVisits[key][minute];
            const avgDistance =
              distances.reduce((sum, dist) => sum + dist, 0) / distances.length;
            return { x: minute, y: avgDistance }; // Format as x-y coordinates
          });

        return {
          label: `${dogName} at ${stationName}`, // Single entry in legend for each dog-station combination
          data: dataPoints,
          fill: false,
          borderColor: getColorForDogStation(dogName, stationName), // Color based on names
          tension: 0.4,
        };
      });

      // Prepare chart data with sorted minute labels on the x-axis
      const allMinutes = [
        ...new Set(
          Object.values(groupedVisits).flatMap((minutes) =>
            Object.keys(minutes)
          )
        ),
      ].sort();

      chartData.value = {
        labels: allMinutes, // Unique sorted minutes for the x-axis
        datasets: datasets,
      };
    }
  } catch (error) {
    console.error("Error fetching dog visits:", error);
  }
}

// async function fetchDogVisits() {
//   try {
//     // Fetch dog_visits data from the API
//     const dogVisits = await DogService.getDogVisits().then(
//       (result) => result.data
//     );
//     console.log(dogVisits);
//     if (JSON.stringify(dogVisits) !== JSON.stringify(oldDogVisits.value)) {
//       oldDogVisits.value = dogVisits;

//       // Fetch dog details (names) from the API
//       const dogs = await DogService.getDogs();

//       // Fetch station details (names) from the API
//       const stations = await StationService.getStations();

//       // Map dog names to dog_ids
//       const dogNamesMap = dogs.results.reduce(
//         (dogEntries: DogEntries, dog: Dog) => {
//           if (dog.dog_id) {
//             dogEntries[dog.dog_id] = dog.name;
//             return dogEntries;
//           }
//         },
//         {}
//       );

//       // Map station names to station_ids
//       const stationNamesMap = stations.results.reduce(
//         (dogEntries: DogEntries, station: Station) => {
//           dogEntries[station.station_id] = station.location;
//           return dogEntries;
//         },
//         {}
//       );

//       // Group visits by dog_id and station_id
//       const groupedVisits: { [key: string]: GroupedVisit } = {};
//       dogVisits.forEach((visit: Visit) => {
//         const key = `${visit.dog_id}-${visit.station_id}`;
//         if (!groupedVisits[key]) {
//           groupedVisits[key] = {
//             dog_id: visit.dog_id,
//             station_id: visit.station_id,
//             visits: [],
//           };
//         }
//         groupedVisits[key].visits.push(visit);
//       });

//       // Prepare datasets for each dog-station combination
//       const datasets = Object.keys(groupedVisits).map((key) => {
//         const group = groupedVisits[key];

//         // Map visit times to "HH:MM" format
//         group.visits.map((visit: Visit) => {
//           const date = new Date(visit.visit_time);
//           const hours = String(date.getHours()).padStart(2, "0"); // Ensure two-digit format
//           const minutes = String(date.getMinutes()).padStart(2, "0"); // Ensure two-digit format
//           return `${hours}:${minutes}`; // Combine hours and minutes
//         });

//         const visitDistances = group.visits.map(
//           (visit: Visit) => visit.distance
//         );

//         // Fetch the dog's name and station's name
//         const dogName = dogNamesMap[group.dog_id] || `Dog ${group.dog_id}`;
//         const stationName =
//           stationNamesMap[group.station_id] || `Station ${group.station_id}`;

//         return {
//           label: `${dogName} at ${stationName}`, // Use dog's name and station's name
//           data: visitDistances,
//           fill: false,
//           borderColor: getColorForDogStation(dogName, stationName), // Color based on names
//           tension: 0.4,
//         };
//       });

//       // Set the chart data based on grouped visits
//       chartData.value = {
//         labels: [
//           ...new Set(
//             dogVisits.map((visit: Visit) => {
//               const date = new Date(visit.visit_time);
//               const hours = String(date.getHours()).padStart(2, "0");
//               const minutes = String(date.getMinutes()).padStart(2, "0");
//               return `${hours}:${minutes}`; // Unique time points in "HH:MM"
//             })
//           ),
//         ], // Unique time points for x-axis
//         datasets: datasets,
//       };
//     }
//   } catch (error) {
//     console.error("Error fetching dog visits:", error);
//   }
// }

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
  chartOptions.value = setChartOptions();

  // Fetch and update chart data every minute
  fetchDogVisits(); // Initial fetch
  setInterval(fetchDogVisits, 2000); // Repeat every 2 seconds
});

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
        min: -90,
        max: -20,
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
  };
};
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
