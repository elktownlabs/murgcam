<?php

require_once("config.php");

// this can be called at the top of an api entry point to check if the
// user has all required rights
function murgcam_authenticate($appdb, $token, $requiredrights="")
{
    $adminuser = MASTERUSER;
    $query = $appdb->prepare("SELECT s.expiration as expiration, u.rights as rights FROM active_logins s, users u WHERE (u.id=s.user_id OR s.user_id=?) AND token=? LIMIT 1;");
    $query->bindParam(1, $adminuser, SQLITE3_TEXT);
    $query->bindParam(2, $token, SQLITE3_TEXT);
    $resultset = $query->execute();
    $authenticated = false;
    while($row = $resultset->fetchArray(SQLITE3_ASSOC)) {
        // check that we're not beyond the expiration time
        if ($row["expiration"] >= time()) $authenticated = true;
        // check rights
        if (trim($requiredrights) != "") {
            $userrights = array_map('trim', explode(",", $row["rights"]));
            if (in_array(trim($requiredrights), $userrights)) $authenticated = $authenticated && true;
        }
    }
    $resultset->finalize();
    return $authenticated;
}


?>