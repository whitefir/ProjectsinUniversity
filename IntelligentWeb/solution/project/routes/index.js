var express = require('express');
var router = express.Router();
var bodyParser= require("body-parser");
var character = require('../controllers/characters');
var user = require('../controllers/users');
var initDB= require('../controllers/init');
var mCharacter = require('../models/characters');

initDB.init();


/* GET home page. */


router.get('/index', function(req, res, next) {
    mCharacter.find()
        .then(function(characters)
        {
            //give views an object with information
            // res.render('list', {obj:JSON.stringify(characters)});
            var obj={};
            if (characters.length > 0) {
                for (index in characters) {
                    var element = characters[index];
                    obj[index] = {
                        name: element.name,
                        longitude: element.pos_longitude,
                        latitude: element.pos_latitude
                    };

                }
                res.render('index', {obj: obj});
            }
        })
        .catch(function(err)
        {
            console.log("fail");
            res.status(500).send('error '+ err);
        });
});

router.post('/index', character.getRelated);

router.get('/list', function(req, res, next) {
    //ask db all information

    mCharacter.find()
        .then(function(characters)
        {
            //give views an object with information
           // res.render('list', {obj:JSON.stringify(characters)});
            var obj={};
            if (characters.length > 0) {
                for (index in characters) {
                    var element = characters[index];
                    obj[index] = {
                        name: element.name,
                        cuisine: element.cuisine,
                        postcode: element.postcode,
                        address: element.address,
                        description: element.description
                    };
                }
                res.render('list', {obj: obj});
            }
        })
        .catch(function(err)
        {
            console.log("fail");
            res.status(500).send('error '+ err);});
});


router.get('/restaurant', function(req, res, next) {
    var Data = req.query.name;
    mCharacter.find({name: Data},
        '_id name cuisine postcode address description reviews rating pos_longitude pos_latitude',
        function (err, characters) {
            if (err)
                res.status(500).send('Invalid data!');
            var character = null;
            if (characters.length > 0) {
                var firstElem = characters[0];
                character = {
                    id:firstElem._id,
                    name: firstElem.name,
                    cuisine: firstElem.cuisine,
                    postcode: firstElem.postcode,
                    address: firstElem.address,
                    description: firstElem.description,
                    reviews: firstElem.reviews,
                    rating: firstElem.rating,
                    longitude: firstElem.pos_longitude,
                    latitude: firstElem.pos_latitude
                };
            }
            console.log(character);
            res.render('restaurant',{obj:character});
        });
});

router.post('/restaurant', character.updateRate);

router.get('/login', function(req, res, next) {
    res.render('login');
});
router.post('/login', user.login);

router.get('/register', function(req, res, next) {
    res.render('register');
});

router.post('/register', user.register);

router.get('/insert', function(req, res, next) {
    res.render('insert');
});
router.post('/insert', character.insert);

module.exports = router;
