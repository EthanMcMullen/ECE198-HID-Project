const axios = require("axios");

const API_URL = "http://localhost:3000";
const PATIENT_ID = "691641bda337ec14e57f5b18";

const sensorReadings = [];
const startTime = new Date("2025-11-21T00:00:00.000Z").getTime();
const endTime = new Date("2025-11-22T00:00:00.000Z").getTime();
const intervalMs = 5 * 60 * 1000; // 5 min

let prevHeartRate = 70; // normal-ish start

// Optional 10-min low HR for episodes
const lowHRStart = startTime + Math.floor(Math.random() * (endTime - startTime - 2 * intervalMs));
const lowHREnd = lowHRStart + 2 * intervalMs;

for (let time = startTime; time < endTime; time += intervalMs) {
  const timestamp = new Date(time).toISOString();
  const hour = new Date(time).getUTCHours();

  let avgHeartRate;

  // Low HR episode (e.g., brief hypotension or arrhythmia)
  if (time >= lowHRStart && time < lowHREnd) {
    avgHeartRate = 45 + Math.floor(Math.random() * 10); // 45-54 bpm
  } else {
    // Base HR drift: slightly higher during day, lower at night
    const base = (hour >= 8 && hour < 22) ? 75 : 60;

    // Random walk to simulate delirium-like variability
    const change = Math.floor(Math.random() * 10) - 5; // small +/– swings
    avgHeartRate = prevHeartRate + change;

    // Drift towards base (slowly)
    avgHeartRate += Math.floor((base - avgHeartRate) * 0.1);

    // Occasional spike (pain, agitation, delirium)
    if (Math.random() < 0.05) {
      avgHeartRate += 20 + Math.floor(Math.random() * 15); // +20 to +35 bpm
    }

    // Clamp realistic elderly HR
    if (avgHeartRate < 40) avgHeartRate = 40;
    if (avgHeartRate > 140) avgHeartRate = 140;

    prevHeartRate = avgHeartRate;
  }

  // Motion: mostly low, occasional movement (restless/confused)
  let motionPercent = Math.floor(Math.random() * 5);
  if (Math.random() < 0.1) motionPercent += Math.floor(Math.random() * 20); // brief agitation

  sensorReadings.push({ timestamp, avgHeartRate, motionPercent });
}

async function postData() {
  for (const reading of sensorReadings) {
    try {
      await axios.post(`${API_URL}/patients/${PATIENT_ID}/data`, reading, {
        headers: { "Content-Type": "application/json" },
      });
      console.log("Posted:", reading.timestamp, "HR:", reading.avgHeartRate);
    } catch (err) {
      console.error("Failed to post:", reading.timestamp, err.response?.data || err.message);
    }
  }
}

postData();
