'use strict'

let sql_separater = "',";
let quotation = "'";
// C library API
const ffi = require('ffi');
const mysql = require('mysql');

var file_name;
// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

let file_array = new Array();

app.use(fileUpload());
// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change

const portNum = process.argv[2];
/*
const connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : 'wang15',
    password : '1013411',
    database : 'wang15'
});
connection.connect(function(err) {
  if (err) res.send(err);
  console.log("done");
});
*/

let connection;
app.get('/connect_dataBase', function(req, res){
  //console.log('Trying to login...');
  //console.log('user name = '+ req.query.userNmae);
  //console.log('pass = '+req.query.password);
  //console.log('database = '+ req.query.data_base);
  connection = mysql.createConnection({
    host     : 'dursley.socs.uoguelph.ca',
    user     : req.query.userNmae,
    password : req.query.password,
    database : req.query.data_base
  });
  connection.connect(function(err, result) {
    if (err) {
      console.log(err);
    } else {
      res.send('{"status":"pass"}').status(200);
    }
  });
}); 

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    console.log(err);
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

//console.log("test 1");

//Sample endpoint
let sharedLib = ffi.Library('./library.so', {
  'glue_code_getFileInfo': ['string', ['string']],
  'glue_code_getEventInfo':['string', ['string']],
  'glue_code_getAlarmInfo':['string',['string', 'int']],
  'glue_code_getOtherInfo':['string', ['string', 'int']],
  'make_calendar_file':['string', ['string','string', 'string', 'string', 'string','string', 'string','string','string','string']],
  'add_event_toFile':['string',['string', 'string', 'string', 'string', 'string', 'string', 'string', 'string', 'string']]
});


//console.log("test 2");

function parserFiles() {
  let result_arr = []; 
  let result;
  let file_director = "./uploads/";
  //console.log("readirsync before");
  let files = fs.readdirSync(file_director);
  //console.log("readirsync after");
  if(files == undefined || files == null) return null;
  if(files.length == 0) return null;

  files.forEach(file => {
    let result_JSON = sharedLib.glue_code_getFileInfo(file);
    //console.log('inside the foreach');
    //INV_FILE, INV_CAL, INV_VER, DUP_VER, INV_PRODID, DUP_PRODID, INV_EVENT, INV_DT, INV_ALARM

    //console.log(result_JSON);

    let parserData = JSON.parse(result_JSON);
   // console.log(result_JSON);

    if (parserData.Error_code != undefined) {
      result = {"File_Name": file, "Error_code":parserData.Error_code}
    } else {
      //let parserData = JSON.parse(result_JSON);
      result = { "File_Name": file, "version": parserData.version, "prodID": parserData.prodID,
        "numProps":parserData.numProps,
        "numEvents":parserData.numEvents,
        "Error_code":"None",
      }
      //result_arr.push(result);
    }
    //console.log(result_JSON);
    result_arr.push(result);
    //console.log(result_arr.length);
  });
  return result_arr;
}

function make_insert_to_FILE_table_command(string, file_name) {
  //var condition = "IF NOT EXISTS() \nBEGIN\n"; 
  //console.log(string);
  var toBeInsert = "INSERT IGNORE INTO FILE (file_name, version, prod_id) VALUES " + string;

  let check_file_query = "SELECT file_name FROM FILE WHERE file_name = " + "'"+ file_name + "'";
  
  connection.query(check_file_query, function(err, result, fields) {
    if (err) throw err;
    if (result.length > 0) {
      console.log("File already in table");
    } else {
      insert_to_table(toBeInsert);
      var command_line = make_insert_Event_command_line(file_name)

      connection.query(command_line,function(err,result){
        if (err) {
          console.log(err);
        } else {
          //console.log("FILE id: " + result[0].cal_id);
          make_insert_to_EVENT_table(file_name, result[0].cal_id);
        }
      });
    }
  });

}

