var Character = require('../models/characters');

exports.getRelated = function (req, res) {
    var userData = req.body;
    if (userData == null) {
        res.status(403).send('No data sent!')
    }
    try {

        if (userData.name.length > 0 && userData.cuisine.length > 0 && userData.postcode.length > 0)
            {
            Character.find({name: userData.name, cuisine: userData.cuisine, postcode: userData.postcode},
                'name cuisine postcode address description',
                function (err, characters) {
                    if (err)
                        res.status(500).send('Invalid data!');
                    var character = null;
                    if (characters.length > 0) {
                        var firstElem = characters[0];
                        character = {
                            name: firstElem.name,
                            cuisine: firstElem.cuisine,
                            postcode: firstElem.postcode,
                            address: firstElem.address,
                            description: firstElem.description
                        };
                    }
                    res.setHeader('Content-Type', 'application/json');
                    res.send(JSON.stringify(character));  //so now it can only send one name, to be modified to supp more
                });
            }
            else
            {
                if (userData.name.length > 0)
                {
                    Character.find({name: userData.name},
                        'name cuisine postcode address description',
                        function (err, characters) {
                            if (err)
                                res.status(500).send('Invalid data!');
                            var character = null;
                            if (characters.length > 0) {
                                var firstElem = characters[0];
                                character = {
                                    name: firstElem.name,
                                    cuisine: firstElem.cuisine,
                                    postcode: firstElem.postcode,
                                    address: firstElem.address,
                                    description: firstElem.description
                                };
                            }
                            res.setHeader('Content-Type', 'application/json');
                            res.send(JSON.stringify(character));  //so now it can only send one name, to be modified to supp more
                        });
                }
            }
        }
    catch
        (e)
        {
            res.status(500).send('error ' + e);
        }


}



exports.getRestaurant = function (req, res) {
    var Data = req.body;
    if (Data == null) {
        res.status(403).send('No data sent!')
    }
    try {
        Character.find({name: Data},
            'name cuisine postcode address description reviews rating rate_count',
            function (err, characters) {
                if (err)
                    res.status(500).send('Invalid data!');
                var character = null;
                if (characters.length > 0) {
                    var firstElem = characters[0];
                    character = {
                        name: firstElem.name,
                        cuisine: firstElem.cuisine,
                        postcode: firstElem.postcode,
                        address: firstElem.address,
                        description: firstElem.description,
                        reviews:firstElem.reviews,
                        rating:firstElem.rating,
                        rate_count:firstElem.rate_count
                    };
                }
                res.setHeader('Content-Type', 'application/json');
                res.send(JSON.stringify(character));  //so now it can only send one name, to be modified to supp more
            });
    } catch (e) {
        res.status(500).send('error ' + e);
    }
}

//    Character.find()
 //       .then(function(characters)
 //       {
 //           res.setHeader('Content-Type', 'application/json');
 //           res.send(JSON.stringify(character));
 //       })
 //       .catch(function(err)
 //       {res.status(500).send('error '+ err);});


exports.insert = function (req, res) {

    var userData = req.body;
    if (userData == null) {
        res.status(403).send('No data sent!')
    }
    try {
        var restaurant = new Character({
            name: userData.name,
            cuisine: userData.cuisine,
            postcode: userData.postcode,
            address: userData.address,
            description:userData.description,
            pos_longitude: userData.pos_longitude,
            pos_latitude: userData.pos_latitude,
            rating:3,
            rate_count:0
        });

        restaurant.save(function (err, results) {
            if (err)
                res.status(500).send('Invalid data!');
            console.log(results._id);

//to render to /list
            Character.find()
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
    } catch (e) {
        res.status(500).send('error ' + e);
    }
}


exports.updateRate = function (req, res) {
    var userData = req.body;
    if (userData == null) {
        res.status(403).send('No data sent!')
    }
    try {
        Character.find({_id: userData.id},
            'reviews rating rate_count',
            function (err, characters) {
                if (err)
                    res.status(500).send('Invalid data!');
                var character = null;
                if (characters.length > 0) {
                    var firstElem = characters[0];
                    character = {
                        reviews:firstElem.reviews,
                        rating:firstElem.rating,
                        rate_count:firstElem.rate_count
                    };
                }
                character.reviews=character.reviews+' '+userData.reviews;
                character.rate_count+=1;
                character.rating=( (character.rating*character.rate_count+parseInt(userData.rate))/(character.rate_count+1) ).toPrecision(3);

                Character.update({'_id':userData.id},{$set:{
                    'reviews':character.reviews,
                    'rating':character.rating,
                    'rate_count':character.rate_count
                    } })
                    .then(function(characters)
                               {console.log(userData);

                                   Character.find({'_id':userData.id},
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
                                           res.render('restaurant',{obj:character});
                                       });

                               })
                    .catch(function(err)
                               {res.status(500).send('error '+ err);});


            });

    } catch (e) {
        res.status(500).send('error ' + e);
    }
}