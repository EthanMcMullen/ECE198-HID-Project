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


    <div class="box my-4" v-if="appointments && appointments.length > 0 || token" style="margin: 20px">
      <div class="">
            <p class="title has-text-centered">Hospital Induced Delerium Risk Factors</p>
        <div class="columns">
          <div class="column">
            <p class="title has-text-centered"></p>
            <table class="table is-fullwidth" v-if="appointments && appointments.length > 0">
              <thead>
                <tr>
                  <th></th>
                  <th class="has-text-centered is-size-5">Heart Rate</th>
                  <th class="has-text-centered is-size-5">Mobility</th>
                  <th class="has-text-centered is-size-5">Estimated HID Risk</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="appointment in appointments" :key="appointment._id">
                  <td><button class="button is-danger is-small" @click="deleteAppointment(appointment._id)">X</button></td>
                  <td class="has-text-centered">{{ appointment.notes }}</td>
                  <td class="has-text-centered">{{ appointment.date }}</td>
                  <td class="has-text-centered">{{ appointment.startTime }}</td>
                </tr>
              </tbody>
            </table>
            <p v-else class="section subtitle is-fullwidth has-text-centered">There are no data for {{ patient.firstName + " " + patient.lastName }} currently.</p>
            <div class="has-text-centered" v-if="token">
              <button class="button is-warning is-size-5 mt-3" @click="inputInformation()">Input Risk Factors Manually?</button>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
    
    
</template>

<script>
import axios from "axios";

const API_URL = 'http://localhost:3000';


export default {
  name: "PatientProfile",
  data() {
    return {
      patient: null,
      windowOpen: false,
      contactOpen: false,
      token: null,
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
    goBack() {
      this.$router.push("/patients");
    },
    inputInformation() {
      this.$router.push("/dashboard")
    }

  },
  created() {
    this.token = localStorage.getItem("token");
    this.fetchPatient();
  },
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