function make_insert_to_EVENT_table(file_name, value) {
  var temp = sharedLib.glue_code_getEventInfo(file_name);
  var obj = JSON.parse(temp);
  

  for (var index = 0; index < obj.length; index++) {
    if (obj[index].summary == '') {
      var summary = 'NULL';
    } else {
      summary = obj[index].summary;
    }
    if (obj[index].location == '') {
      var location = 'NULL';
    } else {
      location = obj[index].location;
    }
    if (obj[index].organizer == '') {
      var organizer = 'NULL';
    } else {
      organizer = obj[index].organizer;
    }

    var year = obj[index].startDT.date[0] + obj[index].startDT.date[1] + obj[index].startDT.date[2] + obj[index].startDT.date[3];
    var month = obj[index].startDT.date[4] + obj[index].startDT.date[5];
    var date = obj[index].startDT.date[6] + obj[index].startDT.date[7];
    var hour = obj[index].startDT.time[0] + obj[index].startDT.time[1];
    var Minutes = obj[index].startDT.time[2] + obj[index].startDT.time[3];
    var seconds = obj[index].startDT.time[4] + obj[index].startDT.time[5];
    var date = year + '-' + month + '-' + date + ' ' + hour + ':' + Minutes + ':' + seconds;
    

    //var date_time = obj[index].startDT.date + obj[index].startDT.time + obj[index].startDT.isUTC;


    var toBeInsert = "INSERT INTO EVENT (summary, start_time, location, organizer, cal_file) VALUES " + 
    "('" + summary + sql_separater + quotation + date + sql_separater
    + quotation + location + sql_separater + quotation + organizer + sql_separater + quotation + value + "')";

    var event_number = index + 1;
    console.log("Event: " + toBeInsert);
    connection.query(toBeInsert,function(err){
      if (err) {
        throw err;
      }
    });

  }
}


function  insert_to_table(comand_line_items) {
  console.log("insert to File table: " + comand_line_items);
  connection.query(comand_line_items, function (err, result) {
    if (err) throw err;
  });

} 

function ready_to_uploads(filenames) {

  for (var index = 0; index < filenames.length; index++) {
    var file_info_json = sharedLib.glue_code_getFileInfo(filenames[index]);
    var file_info = JSON.parse(file_info_json);
    var cal_id_value = index + 1;
  
    var temp_file = "('" + filenames[index] + sql_separater + quotation + file_info.version + sql_separater
            + quotation + file_info.prodID + "')";
     
    console.log("Read to uploads: " + temp_file);
    
    make_insert_to_FILE_table_command(temp_file, filenames[index], cal_id_value);
    //make_insert_to_EVENT_table(filenames[index], cal_id_value);
  }
}

function ready_to_uploads_alarm(filenames) {
  for (var index = 0; index < filenames.length; index++) {
    var sql_command = "SELECT cal_id FROM FILE WHERE file_Name = '" + filenames[index] + "'"
  }
}

function delete_all() {
  let delete_command_file = "DELETE FROM FILE;";
  let delete_command_event = "DELETE FROM EVENT";
  let delete_command_alarm = "DELETE FROM ALARM";
  let reset_file = "ALTER TABLE FILE AUTO_INCREMENT=1";
  let reset_event = "ALTER TABLE EVENT AUTO_INCREMENT=1";
  let reset_alarm = "ALTER TABLE ALARM AUTO_INCREMENT=1";

  connection.query(delete_command_file, function (err, result) {
    if (err) throw err;
    console.log("file table information have been delete");
  });
  connection.query(reset_file, function (err, result) {
    if (err) throw err;
  });

  connection.query(delete_command_event, function (err, result) {
    if (err) throw err;
    console.log("event table information have been delete");
  });
  connection.query(reset_event, function (err, result) {
    if (err) throw err;
  });

  connection.query(delete_command_alarm, function (err, result) {
    if (err) throw err;
    console.log("alarm table information have been delete");
  });
  connection.query(reset_alarm, function (err, result) {
    if (err) throw err;
  });

}

function make_insert_Event_command_line(file) {
  return "SELECT cal_id FROM FILE WHERE file_name = '" + file + "'";
}

function make_json(event_id, summary, start_time, location, organizer, cal_file) {
  var toBeReturn = "{" +
                      '"event_id":"' + event_id + '",' +
                      '"summary":"' + summary + '",'  +
                      '"start_time":"' + start_time + '",' +
                      '"location":"' + location + '",' +
                      '"organizer":"' + organizer + '",' +
                      '"cal_file":"' + cal_file + '"' +
                   "},";
  return toBeReturn;
}







//console.log("test 3");

//parserFiles();
//Sample endpoint

// this following code are dealing with the SQL
// declear all three table
let sql_file_table = 
    "CREATE TABLE FILE (cal_id INT AUTO_INCREMENT, file_Name VARCHAR(60) NOT NULL, version INT NOT NULL, prod_id VARCHAR(256) NOT NULL, PRIMARY KEY(cal_id))";
//make_table(sql_file_table);

let sql_event_table = 
    "CREATE TABLE EVENT (event_id INT AUTO_INCREMENT, summary VARCHAR(1025), start_time DATETIME NOT NULL, location VARCHAR(60), organizer VARCHAR(256), cal_file INT NOT NULL, PRIMARY KEY(event_id),FOREIGN KEY(cal_file) REFERENCES FILE(cal_id) ON DELETE CASCADE)";
//make_table(sql_event_table);

