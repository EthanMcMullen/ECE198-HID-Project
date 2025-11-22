const mongoose = require("mongoose");
const Schema = mongoose.Schema;

const patientDataSchema = new Schema({
    patientId: {
        type: Schema.Types.ObjectId,
        ref: "Patient",
        required: true
    },
    avgHeartRate: {
        type: Number,
        required: true
    },
    motionPercent: {
        type: Number,
        required: true
    },
    timestamp: {
        type: Date,
        default: Date.now,
        index: true
    }
});

patientDataSchema.index(
    { timestamp: 1 },
    { expireAfterSeconds: 24 * 60 * 60 } // Delete overflow data after 24 hours
);

module.exports = mongoose.model("PatientData", patientDataSchema);
