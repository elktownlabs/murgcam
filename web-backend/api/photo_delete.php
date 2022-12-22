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

$authenticated = murgcam_authenticate($appdb, $data["token"], "del");
if (!$authenticated) {
	header('HTTP/1.0 401 Unauthorized');
        $appdb->close();
	return;
}


if (!array_key_exists("id", $data)) {
    header('HTTP/1.0 404 Not Found');
    $db->close();
    $appdb->close();
	return;
}

// generate file name from id
if (preg_match("/^([a-f0-9])+$/", $data["id"])) {
    $file_to_delete = "cam_".$data["id"].".jpg";
    $path_to_delete = PHOTODIR . "/" . $file_to_delete;

    if (file_exists($path_to_delete)) {
        // delete file
        if (unlink($path_to_delete)) {
            // remove record from db
            $query = $db->prepare("DELETE FROM photos WHERE filename = ?");
            $query->bindParam(1, $file_to_delete, SQLITE3_TEXT);
            $result = $query->execute();
            if ($result === false) {
                header('HTTP/1.0 500 Internal Server Error');
            }
        } else {
            header('HTTP/1.0 500 Internal Server Error');
        }
    } else {
        // file not found
        header('HTTP/1.0 404 Not Found');
    }
}

$appdb->close();
$db->close();
?>
