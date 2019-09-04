var mongoose = require('mongoose');
var Character = require('../models/characters');
var User = require('../models/users');


exports.init= function() {
    // uncomment if you need to drop the database

    Character.remove({}, function(err) {
        console.log('restaurant collection removed')
     });

    var character = new Character({
        name: 'Yep Yep Hot Pot',
        cuisine: 'Hot Pot',
        postcode: 'S3 7HE',
        address:'198 Brook Hill',
        description:'this is Yep Yep Hot Pot',
        reviews:' ',
        pos_longitude:-1.482808,
        pos_latitude:53.382084,
        rating:3,
        rate_count:1
    });
    character.save(function (err, results) {
        console.log(results._id);
    });

    var character = new Character({
        name: 'Orient Express',
        cuisine: 'Chinese',
        postcode: 'S10 2HS',
        address:'290 Glossop Rd',
        description:'this is Orient Express',
        reviews:' ',
        pos_longitude:-1.482322,
        pos_latitude:53.380230,
        rating:3,
        rate_count:2
    });
    character.save(function (err, results) {
        console.log(results._id);
    });

    var character = new Character({
        name: 'Efes Restaurant',
        cuisine: 'Turkish',
        postcode: 'S10 2HS',
        address:'278-280 Glossop Rd',
        description:'this is Efes Restaurant',
        reviews:' ',
        pos_longitude:-1.481756,
        pos_latitude:53.380179,
        rating:5,
        rate_count:3
    });
    character.save(function (err, results) {
        console.log(results._id);
    });

    var character = new Character({
        name: 'Twisted Burger Company, The Harley',
        cuisine: 'Fast food',
        postcode: 'S10 2HW',
        address:'334 Glossop Rd',
        description:'this is Twisted Burger Company, The Harley',
        reviews:' ',
        pos_longitude:-1.484881,
        pos_latitude:53.380383,
        rating:1,
        rate_count:4
    });
    character.save(function (err, results) {
        console.log(results._id);
    });

    var character = new Character({
        name: 'Maveli',
        cuisine: 'Indian',
        postcode: 'S10 2GW',
        address:'223 Glossop Rd',
        description:'this is Maveli',
        reviews:' ',
        pos_longitude:-1.481995,
        pos_latitude:53.380025,
        rating:3,
        rate_count:5
    });
    character.save(function (err, results) {
        console.log(results._id);
    });

    var character = new Character({
        name: 'Sundaes Gelato',
        cuisine: 'Dessert',
        postcode: 'S10 2HS',
        address:'262 Glossop Rd',
        description:'this is Sundaes Gelato',
        reviews:' ',
        pos_longitude:-1.480883,
        pos_latitude:53.380159,
        rating:3,
        rate_count:6
    });
    character.save(function (err, results) {
        console.log(results._id);
    });


    User.remove({}, function(err) {
        console.log('user collection removed')
    });


    var user = new User({
        username: 'qqq',
        password: '123'
    });

    user.save(function (err, results) {
        console.log(results._id);
    });

}

