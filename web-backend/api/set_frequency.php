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
$query = $appdb->prepare("SELECT s.expiration as expiration, u.rights as rights, u.id as user_id FROM active_logins s, users u WHERE u.id=s.user_id AND token=? LIMIT 1;");
$query->bindParam(1, $data["token"], SQLITE3_TEXT);
$resultset = $query->execute();
$userid = null;
$authenticated = false;
while($row = $resultset->fetchArray(SQLITE3_ASSOC)) {
    $rights = array_map('trim', explode(",", $row["rights"]));
	if (($row["expiration"] >= time()) && (in_array("freq", $rights))) $authenticated = true;
    $userid = $row["user_id"];
}
$resultset->finalize();
if (!$authenticated) {
	header('HTTP/1.0 401 Unauthorized');
    $appdb->close();
	return;
}

// sanity check
if (!array_key_exists("mode_identifier", $data)) {
    header('HTTP/1.0 400 Bad Request');
    $appdb->close();
    return;
}
if (($data["mode_identifier"] < 0) || ($data["mode_identifier"] > 6)) {
    header('HTTP/1.0 400 Bad Request');
    $appdb->close();
    return;
}

// decode mode
$new_mode = null;
$end_time = null;
switch ($data["mode_identifier"]) {
case 0:
    $new_mode = 0;
    break;
case 1:
    // fast mode, 3h
    $new_mode = 1;
    $end_time = time() + 60*60*3;
    break;
case 2:
    // fast mode, 6h
    $new_mode = 1;
    $end_time = time() + 60*60*6;
    break;
case 3:
    // fast mode, end of day
    $new_mode = 1;
    $end_time  = mktime(23, 59, 59, date("m"), date("d"), date("Y"));
    break;
case 4:
    // express mode, 1h
    $new_mode = 2;
    $end_time = time() + 60*60*1;
    break;
case 5:
    // express mode, 2h
    $new_mode = 2;
    $end_time = time() + 60*60*2;
    break;
case 6:
    // express mode, 3h
    $new_mode = 2;
    $end_time = time() + 60*60*3;
    break;
}

// write mode change to db
$query = $appdb->prepare("UPDATE cam_frequency SET requested_mode=?, requested_mode_end=?, requested_mode_initiating_time=?, requested_mode_initiating_user=? WHERE cam_id=1");
$query->bindParam(1, $new_mode, SQLITE3_INTEGER);
$query->bindParam(2, $end_time, SQLITE3_INTEGER);
$query->bindParam(3, time(), SQLITE3_INTEGER);
$query->bindParam(4, $userid, SQLITE3_TEXT);
$result = $query->execute();
if ($result === false) {
	header("HTTP/1.1 500 Internal Server Error");
    $appdb->close();
    return;
}

// get updated settings
$result = $appdb->querySingle("SELECT current_mode, current_mode_start, current_mode_end, current_mode_initiating_user, current_mode_initiating_time, requested_mode, requested_mode_end, requested_mode_initiating_time, requested_mode_initiating_user FROM cam_frequency WHERE cam_id = 1", true);
if ($result === false) {
    header("HTTP/1.1 500 Internal Server Error");
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
