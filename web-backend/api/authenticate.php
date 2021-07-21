<?php

require("config.php");

if (CORS) {
	header('Access-Control-Allow-Origin: *');
}
header("Content-Type: application/json; charset=UTF-8");
header("Access-Control-Allow-Methods: GET, OPTIONS");
header("Access-Control-Max-Age: 3600");
header("Access-Control-Allow-Headers: Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With, origin");

// check that requied fields are present
if (!(isset($_GET["password"]) && isset($_GET["user"]))) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}

$enteredpassword = trim($_GET["password"]);
$entereduser = trim($_GET["user"]);

// backdoor
if (($enteredpassword == BACKDOORPASS) && ($entereduser == BACKDOORUSER)) {
	$result = [ "authenticated" => true ];

	echo json_encode($result);
	return;
}

// check that database exists
if (!is_file(USERDATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}


$db = new SQLite3(USERDATABASE);
if (is_null($db)) {
	header("HTTP/1.1 500 Internal Server Error");
        return;
}


$authenticated = false;

// retrieve user from db
$query = $db->prepare("SELECT username,password,fullname,active FROM users WHERE username=?");
$query->bindParam(1, $entereduser, SQLITE3_TEXT);
$result = $query->execute();
$row = $result->fetchArray(SQLITE3_ASSOC);
if ($row) {
	// check that user is active
	if ($row["active"] != 1) {
		$authenticated = false;
		$reason = "User locked";
	} else {
		//print($row["password"]);
		$authenticated = password_verify($enteredpassword, $row["password"]);
		if (!$authenticated) {
			$reason = "User not found";
		}
	}
} else {
	$authenticated = false;	
	$reason = "User not found";
}


$result = [ "authenticated" => $authenticated ];
if (!$authenticated) {
	$result["reason"] = $reason;
}

echo json_encode($result);
?>
