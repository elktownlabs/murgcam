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

// authentication
if (!isset($_SERVER['PHP_AUTH_USER'])) {
    header('WWW-Authenticate: Basic realm="WebCam API"');
    header('HTTP/1.0 401 Unauthorized');
    exit;
} elseif ($_SERVER['PHP_AUTH_PW'] != PASS || $_SERVER['PHP_AUTH_USER'] != USER) { 
    header('WWW-Authenticate: Basic realm="WebCam API"');
    header('HTTP/1.0 401 Unauthorized');
    exit;
}


if (!array_key_exists("id", $_GET)) {
    header('HTTP/1.0 404 Not Found');
    exit;
}

$id = trim($_GET["id"]);
$filename = "cam_" . $id . ".jpg";

$photo = file_get_contents(PHOTODIR."/".$filename);

if ($photo === false) {
    header('HTTP/1.0 404 Not Found');
    exit;
}

header("Content-Type: image/jpeg");

echo $photo;

?>
