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

$result = $appdb->querySingle("SELECT active_config, active_timestamp, modified_config, modified_timestamp, server_config, server_timestamp FROM cam_config WHERE cam_id = 1", true);
if ($result === false) {
        header("HTTP/1.1 404 Not Found");
        $appdb->close();
        return;
}

$returnvalue = [];
$returnvalue["active"] = json_decode($result["active_config"]);
$returnvalue["active_timestamp"] = $result["active_timestamp"];
$returnvalue["modified"] = json_decode($result["modified_config"]);
$returnvalue["modified_timestamp"] = $result["modified_timestamp"];
$returnvalue["server"] = json_decode($result["server_config"]);
$returnvalue["server_timestamp"] = $result["server_timestamp"];



$appdb->close();
echo json_encode($returnvalue);
?>
