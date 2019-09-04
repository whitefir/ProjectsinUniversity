var express = require('express');

function sendAjaxQuery(url, data) {
    $.ajax({
        url: url ,
        data: data,
        dataType: 'json',
        type: 'POST',
        success: function (result) {

            document.getElementById('result').innerHTML=result;
            if(result=='password matched')
            {$('#result_success').show();}


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
    sendAjaxQuery('/login', data);
    event.preventDefault();

}