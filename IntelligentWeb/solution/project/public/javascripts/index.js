var express = require('express');

function sendAjaxQuery(url, data) {
    $.ajax({
        url: url ,
        data: data,
        dataType: 'json',
        type: 'POST',
        success: function (dataR) {
            // no need to JSON parse the result, as we are using
            // dataType:json, so JQuery knows it and unpacks the
            // object for us before returning it

            var obj= dataR;
            //var obj = JSON.parse(dataR);
            // in order to have the object printed by alert
            // we need to JSON stringify the object

            document.getElementById('result_name').innerHTML='name: '+obj.name;
                //"<a href=restaurant?name="+obj.name+">name: "+obj.name+"</a>";

            document.getElementById('result_cuisine').innerHTML= 'cuisine: '+obj.cuisine;
            document.getElementById('result_postcode').innerHTML= 'postcode: '+obj.postcode;
            document.getElementById('result_address').innerHTML= 'address: '+obj.address;
            document.getElementById('result_description').innerHTML= 'description: '+obj.description;
        },
        error: function (xhr, status, error) {
            alert('Error: ' + error.message);
        }
    });
}

function onSubmit() {
    var formArray= $("form").serializeArray();
    var data={};
    for (index in formArray){
        data[formArray[index].name]= formArray[index].value;
    }
    // const data = JSON.stringify($(this).serializeArray());
    sendAjaxQuery('/index', data);
    event.preventDefault();
}

function myMap() {

//map
    myCenter=new google.maps.LatLng(53.3816229,-1.4842791);
    var mapOptions= {
        center:myCenter,
        zoom:16, scrollwheel: true, draggable: true,
        mapTypeId:google.maps.MapTypeId.ROADMAP
    };
    var map=new google.maps.Map(document.getElementById("googleMap"),mapOptions);

//centre
    var curr_pos = {
        lat: 53.3816229,
        lng: -1.4842791
    };

    // Try HTML5 geolocation.
    if (navigator.geolocation)
    {
        navigator.geolocation.getCurrentPosition(function(position)
        {
            curr_pos = {
                lat: position.coords.latitude,
                lng: position.coords.longitude
            };
            var marker = new google.maps.Marker({position: curr_pos,
                                                 map: map,
                                                 title:"Current Location"});

            map.setCenter(curr_pos);

//range
            var meter500 = new google.maps.Circle({
                strokeColor: '#FF0000',
                strokeOpacity: 0.3,
                strokeWeight: 2,
                fillColor: '#FF0000',
                fillOpacity: 0.35,
                map: map,
                center: curr_pos,
                radius: 500
            });

        }, function() {
            handleLocationError(true, infoWindow, map.getCenter());
        });
    }
    else
    {
        // Browser doesn't support Geolocation
        handleLocationError(false, infoWindow, map.getCenter());
    }

//marker

    var obj={};
    obj=JSON.parse($("#object").html());

    for (index in obj) {
        var LatLng = new google.maps.LatLng(obj[index].latitude, obj[index].longitude);
        var info = new google.maps.InfoWindow;
        info.setPosition(LatLng);
        info.setContent(obj[index].name);
        info.open(map);

        //var marker = new google.maps.Marker({position: LatLng,
          //                                   map: map,
            //                                 title:obj[index].name});
                }

}

function handleLocationError(browserHasGeolocation, infoWindow, pos) {
    infoWindow.setPosition(pos);
    infoWindow.setContent(browserHasGeolocation ?
        'Error: The Geolocation service failed.' :
        'Error: Your browser doesn\'t support geolocation.');
    infoWindow.open(map);
}
