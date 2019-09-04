var mongoose = require('mongoose');

var Schema = mongoose.Schema;

var Character = new Schema(
    {
        name: {type: String, required: true, max: 100},
        cuisine: {type: String, required: true, max: 100},
        postcode: {type: String, required: true, max: 100},
        address:{type: String, required: true, max: 100},
        description: {type: String, required: true},
        pos_longitude:{type: Number, required: true},
        pos_latitude:{type: Number, required: true},
        reviews:{type: String},
        rating:{type: Number, required: true},
        rate_count:{type: Number, required:true}
    }
);

Character.set('toObject', {getters: true, virtuals: true});

module.exports = mongoose.model('Character', Character);