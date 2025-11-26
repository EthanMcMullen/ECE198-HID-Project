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
                  <th class="has-text-centered is-size-5">Recent Heart Rate</th>
                  <th class="has-text-centered is-size-5">Average Mobility</th>
                  <th class="has-text-centered is-size-5">Recent Mobility</th>
                  <th class="has-text-centered is-size-5">Estimated HID Risk</th>
                </tr>
              </thead>
              <tbody>
                <tr>
                  <td class="has-text-centered">{{ patient.HIDRisk[0]?.heartRate }}</td>
                  <td class="has-text-centered">{{ recentData.avgHeartRate }}</td>
                  <td class="has-text-centered">{{ patient.HIDRisk[0]?.averageMovement }}</td>
                  <td class="has-text-centered">{{ recentData.motionPercent }}</td>
                  <td class="has-text-centered">{{ patient.HIDRisk[0]?.overallRisk }}% {{ HIDmessage }}</td>
                </tr>
              </tbody>
            </table>
            <div class="has-text-centered" v-if="token">
              <button class="button is-warning is-size-5 mt-3" @click="inputInformation()">Use Current Values as Baseline?</button>
            </div>
          </div>
        </div>
      </div>
    </div>

    <div class="chart-container" style="position: relative; height: 300px;">
      <p class="title has-text-centered">Heart Rate</p>
      <LineChartHeartRate 
        v-if="heartRateChartData.datasets[0].data.length" 
        :chart-data="heartRateChartData" 
        :chart-options="heartRateChartOptions" 
      />
      <p v-else class="title has-text-centered">No heart rate data available.</p>
    </div>

    <div class="columns"></div>
    <div class="columns"></div>
    

    <div class="chart-container" style="position: relative; height: 300px;">
      <p class="title has-text-centered">Position</p>
      <LineChartMobility 
        v-if="mobilityChartData.datasets[0].data.length" 
        :chart-data="mobilityChartData" 
        :chart-options="mobilityChartOptions" 
      />
      <p v-else class="title has-text-centered">No mobility data available.</p>
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

    recentData: {
      avgHeartRate: "",
      motionPercent: "",
      timestamp: "",
    }, 

    averageData: {
      avgHeartRate: "",
      motionPercent: "",
    },

      sensorData: [],
      patient: null,
      windowOpen: false,
      inputOpen: false,
      HIDmessage: "",
      token: null,
      HIDFactors: [],
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
      this.setHIDMessage();
    },
    async fetchData() {
      try {
        const response = await axios.get(`${API_URL}/patients/${this.$route.params.patient}/data`);
        let rawData = response.data.data || response.data;

        const now = Date.now();

        const cleanedData = rawData
          .map(d => ({ ...d, timestamp: new Date(d.timestamp.replace(/Z$/, "")) }))
          .filter(d => d.timestamp.getTime() <= now)
          .sort((a, b) => a.timestamp - b.timestamp);

        this.sensorData = cleanedData; 

        const heartRateData = cleanedData.filter(d => Number(d.avgHeartRate) >= 30);
        const mobilityData  = cleanedData.filter(d => Number(d.motionPercent) >= 0);

        this.heartRateChartData = {
          labels: heartRateData.map(d =>
            d.timestamp.toLocaleString([], { month:'2-digit', day:'2-digit', hour:'2-digit', minute:'2-digit' })
          ),
          datasets: [
            {
              label: "Heart Rate (bpm)",
              backgroundColor: "rgba(255, 99, 132, 0.2)",
              borderColor: "rgba(255, 99, 132, 1)",
              pointBackgroundColor: "rgba(255, 99, 132, 1)",
              data: heartRateData.map(d => Number(d.avgHeartRate))
            }
          ]
        };

        // Mobility chart
        this.mobilityChartData = {
          labels: mobilityData.map(d =>
            d.timestamp.toLocaleString([], { month:'2-digit', day:'2-digit', hour:'2-digit', minute:'2-digit' })
          ),
          datasets: [
            {
              label: "Average Mobility (%)",
              backgroundColor: "rgba(54, 162, 235, 0.2)",
              borderColor: "rgba(54, 162, 235, 1)",
              pointBackgroundColor: "rgba(54, 162, 235, 1)",
              data: mobilityData.map(d => Number(d.motionPercent))
            }
          ]
        };

        const lastHeartRate = heartRateData[heartRateData.length - 1];
        const lastMobility = mobilityData[mobilityData.length - 1];

        this.recentData = {
          avgHeartRate: lastHeartRate ? lastHeartRate.avgHeartRate : "",
          motionPercent: lastMobility ? lastMobility.motionPercent : "",
          timestamp: lastHeartRate ? lastHeartRate.timestamp.toISOString() : "",
        };


      } catch (error) {
        console.error("Failed to fetch sensor data:", error);
      }
    },



    goBack() {
      this.$router.push("/patients");
    },
    calculateHIDRisk() {
      // 75+ Very high
      // 50+ High
      // 25+ Medium
      // 0+ Low
      // 0 None

      // Heart Rate
      let heartRateHIDComponent = 0
      let heartRateRatio

      if(this.patient.HIDRisk[0].heartRate == 0) {
        heartRateRatio = 0
      } else {
        heartRateRatio = this.averageData.avgHeartRate/this.patient.HIDRisk[0].heartRate
      }


      console.log("hr")
      console.log(heartRateRatio)


      let averageMovementHIDComponent = 0
      

      if(heartRateRatio < 1.065) {
        heartRateHIDComponent = 0
      } else if (heartRateRatio > 1.25) {
        heartRateHIDComponent = 100
      } else {
        heartRateHIDComponent = (100)*((heartRateRatio-1.065)/(1.25-1.065))
      }

      if(this.averageData.motionPercent > 4) {
        averageMovementHIDComponent = 0
      } else {
        averageMovementHIDComponent = (4 - this.averageData.motionPercent)*(25)
      }

      console.log(heartRateHIDComponent)
      console.log(averageMovementHIDComponent)

      this.patient.HIDRisk[0].overallRisk = Math.floor((0.25)*(heartRateHIDComponent) + (0.75)*(averageMovementHIDComponent))


      // Excercise levels are significantly more

      // 75% excercise
      // Under 4 is concerning
      // 100-4 is a 0
     
      // 0 is 100
      // 
      // 25% heart rate
      // HIgher heart rate
      // increase linearly
      // 6.5% increase in heart rate is 0
      // 25% increate in heart rate is 100
    },

    async inputInformation() {
      try {
        if (!this.sensorData.length) {
          console.warn("No sensor data available to calculate averages.");
          return;
        }

        // Filter valid readings
        const validHeartRateData = this.sensorData.filter(d => Number(d.avgHeartRate) >= 0);
        const validMotionData = this.sensorData.filter(d => Number(d.motionPercent) >= 0);

       
        let totalHeartRate = 0;
        let heartRateCount = validHeartRateData.length;

        let avgHeartRate = 0

        if (heartRateCount > 0) {
          for (let i = 0; i < heartRateCount; i++) {
            totalHeartRate += Number(validHeartRateData[i].avgHeartRate);
          }
          avgHeartRate = totalHeartRate / heartRateCount;
        } 

        let totalMotion = 0;
        let motionCount = validMotionData.length;

        let avgMotion = 0

        if (motionCount > 0) {
          for (let i = 0; i < motionCount; i++) {
            totalMotion += Number(validMotionData[i].motionPercent);
          }
          avgMotion = totalMotion / motionCount;
        }


        this.averageData.avgHeartRate = avgHeartRate.toFixed(2);
        this.averageData.motionPercent = avgMotion.toFixed(2);

        console.log("Sending HID data based on daily averages:", {
          heartRate: this.averageData.avgHeartRate,
          averageMovement: this.averageData.motionPercent,
        });

        this.calculateHIDRisk();

        const response = await axios.put(`${API_URL}/patients/${this.$route.params.patient}`, {
          HIDRisk: {
            heartRate: this.averageData.avgHeartRate,
            averageMovement: this.averageData.motionPercent,
            overallRisk: this.patient.HIDRisk[0].overallRisk
          }
        });

        console.log("Updated patient HID factors:", response.data);

        this.setHIDMessage();

        this.inputOpen = false;
        await this.fetchPatient();
      } catch (error) {
        console.error("Failed to update HID factors:", error);
      }
    },
    setHIDMessage() {
      if(!this.patient.HIDRisk[0].overallRisk) {
        this.HIDmessage = ""
      } else if(this.patient.HIDRisk[0].overallRisk === 0) {
        this.HIDmessage = "No Risk"
      } else if(this.patient.HIDRisk[0].overallRisk < 25) {
        this.HIDmessage = "Low Risk"
      } else if(this.patient.HIDRisk[0].overallRisk < 50) {
        this.HIDmessage = "Moderate Risk"
      } else if(this.patient.HIDRisk[0].overallRisk < 75) {
        this.HIDmessage = "High Risk"
      } else {
        this.HIDmessage = "Very High Risk"
      }
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
