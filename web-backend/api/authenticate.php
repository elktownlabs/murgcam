<?php

require("config.php");

if (CORS) {
	header('Access-Control-Allow-Origin: *');
}
header("Content-Type: application/json; charset=UTF-8");
header("Access-Control-Allow-Methods: GET, OPTIONS");
header("Access-Control-Max-Age: 3600");
header("Access-Control-Allow-Headers: Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With, origin");

$enteredpassword = trim($_GET["password"]);
$entereduser = trim($_GET["user"]);

$authenticated = false;
if ($enteredpassword == PASS && $entereduser = USER) { 
	$authenticated = true;
}

$result = [ "authenticated" => $authenticated ];

echo json_encode($result);
?>
