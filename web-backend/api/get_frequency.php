<?php

require("config.php");

function unix2iso($input)
{
    if ($input === null) return null;
    $datetime = new DateTime('@'.$input);
    $datetime->setTimezone(new DateTimeZone('Europe/Berlin'));
    return $datetime->format(DateTimeInterface::ISO8601);
}

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
$query = $appdb->prepare("SELECT s.expiration as expiration, u.rights as rights FROM active_logins s, users u WHERE u.id=s.user_id AND token=? LIMIT 1;");
$query->bindParam(1, $data["token"], SQLITE3_TEXT);
$resultset = $query->execute();
$authenticated = false;
while($row = $resultset->fetchArray(SQLITE3_ASSOC)) {
    $rights = array_map('trim', explode(",", $row["rights"]));
	if (($row["expiration"] >= time()) && (in_array("freq", $rights))) $authenticated = true;
}
$resultset->finalize();
if (!$authenticated) {
	header('HTTP/1.0 401 Unauthorized');
        $appdb->close();
	return;
}


$result = $appdb->querySingle("SELECT current_mode, current_mode_start, current_mode_end, current_mode_initiating_user, current_mode_initiating_time, requested_mode, requested_mode_end, requested_mode_initiating_time, requested_mode_initiating_user FROM cam_frequency WHERE cam_id = 1", true);
if ($result === false) {
        header("HTTP/1.1 404 Not Found");
        $appdb->close();
        return;
}


$returnvalue = [];
$returnvalue["current"] = [];
$returnvalue["requested"] = [];

$returnvalue["current"]["mode"] = $result["current_mode"];
$returnvalue["current"]["start"] = unix2iso($result["current_mode_start"]);
$returnvalue["current"]["end"] = unix2iso($result["current_mode_end"]);
$returnvalue["current"]["initiating_user"] = $result["current_mode_initiating_user"];
$returnvalue["current"]["initiating_time"] = unix2iso($result["current_mode_initiating_time"]);
if (!(($result["requested_mode"] == $result["current_mode"]) && ($result["current_mode"] == 0))) {
    $returnvalue["requested"]["mode"] = $result["requested_mode"];
    $returnvalue["requested"]["mode_end"] = unix2iso($result["requested_mode_end"]);
    $returnvalue["requested"]["mode_initiating_time"] = unix2iso($result["requested_mode_initiating_time"]);
    $returnvalue["requested"]["mode_initiating_user"] = $result["requested_mode_initiating_user"];
} else {
    $returnvalue["requested"]  = null;
}

$appdb->close();
echo json_encode($returnvalue);
?>
