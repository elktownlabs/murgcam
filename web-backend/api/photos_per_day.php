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

// ignore options requests
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

$authenticated = murgcam_authenticate($appdb, $data["token"]);
if (!$authenticated) {
	header('HTTP/1.0 401 Unauthorized');
        $appdb->close();
	return;
}


// check that arguments are correct
if (!array_key_exists("date",  $data)) {
	header("HTTP/1.1 404 Not Found");
	return;
}

// check that database exists
if (!is_file(DATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}
$db = new SQLite3(DATABASE);
if (is_null($db)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}


// calculate timestamps
$diff1Day = new DateInterval('P1D');
$startDate = new DateTime($data["date"]);
$startDate->setTime(0, 0, 0, 0);
$endDate = clone $startDate;
$endDate->add($diff1Day);

// find num of photos for each day
$query = $db->prepare("SELECT * FROM photos WHERE Timestamp>=? AND Timestamp<? ORDER BY Timestamp ASC");
$startTimestamp = $startDate->getTimestamp();
$endTimestamp = $endDate->getTimestamp();
$query->bindParam(1, $startTimestamp, SQLITE3_INTEGER);
$query->bindParam(2, $endTimestamp, SQLITE3_INTEGER);
$result = $query->execute();
$queryresult = [];
while($row = $result->fetchArray(SQLITE3_ASSOC)) {
	$filename = $row["filename"];
	$row["id"] = substr($filename, 4, 13); 
	$meta = $row["meta"];
	unset($row["meta"]);
	$row["telemetry"] = json_decode($meta, true);
	$queryresult[] = $row;
}

header('Content-Type: application/json');
echo json_encode($queryresult);
?>