let sql_alarm_table = 
    "CREATE TABLE ALARM (alarm_id INT AUTO_INCREMENT, action VARCHAR(256) NOT NULL, `trigger` VARCHAR(256) NOT NULL, event INT NOT NULL, PRIMARY KEY(alarm_id), FOREIGN KEY(event) REFERENCES EVENT(event_id) ON DELETE CASCADE)";
//make_table(sql_alarm_table);


app.get('/file_panel', function(req , res){
  //console.log("/file_panel is called");
  let arrayJsonString = "[]";
  try{
    arrayJsonString = parserFiles();
  }catch(err){
    arrayJsonString = '[]'
  }
  //console.log("done");
  if(arrayJsonString == null) arrayJsonString = '[]';
  //console.log(arrayJsonString);
  res.send(arrayJsonString);
});


app.get('/getEventJson', function(req, res){
  //console.log(sharedLib.glue_code_getEventInfo(req.query.fileName));
  res.send(sharedLib.glue_code_getEventInfo(req.query.fileName)).status(200);
});

app.get('/getAlarmJson', function(req, res){
  //console.log(sharedLib.glue_code_getAlarmInfo(req.query.fileName, req.query.choose_event));
  res.send(sharedLib.glue_code_getAlarmInfo(req.query.fileName, req.query.choose_event)).status(200);
});

app.get('/getOthers', function(req, res){
  var json_temp = (sharedLib.glue_code_getOtherInfo(req.query.fileName, req.query.choose_event));
  res.send(json_temp).status(200);
}); 

app.get('/createFile', function(req, res) {
  res.send(sharedLib.make_calendar_file(req.query.file_name, req.query.version, req.query.Product_ID, req.query.event_uid, req.query.event_stamp, 
            req.query.event_stamp_time, req.query.event_start, req.query.event_start_time, req.query.stamp_UTC, req.query.start_UTC));
});

app.get('/addEvent', function(req,res) {
  res.send(sharedLib.add_event_toFile(req.query.file_name, req.query.uid, req.query.d_stamp, req.query.t_stamp,
                            req.query.stamp_utc, req.query.d_start, req.query.t_start, req.query.start_utc, req.query.summary));
});

app.get('/store_all_files', function(req,res){
  file_array = req.query.filenames;
  ready_to_uploads(req.query.filenames);
  ready_to_uploads_alarm(req.query.filenames);
  res.send("{'status':'sccuess'}").status(200);
});

app.get('/delete_all_tables', function(req,res){
  delete_all();
  res.send("{'status':'sccuess'}").status(200);
});

//pure water/ close to me/ be alright
app.get('/display_db_status_file', function(req, res){
  var sql = "SELECT count(*) as total FROM FILE";
  var query = connection.query(sql, function(err, result) {
    if (err) res.send('{"status":"error"}');
    var tobeReturn = result[0].total;
    res.send('{"status":"'+ tobeReturn + '"}').status(200);
  });
});

app.get('/display_db_status_event', function(req, res){
  var sql = "SELECT count(*) as total FROM EVENT";
  var query = connection.query(sql, function(err, result){
    if (err) res.send('{"status":"error"}');
    var tobeReturn = result[0].total;
    res.send('{"status":"'+ tobeReturn + '"}').status(200);
  });
});

app.get('/display_db_status_alarm', function(req,res){
  var sql = "SELECT count(*) as total FROM ALARM";
  var query = connection.query(sql, function(err, result){
    if (err) res.send('{"status":"error"}');
    var tobeReturn = result[0].total;
    res.send('{"status":"'+ tobeReturn + '"}').status(200);
  });
});

app.get('/execute_query', function(req, res){
  
  if (req.query.option == 1) {
    var sql_command = "SELECT * FROM EVENT ORDER BY start_time";
  } else if (req.query.option == 2) {
    var sql_command = "SELECT * FROM EVENT WHERE cal_file = " + req.query.index;  
  } else if (req.query.option == 3) {
    var sql_command = "SELECT * FROM EVENT EVENT WHERE start_time IN (SELECT start_time FROM EVENT GROUP BY start_time HAVING COUNT(*) >= 2)";
  }
  
  
    console.log(sql_command);

    connection.query(sql_command, function (err, rows, fields) {  // get the rows fomr the event table
      var string = '[';
      //Throw an error if we cannot run the query
        if (err) {
            console.log(err);
        } else {
            //Rows is an array of objects.  Each object has fields corresponding to table columns
            for (let row of rows){
              var temp_string = make_json(row.event_id, row.summary,row.start_time, row.location, row.organizer, row.cal_file);
              string = string + temp_string;
            }
            string = string + '{"query_id":"' + req.query.option + '"}]';
            console.log(string);
            res.send(string).status(200);
        }
      });
});

app.listen(portNum);


