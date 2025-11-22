<template>
<!-- <section class="hero is-primary is-fullheight-with-navbar"> -->
  <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/bulma@0.9.3/css/bulma.min.css">
  <section class="hero is-fullheight-with-navbar section has-background-info-light">
    <div v-if="!hasSearched" class="columns is-centered">
      <div class="column is-half mt-5">
        <h1 class="title">Patient Search</h1>
        <div class="box">
          <div v-if="successMessage" class="notification is-success">
            {{ successMessage }}
          </div>
          <div v-if="errorMessage" class="notification is-danger">
            {{ errorMessage }}
            {{ errorMessage }}
          </div>
          <form @submit.prevent="submitForm">
            <div class="columns">
              <div class="column">
                <div class="field">
                  <label class="label">Patient First Name</label>
                  <div class="control">
                    <input class="input" type="text" v-model="patientFirstName">
                  </div>
                </div>
                <div class="field">
                  <label class="label">Last Name</label>
                  <div class="control">
                    <input class="input" type="text" v-model="lastName">
                  </div>
                </div>
                <div class="field">
                  <label class="label">Date of Birth</label>
                  <div class="control">
                    <input class="input" type="date" v-model="dateOfBirth">
                  </div>
                </div>
              </div>
              <div class="column">
                <div class="field">
                  <label class="label">Sex</label>
                  <div class="select is-fullwidth">
                    <select v-model="sex">
                      <option value="Male">Male</option>
                      <option value="Female">Female</option>
                    </select>
                  </div>
                </div>
                <div class="field">
                  <label class="label">Health Card</label>
                  <div class="control">
                    <input class="input" type="text" v-model="cardNumber">
                  </div>
                </div>
                <div class="field">
                  <label class="label">Phone Number</label>
                  <div class="control">
                    <input class="input" type="text" v-model="phoneNum">
                  </div>
                </div>
              </div>
            </div>
            <div class="field">
              <label class="label">Address</label>
              <div class="control">
                <input class="input" type="text" v-model="address">
              </div>
            </div>
            <div class="field">
              <label class="label">Email</label>
              <div class="control">
                <input class="input" type="text" v-model="email">
              </div>
            </div>
            <div class="field">
              <label class="label">Emergency Contact Name</label>
              <div class="control">
                <input class="input" type="text" v-model="emergencyContactName">
              </div>
            </div>
            <div class="field">
              <label class="label">Emergency Contact Relationship</label>
              <div class="control">
                <input class="input" type="text" v-model="emergencyContactRelationship">
              </div>
            </div>
            <div class="field">
              <label class="label">Emergency Contact Phone</label>
              <div class="control">
                <input class="input" type="text" v-model="emergencyContactPhone">
              </div>
            </div>
<!--            <div class="field">-->
<!--              <label class="label">Conditions</label>-->
<!--              <div class="control">-->
<!--                <multiselect v-model="selectedConditions" :options="conditions" label="name" track-by="_id" multiple close-on-select="false" placeholder="Select conditions"></multiselect>-->
<!--              </div>-->
<!--            </div>-->
            <div class="field is-grouped">
              <div class="control">
                <button class="button is-primary" @click="submitForm">Search</button>
              </div>
              <div class="control">
                <button type="button" class="button is-link" @click="resetForm">Reset</button>
              </div>
              <div class="control">
                <button type="button" class="button is-info" @click="createNewPatient">Add New Patient</button>
              </div>
              <!-- <div class="control"> --->
                <!-- <button type="button" class="button is-success" @click="insertTestPatients">Insert 15 Test Patients</button> -->
              <!-- /div> -->
            </div>
          </form>
        </div>
      </div>
    </div>
    <div v-if="hasSearched" class="columns is-centered">
      <div class="column is-full">
        <br>
        <h1 class="title">Filtered Patients</h1>
        <div class="box">
          <table class="table is-fullwidth has-text-centered ">
          <thead>
          <tr>
            <th>First Name</th>
            <th>Last Name</th>
            <th>Date of Birth</th>
            <th>Sex</th>
            <th>Address</th>
            <th>Health Card</th>
            <th>Actions</th>
          </tr>
          </thead>
          <tbody>
          <tr v-for="patient in paginatedPatients" :key="patient._id">
            <td>{{ patient.firstName }}</td>
            <td>{{ patient.lastName }}</td>
            <td>{{ patient.dateOfBirth }}</td>
            <td>{{ patient.sex }}</td>
            <td>{{ patient.address }}</td>
            <td>{{ patient.cardNumber }}</td>
            <td>
              <button class="button is-link" @click="goToPatientProfile(patient._id)">Profile</button>
              <button class="button is-danger" @click="deletePatient(patient._id)">Delete</button>

            </td>
          </tr>
          </tbody>
        </table>
        </div>
        <nav class="pagination is-centered" role="navigation" aria-label="pagination">
          <button class="pagination-previous" @click="prevPage" :disabled="currentPage === 1">Previous</button>
          <button class="pagination-next" @click="nextPage" :disabled="currentPage * pageSize >= filteredPatients.length">Next</button>
          <ul class="pagination-list">
            <li><a class="pagination-link" :class="{'is-current': n === currentPage}" v-for="n in Math.ceil(filteredPatients.length / pageSize)" :key="n" @click="currentPage = n">
              {{ n }}
            </a></li>
          </ul>
        </nav>
        <div class="control">
          <button class="button is-link pl-4" @click="showSearch">Go Back to Search</button>
        </div>
      </div>
    </div>
  </section>

