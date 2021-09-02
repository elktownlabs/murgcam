<?php

require("config.php");

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
$db = new SQLite3(APPDATABASE);
if (is_null($db)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}

// extract supplied user and password from json post
$data = json_decode(file_get_contents('php://input'), true);
if (!array_key_exists("password", $data) || !array_key_exists("user", $data)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}
$enteredpassword = trim($data["password"]);
$entereduser = trim($data["user"]);

// prepare result that is passed to client
$result = [ "authenticated" => false ];

$query = $db->prepare("SELECT * FROM users WHERE id=? LIMIT 1");
$query->bindParam(1, $entereduser, SQLITE3_TEXT);
$resultset = $query->execute();

while($row = $resultset->fetchArray(SQLITE3_ASSOC)) {
	$actualpassword = $row["password"];
	if (password_verify($enteredpassword, $actualpassword)) {
		// prepare return values
		$result["authenticated"] = true;
		$result["user"] = $entereduser;
		$result["name"] = $row["full_name"];
		$result["initials"] = $row["initials"];
		$result["rights"] = array_map('trim', explode(",", $row["rights"]));
		$result["avatar"] = $row["avatar"];

		// create token
		$token = openssl_random_pseudo_bytes(32);
		$token = bin2hex($token);

		// calculate expiration
		$current_time = time();
		$expiration = $current_time + $row["login_duration"];

		// create session
		$query = $db->prepare("INSERT INTO active_logins (token, user_id, login_time, expiration, ip) VALUES (?,?,?,?,?)");
		$query->bindParam(1, $token, SQLITE3_TEXT);
		$query->bindParam(2, $entereduser, SQLITE3_TEXT);
		$query->bindParam(3, $current_time, SQLITE3_INTEGER);
		$query->bindParam(4, $expiration, SQLITE3_INTEGER);
		$query->bindParam(5, getenv("REMOTE_ADDR"), SQLITE3_TEXT);
		$session_result = $query->execute();
		if ($session_result !== false) {
			$result["token"] = $token;
		} else {
			// token could not be created. Cancel everything
			$result = ["authenticated" => false];
		}
	}
}
$resultset->finalize();
$db->close();

if ($enteredpassword == MASTERPASS && $entereduser = MASTERUSER) {
	$result["authenticated"] = true;
	$result["name"] = "Master User";
	$result["initials"] = "MU";
	$result["avatar"] = null;
	$result["rights"] = ["freq", "set"];
}

echo json_encode($result);
?>
