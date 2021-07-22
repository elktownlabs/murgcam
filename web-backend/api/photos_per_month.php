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
if (!array_key_exists("year",  $_GET)) {
	header("HTTP/1.1 404 Not Found");
	return;
}
if (!array_key_exists("month",  $_GET)) {
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
$startDate->setDate($_GET["year"], $_GET["month"], 1);
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
