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
$authenticated = murgcam_authenticate($appdb, $data["token"]);
if (!$authenticated) {
	header('HTTP/1.0 401 Unauthorized');
        $appdb->close();
	return;
}


// do we want to encode in base64
$do_base64 = false;
if (array_key_exists("base64", $data)) {
	$do_base64 = $data["base64"];
}

if ($do_base64) {
	// base64 encoding also includes meta-data.

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

	// check that database exists
	if (!is_file(CELLDATABASE)) {
		header("HTTP/1.1 500 Internal Server Error");
		return;
	}

	$celldb = new SQLite3(CELLDATABASE);
	if (is_null($db)) {
		header("HTTP/1.1 500 Internal Server Error");
		return;
	}
}


if (!array_key_exists("id", $data)) {
    header('HTTP/1.0 404 Not Found');
    exit;
}

$id = trim($data["id"]);
$filename = "cam_" . $id . ".jpg";

$photo = file_get_contents(PHOTODIR."/".$filename);

if ($photo === false) {
    header('HTTP/1.0 404 Not Found');
    exit;
}

if ($do_base64) {
	$data = [];

	// fetch telemetry data
	$query = $db->prepare("SELECT * FROM photos WHERE filename = ?");
	$query->bindParam(1, $filename, SQLITE3_TEXT);
	$result = $query->execute();
	while($row = $result->fetchArray(SQLITE3_ASSOC)) {
		$data["telemetry"] = json_decode($row["meta"], true);
		// fix bug in firmware
		if (array_key_exists("firmare", $data["telemetry"])) {
			$data["telemetry"]["firmware"] = $data["telemetry"]["firmare"];
			unset($data["telemetry"]["firmare"]);
		}
	}

	// fetch cell network data
	$query = $celldb->prepare("SELECT countryName,operator,brand FROM cell_networks WHERE mcc=? AND mnc=? LIMIT 1");
	$mnc = intval(trim($data["telemetry"]["gsm_mnc"]));
	$mcc = intval(trim($data["telemetry"]["gsm_mcc"]));
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
		$data["cell_provider"] = $provider;
	}

	// fetch cell location data
	$query = $celldb->prepare("SELECT latitude,longitude,description FROM cell_locations WHERE mcc=? AND mnc=? AND ci=? LIMIT 1");
	$query->bindParam(1, $data["telemetry"]["gsm_mcc"], SQLITE3_INTEGER);
	$query->bindParam(2, $data["telemetry"]["gsm_mnc"], SQLITE3_INTEGER);
	$query->bindParam(3, $data["telemetry"]["gsm_cellid"], SQLITE3_INTEGER);
	$result = $query->execute();
	$row = $result->fetchArray(SQLITE3_ASSOC);
	if ($row) {
		$site = [
			"description" => $row["description"],
			"latitude" => $row["latitude"],
			"longitude" => $row["longitude"]
		];
		$data["cell_site"] = $site;
	} else {
		$site = [
			"description" => $row["Unknown"],
			"latitude" => 0.00,
			"longitude" => 0.00
		];
		$data["cell_site"] = $site;
	}

	// add photo
	$data["photo"] = "data:image/jpg;base64," . base64_encode($photo);
	header('Content-Type: application/json');
	echo json_encode($data);
} else {
	header("Content-Type: image/jpeg");
	echo $photo;
}

?>
