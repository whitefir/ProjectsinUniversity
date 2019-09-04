var mongoose = require('mongoose');

var Schema = mongoose.Schema;

var User = new Schema(
    {
        username: {type: String, required: true, max: 100},
        password: {type: String, required: true, max: 100}
    }
);

User.set('toObject', {getters: true, virtuals: true});

module.exports = mongoose.model('User', User);