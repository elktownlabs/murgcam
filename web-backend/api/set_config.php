<?php
require_once("config.php");
require_once("common.php");

if (CORS) {
        header('Access-Control-Allow-Origin: *');
}
header("Content-Type: application/json; charset=UTF-8");
header("Access-Control-Allow-Methods: GET, OPTIONS");
header("Access-Control-Max-Age: 3600");
header("Access-Control-Allow-Headers: Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With, origin");

// ignore post requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
           return 0;
}


// check that database exists
if (!is_file(APPDATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}
$appdb = new SQLite3(APPDATABASE);
if (is_null($appdb)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}


// extract supplied user and password from json post
$data = json_decode(file_get_contents('php://input'), true);
// authentication
if (!array_key_exists("token", $data)) {
        header('HTTP/1.0 401 Unauthorized');
        return;
}

$authenticated = murgcam_authenticate($appdb, $data["token"], "set");
if (!$authenticated) {
	header('HTTP/1.0 401 Unauthorized');
        $appdb->close();
	return;
}


// get existing config
$currentcontent = $appdb->querySingle("SELECT active_config, active_timestamp, modified_config, modified_timestamp, server_config, server_timestamp FROM cam_config WHERE cam_id = 1", true);
if ($currentcontent === false) {
        header("HTTP/1.1 404 Not Found");
        $appdb->close();
        return;
}


// handle server config
if (array_key_exists("server", $data)) {
        // get existing config
        $server_config = json_decode($currentcontent["server_config"], true);
        if ($server_config == null) $server_config = [];
        foreach ($data["server"] as $key => $value) {
                if (!is_object($value) && !is_array($value)) {
                        $server_config[$key] = $value;
                }
        }
}

// handle modified camera config
if (array_key_exists("cam", $data)) {
        // get existing config
        $cam_config = json_decode($currentcontent["modified_config"], true);
        if ($cam_config == null) $cam_config = [];
	foreach ($data["cam"] as $key => $value) {
		if ($key == "sys_secs_between_photos") { $value = intval($value); }
		if ($key == "sys_minimum_voltage") { $value = intval($value); }
                if (!is_object($value) && !is_array($value)) {
                        $cam_config[$key] = $value;
                }
        }
}

// write changes to database
$currentTimestamp=time();
$query = $appdb->prepare("UPDATE cam_config SET  modified_config=?, modified_timestamp=?, server_config=?, server_timestamp=? WHERE cam_id=1");
$query->bindParam(1, json_encode($cam_config), SQLITE3_TEXT);
$query->bindParam(2, $currentTimestamp, SQLITE3_INTEGER);
$query->bindParam(3, json_encode($server_config), SQLITE3_TEXT);
$query->bindParam(4, $currentTimestamp, SQLITE3_INTEGER);
$result = $query->execute();
$appdb->close();
if ($result === false) {
	header("HTTP/1.1 500 Internal Server Error");
        return;
}


// return new values for gui
$returnvalue = [];
$returnvalue["active"] = json_decode($currentcontent["active_config"]);
$returnvalue["active_timestamp"] = $currentcontent["active_timestamp"];
$returnvalue["modified"] = $cam_config;
$returnvalue["modified_timestamp"] = $currentTimestamp;
$returnvalue["server"] = $server_config;
$returnvalue["server_timestamp"] = $currentTimestamp;
echo json_encode($returnvalue);

?>
