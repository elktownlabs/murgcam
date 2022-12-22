<?php
require_once("config.php");
require_once("common.php");

$gauges = ["Schwarzenberg", "Schönmünzach", "Bad Rotenfels"];


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


// check that database exists
if (!is_file(APPDATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}
if (!is_file(GAUGEDATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}
$appdb = new SQLite3(APPDATABASE);
	if (is_null($appdb)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}
$db = new SQLite3(GAUGEDATABASE);
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

// sanity checks
if (!array_key_exists("timestamp", $data)) {
    header('HTTP/1.0 404 Not Found');
    $db->close();
	return;
}


$json_result = [];

// enough data for a network operator query?
$timestamp = $data["timestamp"];
if (!is_numeric($timestamp)) {
	header("HTTP/1.1 404 Not Found");
		$db->close();
		return;
}
$timestamp = intval($timestamp);
$gauge_results = [];
foreach ($gauges as &$gauge) {
	$query=$db->prepare("SELECT gauge_id, timestamp, flow FROM gauge_data WHERE gauge_id=? ORDER BY ABS(?-timestamp) ASC LIMIT 1");
	$query->bindParam(1, $gauge);
	$query->bindParam(2, $timestamp);
	$result = $query->execute();
	$row = $result->fetchArray(SQLITE3_ASSOC);
	if ($row) {
		$timediff = abs($timestamp - $row["timestamp"]);
		// readout should be at most half an hour away
		if ($timediff <= 30*60) {
			$data = [
				"timestamp" => $row["timestamp"],
				"timediff" => $timestamp - $row["timestamp"],
				"flow" => $row["flow"]
			];
			$gauge_results[$gauge] = $data;
		}
	}
}
$json_result["flows"] = $gauge_results;

$db->close();

// calc derived values
$derived_flows = [];
if (array_key_exists("Schwarzenberg", $gauge_results) && array_key_exists("Schönmünzach", $gauge_results)) {
	$derived_flows["Mittlere - 20"] = $gauge_results["Schwarzenberg"]["flow"] + $gauge_results["Schönmünzach"]["flow"] - 20.0;
	$derived_flows["Mittlere - 15"] = $gauge_results["Schwarzenberg"]["flow"] + $gauge_results["Schönmünzach"]["flow"] - 15.0;
	$derived_flows["Mittlere - 10"] = $gauge_results["Schwarzenberg"]["flow"] + $gauge_results["Schönmünzach"]["flow"] - 10.0;
	$derived_flows["Mittlere - 5"] = $gauge_results["Schwarzenberg"]["flow"] + $gauge_results["Schönmünzach"]["flow"] - 5.0;
	$derived_flows["Mittlere - No discharge"] = $gauge_results["Schwarzenberg"]["flow"] + $gauge_results["Schönmünzach"]["flow"];
	foreach ($derived_flows as $key => $value) {
		if ($value < 0.0) $derived_flows[$key] = 0.0;
	}
}
$json_result["derived_flows"] = $derived_flows;


header('Content-Type: application/json');
echo json_encode($json_result);
?>
