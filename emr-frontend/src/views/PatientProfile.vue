<template>
  <section>
    <div class="hero-body is-flex is-justify-content-space-between is-align-items-center">
      <p class="title" v-if="patient">Patient Profile : {{ patient.firstName + " " + patient.lastName }}</p>
      <button v-if="token" class="button is-info" @click="goBack">Back to Patients</button>
    </div>
  </section>

  <div v-if="patient">
    <div class="box my-4" style="margin: 20px">
      <div class="">
        <div class="columns is-mobile">
          <div class="column has-text-centered">
            <p class="title has-text-centered">Patient Info</p>
          </div>
        </div>
        <div class="columns has-text-centered info">
          <div class="column">
            <div class="columns">
              <div class="column">
                <p class="subtitle">First Name : {{ patient.firstName }}</p>
                <p class="subtitle">Last Name : {{ patient.lastName }}</p>
              </div>
              <div class="column">
                <p class="subtitle">Sex : {{ patient.sex }}</p>
                <p class="subtitle">Health Card Number : {{ patient.cardNumber }}</p>
              </div>
              <div class="column">
                <p class="subtitle">DOB : {{ patient.dateOfBirth }}</p>
                <p class="subtitle">Address : {{ patient.address }}</p>
              </div>
              <div class="column">
                <p class="subtitle">Phone : {{ patient.contact.phone }}</p>
                <p class="subtitle">Email : {{ patient.contact.email }}</p>
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>


    <div class="box my-4" style="margin: 20px">
      <div class="">
            <p class="title has-text-centered">Hospital Induced Delerium Risk Factors</p>
        <div class="columns">
          <div class="column">
            <p class="title has-text-centered"></p>
            <table class="table is-fullwidth">
              <thead>
                <tr>
                  <th class="has-text-centered is-size-5">Average Heart Rate</th>
                  <th class="has-text-centered is-size-5">Average Mobility</th>
                  <th class="has-text-centered is-size-5">Estimated HID Risk</th>
                </tr>
              </thead>
              <tbody>
                <tr>
                  <td class="has-text-centered">{{ patient.HIDRisk[0]?.heartRate }}</td>
                  <td class="has-text-centered">{{ patient.HIDRisk[0]?.averageMovement }}</td>
                  <td class="has-text-centered">{{ patient.HIDRisk[0]?.overallRisk }}</td>
                </tr>
              </tbody>
            </table>
            <div class="has-text-centered" v-if="token">
              <button class="button is-warning is-size-5 mt-3" @click="inputForm(true)">Input Risk Factors Manually?</button>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="box my-4" style="margin: 20px">
      <p class="title has-text-centered">Heart Rate Over Time</p>
      <div class="chart-container" style="position: relative; height: 300px;">
        <LineChartHeartRate v-if="sensorData.length" :chart-data="heartRateChartData" :chart-options="heartRateChartOptions" />
        <p v-else class="has-text-centered">No heart rate data available.</p>
      </div>
    </div>

    <div class="box my-4" style="margin: 20px">
      <p class="title has-text-centered">Mobility Over Time</p>
      <div class="chart-container" style="position: relative; height: 300px;">
        <LineChartMobility v-if="sensorData.length" :chart-data="mobilityChartData" :chart-options="mobilityChartOptions" />
        <p v-else class="has-text-centered">No mobility data available.</p>
      </div>
    </div>


    <div class="overlay" v-if="inputOpen">
      <div class="box">
        <form>
          <div class="has-text-centered">
            
          </div>
          <div class="columns has-text-centered">
            <div class="column">
              <div class="field">
                  <label class="label">Heart Rate</label>
                    <div class="control">
                      <input class="input" type="string" v-model="manualHID.heartRate">
                    </div>
                </div>
            </div>
            <div class="column">
              <div class="field">
                <label class="label">Average Motion</label>
                  <div class="control">
                    <input class="input" type="string" v-model="manualHID.averageMotion">
                  </div>
              </div>
            </div>
            <div class="column">
              <div class="field">
                <label class="label">Risk Factor</label>
                  <div class="control">
                    <input class="input" type="string" v-model="manualHID.riskFactor">
                  </div>
              </div>
            </div>
          </div>
          

          <div class="columns has-text-centered">
            <div class="column">
              <button class="button is-danger is-size-5 mt-3" @click="inputInformation">Submit HID Factors</button>
            </div>
          </div>
        </form>
      </div>
    </div>
  </div>
    
    
