import { createRouter, createWebHistory } from 'vue-router';

import Dashboard from '../views/Dashboard.vue';
import SearchPatient from '../views/SearchPatient.vue';
import Patients from '../views/PatientsSearch.vue';
import PatientProfile from '../views/PatientProfile.vue'
import Login from '../views/LoginView.vue';
import Register from '../views/DoctorRegisterView.vue';
import AddHID from '../views/AddHIDView.vue'

const routes = [
    { path: '/', component: Dashboard },
    { path: '/search-patient', component: SearchPatient },
    { path: '/patients', component: Patients },
    { path: '/login', component: Login },
    { path: '/register', component: Register },
    { path: '/patients/:patient', name : 'PatientProfile', component: PatientProfile},
    { path: '/patients/add-treatment/:patient', name: 'AddTreatment', component: AddHID },
];

const router = createRouter({
    history: createWebHistory(),
    routes,
});

export default router;