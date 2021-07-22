<?php
/* Copyright (C) Elktown Labs. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Tobias Frodl <toby@elktown-labs.com>, 2021
 */

require_once("config.php");
require_once("helpers.php");

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

// authentication
if (!authenticate_user()) {
    header('WWW-Authenticate: Basic realm="WebCam API"');
    header('HTTP/1.1 401 Unauthorized');
    exit;
}

// check that arguments are correct
if (!array_key_exists("date",  $_GET)) {
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
$startDate = new DateTime($_GET["date"]);
$startDate->setTime(0, 0, 0, 0);
$endDate = clone $startDate;
$endDate->add($diff1Day);

// find num of photos for each day
$query = $db->prepare("SELECT * FROM photos WHERE Timestamp>=? AND Timestamp<? ORDER BY Timestamp ASC");
$query->bindParam(1, $startDate->getTimestamp(), SQLITE3_INTEGER);
$query->bindParam(2, $endDate->getTimestamp(), SQLITE3_INTEGER);
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