</template>

<script>
import axios from "axios";
import Multiselect from 'vue-multiselect';
import 'vue-multiselect/dist/vue-multiselect.css';

const API_URL = 'http://localhost:3000';


export default {
  components: { Multiselect },
  data() { // data
    return {
      currentPage: 1,
      pageSize: 10,
      patientFirstName: '',
      lastName: '',
      dateOfBirth: '',
      address: '',
      sex: '',
      cardNumber: '',
      patients: [],
      conditions: [],
      selectedConditions: [],
      filteredPatients: [],
      hasSearched: false,
      phoneNum: '',
      email: '',
      emergencyContactName: '',
      emergencyContactRelationship: '',
      emergencyContactPhone: '',
      successMessage: '',
      errorMessage: ''
    };
  },
  setup() {
    let token = localStorage.getItem('token') || null
    
    if(!token) {
      window.location = ('/')
    }
  },
  computed: {
    paginatedPatients() { // List of the patients that should be displayed on the current page of pagination
      const start = (this.currentPage - 1) * this.pageSize;
      const end = start + this.pageSize;
      return this.filteredPatients.slice(start, end);
    }
  },
  methods: {
    nextPage() { // Goes to the next page of pagination
      if ((this.currentPage * this.pageSize) < this.filteredPatients.length) {
        this.currentPage++;
      }
    },
    prevPage() { // goes to the previous page of pagination
      if (this.currentPage > 1) {
        this.currentPage--;
      }
    },
    async fetchPatients() { // Fetches an array of all the patients from the database
      try {
        const response = await axios.get(`${API_URL}/patients`);
        this.patients = response.data;
      } catch (error) {
        console.error(error);
      }
    },
    async deletePatient(patientId) { // Deletes a selected patient and all things tied to that patient
      try {
        await axios.delete(`${API_URL}/patients/${patientId}`);
        await axios.delete(`${API_URL}/appointments/patient/${patientId}`)

        this.successMessage = 'Patient deleted successfully.';
        this.filteredPatients = this.filteredPatients.filter(patient => patient._id !== patientId);
      } catch (error) {
        console.error(error);
      }
      this.showSearch()
      location.reload()
    },
    submitForm() { // For each parameter in the search for patients page, find which patients match & add return an array (that is used in the table)
      this.filteredPatients = this.patients.filter(patient => {
        const firstNameMatch = this.patientFirstName === '' || patient.firstName.toLowerCase().includes(this.patientFirstName.toLowerCase());
        const lastNameMatch = this.lastName === '' || patient.lastName.toLowerCase().includes(this.lastName.toLowerCase());
        const dateOfBirthMatch = this.dateOfBirth === '' || patient.dateOfBirth === this.dateOfBirth;
        const addressMatch = this.address === '' || patient.address.toLowerCase().includes(this.address.toLowerCase());
        const cardNumberMatch = this.cardNumber === '' || patient.cardNumber.toLowerCase().includes(this.cardNumber.toLowerCase());
        const sexMatch = this.sex === '' || patient.sex.toLowerCase().includes(this.sex.toLowerCase());
        const conditionsMatch = this.selectedConditions.length === 0 || this.selectedConditions.every(condition => patient.conditions.includes(condition._id));
        return firstNameMatch && lastNameMatch && dateOfBirthMatch && addressMatch && cardNumberMatch && sexMatch && conditionsMatch;
      });

      this.hasSearched = true;
      localStorage.setItem('searchResults', JSON.stringify(this.filteredPatients));
      localStorage.setItem('searchState', JSON.stringify({
        patientFirstName: this.patientFirstName,
        lastName: this.lastName,
        dateOfBirth: this.dateOfBirth,
        address: this.address,
        sex: this.sex,
        cardNumber: this.cardNumber,
        phoneNum: this.phoneNum,
        email: this.email,
        emergencyContactName: this.emergencyContactName,
        emergencyContactRelationship: this.emergencyContactRelationship,
        emergencyContactPhone: this.emergencyContactPhone,
        selectedConditions: this.selectedConditions,
        hasSearched: this.hasSearched
      }));
    },
    showSearch() { // Shows the two different pages (search & table)
      this.hasSearched = false;
      localStorage.removeItem('searchResults');
      localStorage.removeItem('searchState');
    },
    goToPatientProfile(patient_id) { // Sends you to the patient profile page differentiated through the patiends id
      this.$router.push({ name: 'PatientProfile', params: { patient: patient_id } });
    },
    async createNewPatient() { // Creates a patient using the information provided on the search page
      let newPatient = {
        firstName: this.patientFirstName,
        lastName: this.lastName,
        dateOfBirth: this.dateOfBirth,
        sex: this.sex,
        address: this.address,
        contact: {
          phone: this.phoneNum,
          email: this.email,
        },
        emergencyContact: {
          name: this.emergencyContactName,
          relationship: this.emergencyContactRelationship,
          phone: this.emergencyContactPhone
        },
        cardNumber: this.cardNumber,
        conditions: this.selectedConditions.map(condition => condition._id),
        contactPreference: "email",
        HIDRisk : {
          heartRate : "No Data",
          averageMovement : "No Data",
          overallRisk: "No Data"
        }
      };

      if (this.patientFirstName !== '' && this.lastName !== '' && this.dateOfBirth !== '' && this.sex !== '' && this.address !== '' && this.phoneNum !== '' && this.email !== '' && this.emergencyContactName !== '' && this.emergencyContactRelationship !== '' && this.emergencyContactPhone !== '' && this.cardNumber !== '') {
        try {
          await axios.post(`${API_URL}/patients`, newPatient);
          this.successMessage = 'New patient added successfully.';
          this.errorMessage = '';
          this.resetForm();
          await this.fetchPatients();
        } catch (error) {
          this.successMessage = '';
          this.errorMessage = 'Failed to add new patient.';
        }
      } else {
        this.successMessage = '';
        this.errorMessage = 'Please ensure you fill out all fields.';
      }
    },
    resetForm() { // Resets all the values on the form to ''
      this.patientFirstName = '';
      this.lastName = '';
      this.dateOfBirth = '';
      this.address = '';
      this.sex = '';
      this.cardNumber = '';
      this.phoneNum = '';
      this.email = '';
      this.emergencyContactName = '';
      this.emergencyContactRelationship = '';
      this.emergencyContactPhone = '';
      this.selectedConditions = [];
    },
    insertTestPatients() {
    const testPatients = [
      { patientFirstName: 'Alice', lastName: 'Smith', dateOfBirth: '1985-01-01', sex: 'Female', address: '123 Maple St', cardNumber: 'H123456789', phoneNum: '555-1234', email: 'alice.smith@example.com', emergencyContactName: 'Bob Smith', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-5678' },
      { patientFirstName: 'Bob', lastName: 'Jones', dateOfBirth: '1990-05-12', sex: 'Male', address: '456 Oak Ave', cardNumber: 'H987654321', phoneNum: '555-2345', email: 'bob.jones@example.com', emergencyContactName: 'Carol Jones', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-6789' },
      { patientFirstName: 'Carol', lastName: 'Lee', dateOfBirth: '1975-03-23', sex: 'Female', address: '789 Pine Rd', cardNumber: 'H234567890', phoneNum: '555-3456', email: 'carol.lee@example.com', emergencyContactName: 'Dan Lee', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-7890' },
      { patientFirstName: 'Dan', lastName: 'Brown', dateOfBirth: '1982-07-14', sex: 'Male', address: '101 Birch Ln', cardNumber: 'H345678901', phoneNum: '555-4567', email: 'dan.brown@example.com', emergencyContactName: 'Eve Brown', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-8901' },
      { patientFirstName: 'Eve', lastName: 'Davis', dateOfBirth: '1995-11-30', sex: 'Female', address: '202 Cedar St', cardNumber: 'H456789012', phoneNum: '555-5678', email: 'eve.davis@example.com', emergencyContactName: 'Frank Davis', emergencyContactRelationship: 'Father', emergencyContactPhone: '555-9012' },
      { patientFirstName: 'Frank', lastName: 'Wilson', dateOfBirth: '1970-02-18', sex: 'Male', address: '303 Elm Ave', cardNumber: 'H567890123', phoneNum: '555-6789', email: 'frank.wilson@example.com', emergencyContactName: 'Grace Wilson', emergencyContactRelationship: 'Mother', emergencyContactPhone: '555-0123' },
      { patientFirstName: 'Grace', lastName: 'Taylor', dateOfBirth: '1988-09-05', sex: 'Female', address: '404 Spruce Rd', cardNumber: 'H678901234', phoneNum: '555-7890', email: 'grace.taylor@example.com', emergencyContactName: 'Henry Taylor', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-1234' },
      { patientFirstName: 'Henry', lastName: 'Anderson', dateOfBirth: '1992-12-20', sex: 'Male', address: '505 Fir Ln', cardNumber: 'H789012345', phoneNum: '555-8901', email: 'henry.anderson@example.com', emergencyContactName: 'Ivy Anderson', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-2345' },
      { patientFirstName: 'Ivy', lastName: 'Thomas', dateOfBirth: '1980-04-07', sex: 'Female', address: '606 Willow St', cardNumber: 'H890123456', phoneNum: '555-9012', email: 'ivy.thomas@example.com', emergencyContactName: 'Jack Thomas', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-3456' },
      { patientFirstName: 'Jack', lastName: 'Moore', dateOfBirth: '1978-06-16', sex: 'Male', address: '707 Aspen Ave', cardNumber: 'H901234567', phoneNum: '555-0123', email: 'jack.moore@example.com', emergencyContactName: 'Kelly Moore', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-4567' },
      { patientFirstName: 'Kelly', lastName: 'White', dateOfBirth: '1991-08-29', sex: 'Female', address: '808 Poplar Rd', cardNumber: 'H012345678', phoneNum: '555-1235', email: 'kelly.white@example.com', emergencyContactName: 'Leo White', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-5679' },
      { patientFirstName: 'Leo', lastName: 'Harris', dateOfBirth: '1983-01-12', sex: 'Male', address: '909 Cherry Ln', cardNumber: 'H123456780', phoneNum: '555-2346', email: 'leo.harris@example.com', emergencyContactName: 'Mia Harris', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-6780' },
      { patientFirstName: 'Mia', lastName: 'Martin', dateOfBirth: '1976-10-03', sex: 'Female', address: '1010 Maple St', cardNumber: 'H234567801', phoneNum: '555-3457', email: 'mia.martin@example.com', emergencyContactName: 'Nick Martin', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-7891' },
      { patientFirstName: 'Nick', lastName: 'Garcia', dateOfBirth: '1987-05-21', sex: 'Male', address: '1111 Oak Ave', cardNumber: 'H345678012', phoneNum: '555-4568', email: 'nick.garcia@example.com', emergencyContactName: 'Olivia Garcia', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-8902' },
      { patientFirstName: 'Olivia', lastName: 'Martinez', dateOfBirth: '1993-03-11', sex: 'Female', address: '1212 Pine Rd', cardNumber: 'H456789013', phoneNum: '555-5679', email: 'olivia.martinez@example.com', emergencyContactName: 'Paul Martinez', emergencyContactRelationship: 'Father', emergencyContactPhone: '555-9013' },
      { patientFirstName: 'Alice', lastName: 'Brown', dateOfBirth: '1984-09-22', sex: 'Female', address: '1313 Oak St', cardNumber: 'H567890124', phoneNum: '555-1010', email: 'alice.brown@example.com', emergencyContactName: 'Tom Brown', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-1111' },
      { patientFirstName: 'Bob', lastName: 'Green', dateOfBirth: '1991-12-03', sex: 'Male', address: '1414 Maple Ave', cardNumber: 'H678901235', phoneNum: '555-1212', email: 'bob.green@example.com', emergencyContactName: 'Sue Green', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-1313' },
      { patientFirstName: 'Carol', lastName: 'Hall', dateOfBirth: '1986-06-16', sex: 'Female', address: '1515 Pine Rd', cardNumber: 'H789012346', phoneNum: '555-1414', email: 'carol.hall@example.com', emergencyContactName: 'Ethan Hall', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-1515' },
      { patientFirstName: 'Dan', lastName: 'Adams', dateOfBirth: '1982-02-28', sex: 'Male', address: '1616 Birch Ln', cardNumber: 'H890123457', phoneNum: '555-1616', email: 'dan.adams@example.com', emergencyContactName: 'Fiona Adams', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-1717' },
      { patientFirstName: 'Eve', lastName: 'Baker', dateOfBirth: '1994-07-09', sex: 'Female', address: '1717 Cedar St', cardNumber: 'H901234568', phoneNum: '555-1818', email: 'eve.baker@example.com', emergencyContactName: 'George Baker', emergencyContactRelationship: 'Father', emergencyContactPhone: '555-1919' },
      { patientFirstName: 'Frank', lastName: 'Clark', dateOfBirth: '1977-11-19', sex: 'Male', address: '1818 Elm Ave', cardNumber: 'H012345679', phoneNum: '555-2020', email: 'frank.clark@example.com', emergencyContactName: 'Hannah Clark', emergencyContactRelationship: 'Mother', emergencyContactPhone: '555-2121' },
      { patientFirstName: 'Grace', lastName: 'Evans', dateOfBirth: '1989-03-23', sex: 'Female', address: '1919 Spruce Rd', cardNumber: 'H123456791', phoneNum: '555-2222', email: 'grace.evans@example.com', emergencyContactName: 'Ian Evans', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-2323' },
      { patientFirstName: 'Henry', lastName: 'Foster', dateOfBirth: '1993-08-05', sex: 'Male', address: '2020 Fir Ln', cardNumber: 'H234567902', phoneNum: '555-2424', email: 'henry.foster@example.com', emergencyContactName: 'Jill Foster', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-2525' },
      { patientFirstName: 'Ivy', lastName: 'Garner', dateOfBirth: '1981-01-17', sex: 'Female', address: '2121 Willow St', cardNumber: 'H345678013', phoneNum: '555-2626', email: 'ivy.garner@example.com', emergencyContactName: 'Kevin Garner', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-2727' },
      { patientFirstName: 'Jack', lastName: 'Hughes', dateOfBirth: '1979-04-28', sex: 'Male', address: '2222 Aspen Ave', cardNumber: 'H456789024', phoneNum: '555-2828', email: 'jack.hughes@example.com', emergencyContactName: 'Laura Hughes', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-2929' },
      { patientFirstName: 'Kelly', lastName: 'Irwin', dateOfBirth: '1992-12-14', sex: 'Female', address: '2323 Poplar Rd', cardNumber: 'H567890135', phoneNum: '555-3030', email: 'kelly.irwin@example.com', emergencyContactName: 'Mark Irwin', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-3131' },
      { patientFirstName: 'Leo', lastName: 'Jacobs', dateOfBirth: '1985-06-26', sex: 'Male', address: '2424 Cherry Ln', cardNumber: 'H678901246', phoneNum: '555-3232', email: 'leo.jacobs@example.com', emergencyContactName: 'Nina Jacobs', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-3333' },
      { patientFirstName: 'Mia', lastName: 'Kennedy', dateOfBirth: '1976-10-18', sex: 'Female', address: '2525 Maple St', cardNumber: 'H789012357', phoneNum: '555-3434', email: 'mia.kennedy@example.com', emergencyContactName: 'Oscar Kennedy', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-3535' },
      { patientFirstName: 'Nick', lastName: 'Lee', dateOfBirth: '1987-05-09', sex: 'Male', address: '2626 Oak Ave', cardNumber: 'H890123468', phoneNum: '555-3636', email: 'nick.lee@example.com', emergencyContactName: 'Paula Lee', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-3737' },
      { patientFirstName: 'Olivia', lastName: 'Morris', dateOfBirth: '1993-03-31', sex: 'Female', address: '2727 Pine Rd', cardNumber: 'H901234579', phoneNum: '555-3838', email: 'olivia.morris@example.com', emergencyContactName: 'Quinn Morris', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-3939' },
      { patientFirstName: 'Aaron', lastName: 'Nelson', dateOfBirth: '1984-08-02', sex: 'Male', address: '2828 Birch Ln', cardNumber: 'H012345680', phoneNum: '555-4040', email: 'aaron.nelson@example.com', emergencyContactName: 'Rita Nelson', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-4141' },
      { patientFirstName: 'Beth', lastName: 'Owens', dateOfBirth: '1990-01-21', sex: 'Female', address: '2929 Cedar St', cardNumber: 'H123456792', phoneNum: '555-4242', email: 'beth.owens@example.com', emergencyContactName: 'Sam Owens', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-4343' },
      { patientFirstName: 'Carl', lastName: 'Parker', dateOfBirth: '1979-07-14', sex: 'Male', address: '3030 Elm Ave', cardNumber: 'H234567803', phoneNum: '555-4444', email: 'carl.parker@example.com', emergencyContactName: 'Tina Parker', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-4545' },
      { patientFirstName: 'Diana', lastName: 'Quinn', dateOfBirth: '1986-11-23', sex: 'Female', address: '3131 Spruce Rd', cardNumber: 'H345678014', phoneNum: '555-4646', email: 'diana.quinn@example.com', emergencyContactName: 'Umar Quinn', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-4747' },
      { patientFirstName: 'Ethan', lastName: 'Reed', dateOfBirth: '1992-05-05', sex: 'Male', address: '3232 Fir Ln', cardNumber: 'H456789035', phoneNum: '555-4848', email: 'ethan.reed@example.com', emergencyContactName: 'Vera Reed', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-4949' },
      { patientFirstName: 'Fiona', lastName: 'Santos', dateOfBirth: '1981-02-19', sex: 'Female', address: '3333 Willow St', cardNumber: 'H567890146', phoneNum: '555-5050', email: 'fiona.santos@example.com', emergencyContactName: 'Will Santos', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-5151' },
      { patientFirstName: 'George', lastName: 'Turner', dateOfBirth: '1975-09-30', sex: 'Male', address: '3434 Aspen Ave', cardNumber: 'H678901257', phoneNum: '555-5252', email: 'george.turner@example.com', emergencyContactName: 'Xena Turner', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-5353' },
      { patientFirstName: 'Hannah', lastName: 'Underwood', dateOfBirth: '1989-06-12', sex: 'Female', address: '3535 Poplar Rd', cardNumber: 'H789012368', phoneNum: '555-5454', email: 'hannah.underwood@example.com', emergencyContactName: 'Yusuf Underwood', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-5555' },
      { patientFirstName: 'Ian', lastName: 'Vasquez', dateOfBirth: '1994-12-27', sex: 'Male', address: '3636 Cherry Ln', cardNumber: 'H890123479', phoneNum: '555-5656', email: 'ian.vasquez@example.com', emergencyContactName: 'Zoe Vasquez', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-5757' },
      { patientFirstName: 'Jill', lastName: 'Walker', dateOfBirth: '1983-08-15', sex: 'Female', address: '3737 Maple St', cardNumber: 'H901234580', phoneNum: '555-5858', email: 'jill.walker@example.com', emergencyContactName: 'Alice Walker', emergencyContactRelationship: 'Mother', emergencyContactPhone: '555-5959' },
      { patientFirstName: 'Nina', lastName: 'Taylor', dateOfBirth: '1990-04-21', sex: 'Female', address: '4141 Elm Ave', cardNumber: 'H345678025', phoneNum: '555-6666', email: 'nina.taylor@example.com', emergencyContactName: 'Oscar Taylor', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-6767' },
      { patientFirstName: 'Oscar', lastName: 'Wilson', dateOfBirth: '1982-08-14', sex: 'Male', address: '4242 Spruce Rd', cardNumber: 'H456789036', phoneNum: '555-6868', email: 'oscar.wilson@example.com', emergencyContactName: 'Paula Wilson', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-6969' },
      { patientFirstName: 'Paula', lastName: 'Moore', dateOfBirth: '1978-12-05', sex: 'Female', address: '4343 Fir Ln', cardNumber: 'H567890147', phoneNum: '555-7070', email: 'paula.moore@example.com', emergencyContactName: 'Quinn Moore', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-7171' },
      { patientFirstName: 'Quinn', lastName: 'Davis', dateOfBirth: '1993-03-18', sex: 'Male', address: '4444 Willow St', cardNumber: 'H678901258', phoneNum: '555-7272', email: 'quinn.davis@example.com', emergencyContactName: 'Rita Davis', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-7373' },
      { patientFirstName: 'Rita', lastName: 'Evans', dateOfBirth: '1984-07-30', sex: 'Female', address: '4545 Aspen Ave', cardNumber: 'H789012369', phoneNum: '555-7474', email: 'rita.evans@example.com', emergencyContactName: 'Sam Evans', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-7575' },
      { patientFirstName: 'Sam', lastName: 'Foster', dateOfBirth: '1991-05-21', sex: 'Male', address: '4646 Poplar Rd', cardNumber: 'H890123480', phoneNum: '555-7676', email: 'sam.foster@example.com', emergencyContactName: 'Tina Foster', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-7777' },
      { patientFirstName: 'Tina', lastName: 'Garcia', dateOfBirth: '1987-11-11', sex: 'Female', address: '4747 Cherry Ln', cardNumber: 'H901234591', phoneNum: '555-7878', email: 'tina.garcia@example.com', emergencyContactName: 'Umar Garcia', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-7979' },
      { patientFirstName: 'Umar', lastName: 'Harris', dateOfBirth: '1980-02-27', sex: 'Male', address: '4848 Maple St', cardNumber: 'H012345692', phoneNum: '555-8080', email: 'umar.harris@example.com', emergencyContactName: 'Vera Harris', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-8181' },
      { patientFirstName: 'Vera', lastName: 'Irwin', dateOfBirth: '1995-06-14', sex: 'Female', address: '4949 Oak Ave', cardNumber: 'H123456704', phoneNum: '555-8282', email: 'vera.irwin@example.com', emergencyContactName: 'Will Irwin', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-8383' },
      { patientFirstName: 'Will', lastName: 'Jacobs', dateOfBirth: '1988-09-07', sex: 'Male', address: '5050 Pine Rd', cardNumber: 'H234567815', phoneNum: '555-8484', email: 'will.jacobs@example.com', emergencyContactName: 'Xena Jacobs', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-8585' },
      { patientFirstName: 'Xena', lastName: 'Kennedy', dateOfBirth: '1983-12-03', sex: 'Female', address: '5151 Birch Ln', cardNumber: 'H345678026', phoneNum: '555-8686', email: 'xena.kennedy@example.com', emergencyContactName: 'Yusuf Kennedy', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-8787' },
      { patientFirstName: 'Yusuf', lastName: 'Lee', dateOfBirth: '1992-04-18', sex: 'Male', address: '5252 Cedar St', cardNumber: 'H456789037', phoneNum: '555-8888', email: 'yusuf.lee@example.com', emergencyContactName: 'Zoe Lee', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-8989' },
      { patientFirstName: 'Zoe', lastName: 'Martinez', dateOfBirth: '1981-07-22', sex: 'Female', address: '5353 Elm Ave', cardNumber: 'H567890158', phoneNum: '555-9090', email: 'zoe.martinez@example.com', emergencyContactName: 'Aaron Martinez', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-9191' },
      { patientFirstName: 'Alice', lastName: 'Nelson', dateOfBirth: '1985-01-16', sex: 'Female', address: '5454 Spruce Rd', cardNumber: 'H678901269', phoneNum: '555-9292', email: 'alice.nelson@example.com', emergencyContactName: 'Beth Nelson', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-9393' },
      { patientFirstName: 'Bob', lastName: 'Owens', dateOfBirth: '1990-09-05', sex: 'Male', address: '5555 Fir Ln', cardNumber: 'H789012370', phoneNum: '555-9494', email: 'bob.owens@example.com', emergencyContactName: 'Carl Owens', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-9595' },
      { patientFirstName: 'Carol', lastName: 'Parker', dateOfBirth: '1986-03-11', sex: 'Female', address: '5656 Willow St', cardNumber: 'H890123481', phoneNum: '555-9696', email: 'carol.parker@example.com', emergencyContactName: 'Diana Parker', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-9797' },
      { patientFirstName: 'Dan', lastName: 'Quinn', dateOfBirth: '1982-10-29', sex: 'Male', address: '5757 Aspen Ave', cardNumber: 'H901234592', phoneNum: '555-9898', email: 'dan.quinn@example.com', emergencyContactName: 'Ethan Quinn', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-9999' },
      { patientFirstName: 'Eve', lastName: 'Reed', dateOfBirth: '1994-12-06', sex: 'Female', address: '5858 Poplar Rd', cardNumber: 'H012345693', phoneNum: '555-1001', email: 'eve.reed@example.com', emergencyContactName: 'Fiona Reed', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-1010' },
      { patientFirstName: 'Frank', lastName: 'Santos', dateOfBirth: '1977-04-23', sex: 'Male', address: '5959 Cherry Ln', cardNumber: 'H123456705', phoneNum: '555-1020', email: 'frank.santos@example.com', emergencyContactName: 'Grace Santos', emergencyContactRelationship: 'Wife', emergencyContactPhone: '555-1030' },
      { patientFirstName: 'Grace', lastName: 'Turner', dateOfBirth: '1989-08-11', sex: 'Female', address: '6060 Maple St', cardNumber: 'H234567826', phoneNum: '555-1040', email: 'grace.turner@example.com', emergencyContactName: 'Henry Turner', emergencyContactRelationship: 'Brother', emergencyContactPhone: '555-1050' },
      { patientFirstName: 'Henry', lastName: 'Underwood', dateOfBirth: '1993-05-15', sex: 'Male', address: '6161 Oak Ave', cardNumber: 'H345678037', phoneNum: '555-1060', email: 'henry.underwood@example.com', emergencyContactName: 'Ivy Underwood', emergencyContactRelationship: 'Sister', emergencyContactPhone: '555-1070' },
      { patientFirstName: 'Ivy', lastName: 'Vasquez', dateOfBirth: '1981-11-02', sex: 'Female', address: '6262 Pine Rd', cardNumber: 'H456789048', phoneNum: '555-1080', email: 'ivy.vasquez@example.com', emergencyContactName: 'Jack Vasquez', emergencyContactRelationship: 'Husband', emergencyContactPhone: '555-1090' }
    
    ];
    // Insert each patient
      testPatients.forEach(p => {
        Object.assign(this, p);
        this.createNewPatient();
      });
      this.successMessage = 'Inserted 15 test patients!';
    }
  },
  created() { // Fetches the patients 
    this.fetchPatients();
    let storedResults = localStorage.getItem('searchResults');
    let storedState = localStorage.getItem('searchState');
    if (storedResults && storedState) {
      this.filteredPatients = JSON.parse(storedResults);
      Object.assign(this, JSON.parse(storedState));
    }
  }
};
</script>

<style>
.container {
  overflow-x: hidden;
  overflow-y: hidden;
}
</style>
