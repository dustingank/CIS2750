//import { DEFAULT_ENCODING } from "crypto";
var file_row = 0;
var event_row = 0;
var alarm_row = 0;

// Put all onload AJAX calls here, and event listeners
$(document).ready(function() {
    // On page-load AJAX Example
    let filenames = new Array();

    $.ajax({
        type: 'get',            //Request type
        url: '/file_panel',   //The server endpoint we are connecting to
        data: {feedback: "ok"},
        dataType: 'json',
        success: function (json_array) {
            var index = 0;
            if (json_array.length == 0) {
                //console.log("here?");
                let tanle = document.getElementById("fileInfo");
                let row = tanle.insertRow(-1);

                row.innerHTML = "<h5><strong>No File";
                let cell1 = row.insertCell(0);
                let cell2 = row.insertCell(1);
                let cell3 = row.insertCell(2);
                let cell4 = row.insertCell(3);

                cell1.innerHTML = "";
                cell2.innerHTML = "";
                cell3.innerHTML = "";
                cell4.innerHTML = "";

                //let oldtext = document.getElementById("status_message").innerHTML;
                document.getElementById("status_message").innerHTML = "No File Avaliable";
                alert("No File Are Available");

            } else {
                for (let a = 0; a < json_array.length; a++) {

                    if (json_array[a].Error_code != "None" ) {

                        let oldtext = document.getElementById("status_message").innerHTML;
                        document.getElementById("status_message").innerHTML = oldtext + "<br> Detected " + json_array[a].File_Name + " is invaild"
                            + "(" + json_array[a].Error_code + ")";

                    } else {
                        let table = document.getElementById("fileInfo");
                        let row = table.insertRow(-1);

                        let cell1 = row.insertCell(0);
                        let cell2 = row.insertCell(1);
                        let cell3 = row.insertCell(2);
                        let cell4 = row.insertCell(3);
                        let cell5 = row.insertCell(4);

                        let linked = document.createElement('a');
                        let linkText = document.createTextNode(json_array[a].File_Name);
                        linked.appendChild(linkText);
                        linked.title = json_array[a].File_Name;
                        linked.href = "/uploads/" + json_array[a].File_Name;

                        cell1.appendChild(linked);
                        cell2.innerHTML = json_array[a].version;
                        cell3.innerHTML = json_array[a].prodID;
                        cell4.innerHTML = json_array[a].numEvents;
                        cell5.innerHTML = json_array[a].numProps;
                        // code above is to add row to the file panel

                        let oldtext = document.getElementById("status_message").innerHTML;
                        document.getElementById("status_message").innerHTML = oldtext + "<br> Detected " + json_array[a].File_Name + " ben added";
                        
                        filenames[index] = json_array[a].File_Name;
                        index++;
                        addToDropDownList("all_file_list", json_array[a].File_Name);
                        addToDropDownList("Create_event_list", json_array[a].File_Name);

                    }
                }
                alert("All File Have Been Loaded");
            }
            //We write the object to the console to show that the request was successful
            //console.log(data);
        },
        fail: function(error) {
            // Non-200 return, do something with error
            console.log(error);
        }
    });

    document.getElementById('clear').onclick = function() {
        document.getElementById('status_message').innerHTML = " ";
    };

    document.getElementById('all_file_list').onchange = function(e) { // drop down button for view the events


        var name = document.getElementById("all_file_list");
        var choose_file_name = name.options[name.selectedIndex].value;
        //alert(choose_file_name);
        $.ajax({
            type: 'get',
            url: '/getEventJson',
            data: {'fileName': choose_file_name},
            dataType: 'json',

            success: function (Event_array) {
                var oldtext = document.getElementById("status_message").innerHTML;
                document.getElementById("status_message").innerHTML = oldtext + "<br> viewing " + choose_file_name;

                $('#CalendarInfo').find("tr:gt(0)").remove();
                document.getElementById("event_list").options.length = 1;
                /*
                var select = document.getElementById("event_list");
                var length = select.options.length;
                
                for (i = 1; i < length; i++) {
                    select.options[i] = null;
                }
                */

                for (var a = 0; a < Event_array.length; a++) {
                    var date = Event_array[a].startDT.date[0] + Event_array[a].startDT.date[1] + 
                               Event_array[a].startDT.date[2] + Event_array[a].startDT.date[3] +
                               "/" + Event_array[a].startDT.date[4] + Event_array[a].startDT.date[5] + "/" + 
                               Event_array[a].startDT.date[6] + Event_array[a].startDT.date[7];

                    var time = Event_array[a].startDT.time[0] + Event_array[a].startDT.time[1] + ":"
                               + Event_array[a].startDT.time[2] + Event_array[a].startDT.time[3] + ":"
                               + Event_array[a].startDT.time[4] + Event_array[a].startDT.time[5];

                    var table = document.getElementById("CalendarInfo");
                    var row = table.insertRow(-1);


                    let cell1 = row.insertCell(0);
                    let cell2 = row.insertCell(1);
                    let cell3 = row.insertCell(2);
                    let cell4 = row.insertCell(3);
                    let cell5 = row.insertCell(4);
                    let cell6 = row.insertCell(5);

                    cell1.innerHTML = a + 1;
                    cell2.innerHTML = date;
                    if (Event_array[a].startDT.isUTC == true ) {
                        cell3.innerHTML = time + ("(UTC)");
                    } else {
                        cell3.innerHTML = time;
                    }
                    cell4.innerHTML = Event_array[a].summary;
                    cell5.innerHTML = Event_array[a].numProps;
                    cell6.innerHTML = Event_array[a].numAlarms;

                    addToDropDownList('event_list', a + 1);
                }

                console.log(Event_array);
            },
            fail: function(error) {
                console.log(error);
            }
        });


    };

    document.getElementById('viewAlarm').onclick = function() {
        var name = document.getElementById("all_file_list");
        var choose_file_name = name.options[name.selectedIndex].value;
        var temp = document.getElementById("event_list");
        var choose_event = temp.options[temp.selectedIndex].value;

        var oldtext = document.getElementById("status_message").innerHTML;
        document.getElementById("status_message").innerHTML = oldtext + "<br> viewing " + choose_file_name + " Alarm";



        if (choose_file_name.length == 0 || choose_event.length == 0) {
            if (choose_file_name.length == 0) {
              alert("No File Been Choosen");
            } else if (choose_event.length == 0) {
              alert("No Event Been Choosen");
            }
        } else {
            $.ajax({
                type: 'get',
                url: '/getAlarmJson',
                data: {'fileName': choose_file_name, 'choose_event':choose_event},
                dataType: 'json',

                success: function (Event_array) {

                    if(Event_array.length == 0) {
                        document.getElementById("text_area").style.visibility = "visible";
                        document.getElementById("text_area").innerHTML = "This Event Do Not Have Alarm"
                    } else {
                        document.getElementById("text_area").style.visibility = "visible";(
                        document.getElementById)("text_area").innerHTML = "Alarm Information for " + choose_file_name + " Event No. " + choose_event +
                                                    ": ";
                        for (var index = 0; index < Event_array.length; index++) {

                            var oldtext = document.getElementById("text_area").innerHTML;
                            var temp_index = index + 1;
                            document.getElementById("text_area").innerHTML = oldtext + "<br>Alarm " + temp_index + ":<br>Action:" + Event_array[index].action + 
                                                                                "<br>Trigger: " + Event_array[index].trigger + "<br>Duration:" + Event_array[index].duration 
                                                                                + "<br>Repeat: " + Event_array[index].repeat + "<br>Attach: " + Event_array[index].attach;


                        }
                    }
                },
                fail: function(error) {
                    console.log(error);
                }
            });
        }
    };

    document.getElementById('view').onclick = function() {
        var name = document.getElementById("all_file_list");
        var choose_file_name = name.options[name.selectedIndex].value;
        var temp = document.getElementById("event_list");
        var choose_event = temp.options[temp.selectedIndex].value;

        var oldtext = document.getElementById("status_message").innerHTML;
        document.getElementById("status_message").innerHTML = oldtext + "<br> viewing " + choose_file_name + " Event Properties";

        if (choose_file_name.length == 0|| choose_event.length == 0) {
            if (choose_file_name.length == 0) {
              alert("No File Been Choosen");
            } else if (choose_event.length == 0) {
              alert("No Event Been Choosen");
            }
        } else {
            $.ajax({
                type: 'get',
                url: '/getOthers',
                data: {'fileName': choose_file_name, 'choose_event':choose_event},
                dataType: 'json',

                success: function (data) {
                    //console.log("here\n");
                    document.getElementById("text_area").style.visibility = "visible";(
                    document.getElementById)("text_area").innerHTML = "Event Properties Information for " + choose_file_name + " Event No. " + choose_event +
                                                    ": <br>" + "Organizer: " + data.organizer + "<br>class: " + data.class + "<br>DT_END: " + data.dtEnd;
                },
                fail: function(error) {
                    console.log(error);
                }
            });
        }
    };

    document.getElementById('create_calendar').onclick = function() {
       
        var new_file_name = document.getElementById('file_name').value;
        var version = document.getElementById('version_number').value;
        var product_id = document.getElementById('product_id').value;
        var event_uid = document.getElementById('event_id').value;
        var event_stamp = document.getElementById('start_stamp').value;
        var event_stemp_time = document.getElementById('start_stamp_time').value;
        var Stamp_UTC = document.getElementById('stamp_UTC').value;
        var event_start = document.getElementById('start_date').value;
        var event_start_time = document.getElementById('start_time').value;
        var Start_UTC = document.getElementById('start_UTC').value;
        
        if (new_file_name == "" || version == "" || product_id == "" || event_uid == "" || event_stamp == "" || event_start == "" ||
                event_stemp_time == "" || event_start_time == "") {
            updata_status("Failed to Create: Empty value recieved while adding calendar");
            alert("Field Can't Be Empty!");
            return;
        }

        for (var i = 0; i < filenames.length; i++) {
            if (filenames[i] == new_file_name) {
                updata_status("Failed to Create: File already exists")
                alert("File Already exists");
                return
            }
        }


        var temp = new_file_name;
        var file_extention = temp.split('.').pop();

        if (file_extention != 'ics') {
            updata_status("Failed to Created: File exention is not .ics or no file exention");
            alert('File exention is not .ics or no file exention');
            return;
        }
        
        if (check_UTC(Stamp_UTC) != 1) {
            updata_status("Failed to Create: UTC can only be True or False");
            alert("UTC can only be True or False");
            return;
        }

        if (check_UTC(Start_UTC) != 1) {
            updata_status("Failed to Create: UTC can only be True or False");
            alert("UTC can only be True or False");
            return;
        }
         
        $.ajax({
            type: 'get',
            url: '/createFile',
            data: {'file_name':new_file_name,
            'version':version,
            'Product_ID': product_id,
            'event_uid':event_uid,
            'event_stamp':event_stamp,
            'event_stamp_time':event_stemp_time,
            'event_start':event_start, 
            'event_start_time':event_start_time,
            'stamp_UTC':Stamp_UTC,
            'start_UTC':Start_UTC
            },
            dataType: 'json',

            success:function(data) {
                if (data.Error_code == "OK") {
                    alert("New File Been Added, Refresh the page please...");
                    $('#file_name').val("");
                    $('#version_number').val("");
                    $('#product_id').val("");
                    $('#stamp_UTC').val("");
                    $('#start_UTC').val("");
                    $('#event_id').val("");
                    $('#start_stamp').val("");
                    $('#start_stamp_time').val("");
                    $('#start_date').val("");
                    $('#start_time').val("");
                } else {
                    alert("Error, check Status for more infomation")
                    var oldtext = document.getElementById("status_message").innerHTML;
                    document.getElementById("status_message").innerHTML = oldtext + '<br> Unable to upload the file Due to: ' + data.Error_code;
                }
            },
            fail: function(error) {
                alert("False!\n");
            },     
        });
        
    };

    document.getElementById('make_event_button').onclick = function() {
        var name = document.getElementById("Create_event_list");
        var choose_file_name = name.options[name.selectedIndex].value;
        if (choose_file_name.length == 0) {
            alert('No File Been Choosen!');
            return;
        }

        var event_uid = document.getElementById('Create_event_UID').value;
        var event_creation_date = document.getElementById('Create_Creation_startDT').value;
        var event_creation_time = document.getElementById('Create_Creation_startTime').value;
        var creation_UTC = document.getElementById('Stamp_UTC').value;
        var event_start_date = document.getElementById('Create_event_startDT').value;
        var event_start_time = document.getElementById('Create_event_startTime').value;
        var start_UTC = document.getElementById('Start_UTC').value;
        var event_summary = document.getElementById('Create_event_summary').value;
    
        if (event_uid == "" || event_creation_date == "" || event_creation_time == "" || event_start_date == "" || event_start_time == "" || event_summary == ""
            || creation_UTC == "" || start_UTC == "") {
            updata_status("Failed to Create: Empty value recieved while adding Event");
            alert("Field Can't Be Empty!");
            return;
        }

        if (check_UTC(creation_UTC) != 1) {
            updata_status("Failed to Create: UTC can only be True or False");
            alert("UTC can only be True or False");
            return;
        }

        if (check_UTC(start_UTC) != 1) {
            updata_status("Failed to Create: UTC can only be True or False");
            alert("UTC can only be True or False");
            return;
        }

        $.ajax({
            type: 'get',
            url: '/addEvent',
            data: {'file_name': choose_file_name,
                   'uid': event_uid,
                   'd_stamp':event_creation_date,
                   't_stamp':event_creation_time,
                   'stamp_utc':creation_UTC,
                   'd_start':event_start_date,
                   't_start':event_start_time,
                   'start_utc':start_UTC,
                   'summary':event_summary
                },
            dataType: 'json',

            success:function(data) {
                if (data.Error_code == "OK") {
                    alert("New Event Been Added, Refresh the page to updates the file log...");
                    $('#Create_event_UID').val("");
                    $('#Create_Creation_startDT').val("");
                    $('#Create_Creation_startTime').val("");
                    $('#Stamp_UTC').val("");
                    $('#Create_event_startDT').val("");
                    $('#Create_event_startTime').val("");
                    $('#Start_UTC').val("");
                    $('#Create_event_summary').val("");
                    var oldtext = document.getElementById("status_message").innerHTML;
                    document.getElementById("status_message").innerHTML = oldtext + "<br> A Event been added to " + choose_file_name;
                } else {
                    alert("Error, check Status for more infomation")
                    var oldtext = document.getElementById("status_message").innerHTML;
                    document.getElementById("status_message").innerHTML = oldtext + '<br> Unable to upload the file Due to: ' + data.Error_code;
                }
            },
            fail: function(error) {

            },
        });
    };

    document.getElementById('Store_All_Files').onclick = function() {
            $.ajax({
                type: 'get',            //Request type
                url: '/store_all_files',   //The server endpoint we are connecting to
                data: {filenames: filenames},

                success: function(data) {
                    alert("All files have been loaded into Database");
                },
                fail:function(error) {
                    console.log(error);
                } 
            });
    };

    document.getElementById('clear_all_data').onclick = function() {
        $.ajax({
            type: 'get',
            url: '/delete_all_tables',   
            
            success: function(data) {
                alert("All information have been deleted form three tables");
            },
            fail:function(error) {
                console.log(error);
            }
        });
    };

    document.getElementById('login_buttom').onclick = function() {
        var use_name = document.getElementById('Usename').value;
        var password = document.getElementById('Password').value;
        var data_base = document.getElementById('Database').value;
        document.getElementById("Store_All_Files").disabled = true;
        document.getElementById("clear_all_data").disabled = true;
        document.getElementById("display_db_status").disabled = true;
        document.getElementById('execute_query').disabled = true;

        //if (use_name)

        //look back at it
        $.ajax({
            type: 'get',
            url: '/connect_dataBase',
            data: {'userNmae': use_name,
                   'password': password,
                   'data_base':data_base
                   },
            dataType: 'json',

            success:function(data) {
                if (data.status == 'pass') {
                    alert('database have been connect')
                    updata_status("database have been connect");
                    document.getElementById("Store_All_Files").disabled = false;
                    document.getElementById("clear_all_data").disabled = false;
                    document.getElementById("display_db_status").disabled = false;
                    document.getElementById('execute_query').disabled = false;
                    document.getElementById("database_view").style.display = "block";
                } else if (data.status == "error") {
                    alert('Unable to connect database: check username or password');
                    updata_status("Unable to connect database: check username or password");
                }
                
            },
            fail:function(error) {
                console.log(error);
            }

        });
    };

    document.getElementById('display_db_status').onclick = function() {
        $.ajax({
            type: 'get',
            url: '/display_db_status_file',
            dataType: 'json',

            success:function(data) {
                if (data.status != "error") {
                    file_row = data.status;
                    get_event_row(file_row);
                }
            },
            fail:function(error) {
                console.log(error);
            }
        });
    };

    document.getElementById('execute_query').onclick = function() {
        var file_index = -1;
        var options = document.getElementById('options').value;
        var file_name = document.getElementById('file_name').value;

        if (check_options(options) == 0) {
            alert("error input!");
            return;
        }

        for (var index = 0; index < filenames.length; index++) {
            if (file_name == filenames[index])  {
                file_index = index + 1;
            }
        }

        if (file_index == -1) {
            alert("Uable to find the file");
            return;
        }

       // console.log(options);

            $.ajax({
                type: 'get',
                url: '/execute_query',
                dataType: 'json',
                data: {'option':options,'file_name':file_name, 'index':file_index},
    
                success:function(data) {
                    console.log(options);
                    if (options == 1) {
                        document.getElementById("database_panel").style.display = "block";
                        document.getElementById("database").innerHTML =  "<th>Event ID</th><th>Summary</th><th>Start Time</th><th>Loction</th><th>Organizer</th> <th>Cal_id</th>";
                        for (var index = 0; index < data.length - 1; index++) {
                            var table = document.getElementById("database");
                            var row = table.insertRow(-1);

                            var cell1 = row.insertCell(0);
                            var cell2 = row.insertCell(1);
                            var cell3 = row.insertCell(2);
                            var cell4 = row.insertCell(3);
                            var cell5 = row.insertCell(4);
                            var cell6 = row.insertCell(5);

                            cell1.innerHTML = data[index].event_id;
                            cell2.innerHTML = data[index].summary;
                            cell3.innerHTML = data[index].start_time;
                            cell4.innerHTML = data[index].location;
                            cell5.innerHTML = data[index].organizer;
                            cell6.innerHTML = data[index].cal_file;
                        }
                    } else if (options == 2) {
                        document.getElementById("database_panel").style.display = "block";
                        document.getElementById("database").innerHTML =  "<th>Summary</th><th>Start Time</th>";
                        for (var index = 0; index < data.length - 1; index++) {
                            var table = document.getElementById("database");
                            var row = table.insertRow(-1);

                            var cell1 = row.insertCell(0);
                            var cell2 = row.insertCell(1);
                        
                            cell1.innerHTML = data[index].summary;
                            cell2.innerHTML = data[index].start_time;
                            
                        }
                    } else if (options ==  3) {
                        document.getElementById("database_panel").style.display = "block";
                        document.getElementById("database").innerHTML =  "<th>Event ID</th><th>Summary</th><th>Start Time</th><th>Loction</th><th>Organizer</th> <th>Cal_id</th>";
                        for (var index = 0; index < data.length - 1; index++) {
                            var table = document.getElementById("database");
                            var row = table.insertRow(-1);

                            var cell1 = row.insertCell(0);
                            var cell2 = row.insertCell(1);
                            var cell3 = row.insertCell(2);
                            var cell4 = row.insertCell(3);
                            var cell5 = row.insertCell(4);
                            var cell6 = row.insertCell(5);

                            cell1.innerHTML = data[index].event_id;
                            cell2.innerHTML = data[index].summary;
                            cell3.innerHTML = data[index].start_time;
                            cell4.innerHTML = data[index].location;
                            cell5.innerHTML = data[index].organizer;
                            cell6.innerHTML = data[index].cal_file;
                        }


                    }
                },
                fail:function(error) {
                    console.log(error);
                }
            });
    };


});