</template>

<script>
import axios from "axios";
import { h } from 'vue';
import { Chart } from 'chart.js';
import {
  Chart as ChartJS,
  Title,
  Tooltip,
  Legend,
  LineElement,
  PointElement,
  LinearScale,
  CategoryScale
} from 'chart.js';

import { Line } from 'vue-chartjs';

ChartJS.register(
  Title,
  Tooltip,
  Legend,
  LineElement,
  PointElement,
  LinearScale,
  CategoryScale
);


const API_URL = 'http://localhost:3000';


export default {
  name: "PatientProfile",
  data() {
    return {
      heartRateChartData: {
      labels: [],
      datasets: [
        {
          label: "Heart Rate (bpm)",
          backgroundColor: "rgba(255, 99, 132, 0.2)",
          borderColor: "rgba(255, 99, 132, 1)",
          pointBackgroundColor: "rgba(255, 99, 132, 1)",
          data: []
        }
      ]
    },
    heartRateChartOptions: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: { title: { display: true, text: "Time" } },
        y: { title: { display: true, text: "Heart Rate (bpm)" }, beginAtZero: true }
      }
    },

    mobilityChartData: {
      labels: [],
      datasets: [
        {
          label: "Average Mobility (%)",
          backgroundColor: "rgba(54, 162, 235, 0.2)",
          borderColor: "rgba(54, 162, 235, 1)",
          pointBackgroundColor: "rgba(54, 162, 235, 1)",
          data: []
        }
      ]
    },
    mobilityChartOptions: {
      responsive: true,
      maintainAspectRatio: false,
      scales: {
        x: { title: { display: true, text: "Time" } },
        y: { title: { display: true, text: "Average Mobility (%)" }, beginAtZero: true }
      }
    },

      sensorData: [],
      patient: null,
      windowOpen: false,
      inputOpen: false,
      token: null,
      HIDFactors: [],
      manualHID: {
        heartRate: "",
        averageMotion: "",
        riskFactor: "",
      }
    };
  },
  methods: {
    async fetchPatient() {
      try {
        const response = await axios.get(`${API_URL}/patients/${this.$route.params.patient}`);
        this.patient = response.data;
        console.log("patient", this.patient);
      } catch (error) {
        console.error("Failed to fetch patient data:", error);
      }
    },
    async fetchData() {
      try {
        const response = await axios.get(`${API_URL}/patients/${this.$route.params.patient}/data`);
        this.sensorData = response.data.data || response.data; // depends on backend response
        console.log("Sensor data:", this.sensorData);

        // Populate heart rate chart
        this.heartRateChartData.labels = this.sensorData.map(d => {
          const date = new Date(d.timestamp);
          return date.getHours() + ":" + String(date.getMinutes()).padStart(2, "0");
        });

        this.heartRateChartData.datasets[0].data = this.sensorData.map(d => d.avgHeartRate);

        // After building heartRateChartData
        this.mobilityChartData = {
          labels: this.sensorData.map(d => new Date(d.timestamp).toLocaleTimeString()),
          datasets: [
            {
              label: "Average Mobility (%)",
              backgroundColor: "rgba(54, 162, 235, 0.2)",
              borderColor: "rgba(54, 162, 235, 1)",
              pointBackgroundColor: "rgba(54, 162, 235, 1)",
              data: this.sensorData.map(d => Number(d.motionPercent)) // or avgMotion if you rename
            }
          ]
        };


        // (Optional) populate motion chart if you want a second chart
        //this.motionChartData.labels = this.sensorData.map(d =>
        //  new Date(d.timestamp).toLocaleTimeString()
        //);
        //this.motionChartData.datasets[0].data = this.sensorData.map(d => d.avgMotion);

      } catch (error) {
        console.error("Failed to fetch sensor data:", error);
      }

      console.log("sensorData", this.sensorData);

            // Sort by time (optional, but keeps the chart in order)
      this.sensorData.sort((a, b) => new Date(a.timestamp) - new Date(b.timestamp));

      // Rebuild the chart data completely
      this.heartRateChartData = {
        labels: this.sensorData.map(d => new Date(d.timestamp).toLocaleTimeString()),
        datasets: [
          {
            label: "Heart Rate (bpm)",
            backgroundColor: "rgba(255, 99, 132, 0.2)",
            borderColor: "rgba(255, 99, 132, 1)",
            pointBackgroundColor: "rgba(255, 99, 132, 1)",
            data: this.sensorData.map(d => Number(d.avgHeartRate)) // make sure it's a number
          }
        ]
      };

    },
    goBack() {
      this.$router.push("/patients");
    },
    async inputInformation(event) {
      event.preventDefault(); 

      try {
        
        console.log("Sending HID data:", this.manualHID);

        const response = await axios.put(`${API_URL}/patients/${this.$route.params.patient}`, {
          HIDRisk: {
            heartRate: this.manualHID.heartRate,
            averageMovement: this.manualHID.averageMotion,
            overallRisk: this.manualHID.riskFactor
          }
        });

        console.log("Updated patient HID factors:", response.data);

      
        this.inputOpen = false;
        await this.fetchPatient();
      } catch (error) {
        console.error("Failed to update HID factors:", error);
      }
    },
    inputForm(open) {
      this.inputOpen = open;
    }

  },
  created() {
    this.token = localStorage.getItem("token");
    this.fetchPatient();
    this.fetchData();
  },
  components: {
    LineChartHeartRate: {
      props: ["chartData", "chartOptions"],
      render() {
        return h('canvas', { ref: 'canvas' });
      },
      mounted() {
        this.chartInstance = new Chart(this.$refs.canvas.getContext("2d"), {
          type: "line",
          data: this.chartData,
          options: this.chartOptions
        });
      },
      watch: {
        chartData: {
          deep: true,
          handler(newData) {
            if (this.chartInstance) this.chartInstance.destroy();
            this.chartInstance = new Chart(this.$refs.canvas.getContext("2d"), {
              type: "line",
              data: newData,
              options: this.chartOptions
            });
          }
        }
      }
    },


    LineChartMobility: {
      props: ["chartData", "chartOptions"],
      render() {
        return h('canvas', { ref: 'canvas' });
      },
      mounted() {
        this.chartInstance = new Chart(this.$refs.canvas.getContext("2d"), {
          type: "line",
          data: this.chartData,
          options: this.chartOptions
        });
      },
      watch: {
        chartData: {
          deep: true,
          handler(newData) {
            if (this.chartInstance) this.chartInstance.destroy();
            this.chartInstance = new Chart(this.$refs.canvas.getContext("2d"), {
              type: "line",
              data: newData,
              options: this.chartOptions
            });
          }
        }
      }
    }

    
  }


};
</script>

<style scoped>
.columns {
  padding: 20px;
}

.info {
  padding-bottom: 50px;
  padding-top: 50px;
}

.is-flex {
  display: flex;
}

.is-justify-content-space-between {
  justify-content: space-between;
}

.is-align-items-center {
  align-items: center;
}

.card {
  box-shadow: 0 2px 3px rgba(10, 10, 10, 0.1), 0 0 0 1px rgba(10, 10, 10, 0.1);
  border-radius: 6px;
}

.card-content {
  padding: 1.5rem 2rem;
}
</style>
