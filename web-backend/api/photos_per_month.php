<?php

require("config.php");

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
$query = $appdb->prepare("SELECT expiration FROM active_logins WHERE token=? LIMIT 1");
$query->bindParam(1, $data["token"], SQLITE3_TEXT);
$resultset = $query->execute();
$authenticated = false;
while($row = $resultset->fetchArray(SQLITE3_ASSOC)) {
	if ($row["expiration"] >= time()) $authenticated = true;
}
$resultset->finalize();
$appdb->close();
if (!$authenticated) {
	header('HTTP/1.0 401 Unauthorized');
	return;
}

// check that arguments are correct
if (!array_key_exists("year",  $data)) {
	header("HTTP/1.1 404 Not Found");
	return;
}
if (!array_key_exists("month",  $data)) {
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
$diff1Month = new DateInterval('P1M');
$diff1Day = new DateInterval('P1D');
$startDate = new DateTime();
$startDate->setDate($data["year"], $data["month"], 1);
$startDate->setTime(0, 0, 0, 0);
$endDate = clone $startDate;
$endDate->add($diff1Month);

// find num of photos for each day
$query = $db->prepare("SELECT COUNT(*) FROM photos WHERE Timestamp>=? AND Timestamp<?");
$i=$startDate;
$queryresult = array();
while ($i<$endDate) {
	$j = clone $i;
	$query->bindParam(1, $i->getTimestamp(), SQLITE3_INTEGER);
	$i->add($diff1Day);
	$query->bindParam(2, $i->getTimestamp(), SQLITE3_INTEGER);
	$result = $query->execute();
	if ($result->fetchArray()[0] > 0) $queryresult[] = $j->format(DateTime::ATOM);
}

header('Content-Type: application/json');
echo json_encode($queryresult);
?>
