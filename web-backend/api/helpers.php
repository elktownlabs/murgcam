<?php
/* Copyright (C) Elktown Labs. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Tobias Frodl <toby@elktown-labs.com>, 2021
 */

require_once("config.php");


function output_error($str) {
    if ($_SERVER['ENV'] == "dev") {
        print("<html><body>".$str."</body></html\n".__ROOT__);
    }
}

function authenticate_user() {
    if (!isset($_SERVER['PHP_AUTH_USER'])) {
        return false;
    }
    if (!isset($_SERVER['PHP_AUTH_PW'])) {
        return false;
    }

    $user = $_SERVER['PHP_AUTH_USER'];
    $pass = $_SERVER['PHP_AUTH_PW'];

    // backdoor
    if (($pass == BACKDOORPASS) && ($user == BACKDOORUSER)) {
        return true;
    }

    // db authentication

    // check that database exists
    if (!is_file(USERDATABASE)) {
        output_error("authenticate_user: Database not found.");
        return false;
    }

    $db = new SQLite3(USERDATABASE);
    if (is_null($db)) {
        output_error("authenticate_user: Database interface could not be created.");
        return false;
    }

    // retrieve user from db
    $query = $db->prepare("SELECT username,password,fullname,active FROM users WHERE username=?");
    $query->bindParam(1, $entereduser, SQLITE3_TEXT);
    $result = $query->execute();
    $row = $result->fetchArray(SQLITE3_ASSOC);
    if ($row) {
        // check that user is active
        if ($row["active"] != 1) {
            output_error("authenticate_user: User inactive.");
            return false;
        } else {
            $authenticated = password_verify($enteredpassword, $row["password"]);
            if ($authenticated == false) {
                output_error("authenticate_user: Password wrong.");
            }
            return $authenticated;
        }
    } else {
        output_error("authenticate_user: User does not exist.");
        return false;
    }


}


?>