function addToDropDownList (elememt_id,file_name) {
    let calendar_drop_down_list = document.getElementById(elememt_id);
    let option_calendar = document.createElement("Option");
    option_calendar.text = file_name;
    calendar_drop_down_list.add(option_calendar);

}

function updata_status(newtext) {
    var oldtext = document.getElementById("status_message").innerHTML;
    document.getElementById("status_message").innerHTML = oldtext + '<br>' + newtext;
}

function check_UTC (text) {
    if (text == 'True' || text == 'False') {
        return 1;
    }

    return 0;
} 

function get_event_row(row1) {
    $.ajax({
        type: 'get',
        url: '/display_db_status_event',
        dataType: 'json',

        success:function(data) {
            if (data.status != "error") {
                event_row = data.status;
                get_alarm_row(row1, event_row);
            }
        },
        fail:function(error) {
            console.log(error);
        }
    });
}

function get_alarm_row(row1, row2) {
    $.ajax({
        type: 'get',
        url: '/display_db_status_alarm',
        dataType: 'json',

        success:function(data) {
            if (data.status != "error") {
                alarm_row = data.status;
                print_result(row1, row2, alarm_row);
            }
        },
        fail:function(error) {
            console.log(error);
        }
    });
}

function print_result(number1, number2, number3) {
    var toBePrint = "Database has " + number1 + " files, " + number2 + " events,and " + number3 + "alarms"
    alert(toBePrint);
    updata_status(toBePrint);
}

function check_options (number) {
    if (number == 1) {
        return 1;
    } else if (number == 2) {
        return 1;
    } else if (number == 3) {
        return 1;
    } else if (number == 4) {
        return 1;
    } else if (number == 5) {
        return 1;
    } else if (number == 6) {
        return 1;
    }
    return 0;
}



