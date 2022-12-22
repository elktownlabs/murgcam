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
if (!is_file(DATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}
$appdb = new SQLite3(APPDATABASE);
if (is_null($appdb)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}
$db = new SQLite3(DATABASE);
if (is_null($db)) {
    $appdb->close();
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

$authenticated = murgcam_authenticate($appdb, $data["token"]);
if (!$authenticated) {
	header('HTTP/1.0 401 Unauthorized');
        $appdb->close();
	return;
}

if (!is_numeric($data["duration"])) {
    header('HTTP/1.0 404 Not Found');
    $db->close();
    $appdb->close();
	return;
}

// calculate relevant time stamp
$timestamp = time() - intval($data["duration"]);

// check for photos
$query = $db->prepare("SELECT COUNT(*) FROM photos WHERE timestamp >= ?");
$query->bindParam(1, $timestamp, SQLITE3_INTEGER);
$result = $query->execute();
if ($result !== false) {
    $result_row = $result->fetchArray();
    if ($result_row !== false) {
        $json_result = [ "available" => $result_row[0] ];
        print(json_encode($json_result));
    } else {
        header('HTTP/1.0 500 Internal Server Error');
    }
} else {
    header('HTTP/1.0 500 Internal Server Error');
}

$appdb->close();
$db->close();
?>
