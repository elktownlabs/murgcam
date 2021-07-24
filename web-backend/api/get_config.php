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

$do_base64 = false;
if (array_key_exists("base64", $_GET)) {
	$do_base64 = true;
}
 
// check that database exists
if (!is_file(SETTINGSDATABASE)) {
       	header("HTTP/1.1 500 Internal Server Error");
        return;
}
$db = new SQLite3(SETTINGSDATABASE);
if (is_null($db)) {
        header("HTTP/1.1 500 Internal Server Error");
        return;
}


$result = $db->querySingle("SELECT active_config, active_timestamp, modified_config, modified_timestamp FROM config WHERE cam_id = 1", true);
if ($result === false) {
        header("HTTP/1.1 404 Not Found");
        return;
}

$returnvalue = [];
$returnvalue["active"] = json_decode($result["active_config"]);
$returnvalue["active_timestamp"] = $result["active_timestamp"];
$returnvalue["modified"] = json_decode($result["modified_config"]);
$returnvalue["modified_timestamp"] = $result["modified_timestamp"];

echo json_encode($returnvalue);
?>
