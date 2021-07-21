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

//ignore options requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
	return 0;
}

// authentication
if (!authenticate_user()) {
    header('WWW-Authenticate: Basic realm="WebCam API"');
    header('HTTP/1.1 401 Unauthorized');
    exit;
}

// check that database exists
if (!is_file(CELLDATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}

$db = new SQLite3(CELLDATABASE);
if (is_null($db)) {
	header("HTTP/1.1 500 Internal Server Error");
        return;
}

$json_result = [];

// enough data for a network operator query?
if (array_key_exists("mcc", $_GET) && array_key_exists("mnc", $_GET)) {

	$query = $db->prepare("SELECT countryName,operator,brand FROM cell_networks WHERE mcc=? AND mnc=? LIMIT 1");
	$mnc = intval(trim($_GET["mnc"]));
	$mcc = intval(trim($_GET["mcc"]));
	$query->bindParam(1, $mcc, SQLITE3_INTEGER);
	$query->bindParam(2, $mnc, SQLITE3_INTEGER);
	$result = $query->execute();
	$row = $result->fetchArray(SQLITE3_ASSOC);
	if ($row) {
		$provider = [
			"country" => $row["countryName"],
			"operator" => $row["operator"],
			"brand" => $row["brand"]
		];
		$json_result["provider"] = $provider;
	}
}

// enough data for cell site query?
if (array_key_exists("mcc", $_GET) && array_key_exists("mnc", $_GET) && array_key_exists("cid", $_GET)) {

	$query = $db->prepare("SELECT latitude,longitude,description FROM cell_locations WHERE mcc=? AND mnc=? AND ci=? LIMIT 1");
	$mnc = intval(trim($_GET["mnc"]));
	$mcc = intval(trim($_GET["mcc"]));
	$cid = intval(trim($_GET["cid"]));
	$query->bindParam(1, $mcc, SQLITE3_INTEGER);
	$query->bindParam(2, $mnc, SQLITE3_INTEGER);
	$query->bindParam(3, $cid, SQLITE3_INTEGER);
	$result = $query->execute();
	$row = $result->fetchArray(SQLITE3_ASSOC);
	if ($row) {
		$site = [
			"description" => $row["description"],
			"latitude" => $row["latitude"],
			"longitude" => $row["longitude"]
		];
		$json_result["site"] = $site;
	}
}


header('Content-Type: application/json');
echo json_encode($json_result);
?>
