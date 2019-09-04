var User = require('../models/users');
var Character = require('../models/characters');

exports.register = function (req, res) {

    var userData = req.body;
    if (userData == null) {
        res.status(403).send('No data sent!')
    }
    try {
        var user = new User({
            username: userData.username,
            password: userData.password
        });

        user.save(function (err, results) {
            if (err)
                res.status(500).send('Invalid data!');

            console.log(results);

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
    } catch (e) {
        res.status(500).send('error ' + e);
    }
}


exports.login = function (req, res) {

    var userData = req.body;
    if (userData == null) {
        res.status(403).send('No data sent!')
    }
    try {
        console.log(userData);
        User.find({username: userData.username},
            'username password',
            function (err, result) {
                if (err)
                    res.status(500).send('Invalid data!');

                console.log(result);

                if (result.length > 0) {
                    if(result[0].password==userData.password)
                    {
                        console.log('123');
                        var result='password matched';
                        res.setHeader('Content-Type', 'application/json');
                        res.send(JSON.stringify(result));
                    }
                    else
                    {
                        var result='password wrong';
                        res.setHeader('Content-Type', 'application/json');
                        res.send(JSON.stringify(result));
                    }
                }
                else
                {
                    var result='username invalid';
                    res.setHeader('Content-Type', 'application/json');
                    res.send(JSON.stringify(result));
                }

            });
    } catch (e) {
        res.status(500).send('error ' + e);
    }
}
