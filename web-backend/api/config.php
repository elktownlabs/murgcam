<?php
/* Copyright (C) Elktown Labs. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Tobias Frodl <toby@elktown-labs.com>, 2021
 */

define('__ROOT__', dirname(dirname(__FILE__)));

if ($_SERVER['ENV'] == "dev") {
    // development environment
    define('DATABASE', __ROOT__.'/photos/photos.db');
    define('CELLDATABASE', __ROOT__.'/db/cell.db');
    define('USERDATABASE', __ROOT__.'/db/users.db');
    define('PHOTODIR', __ROOT__.'/photos/');
} else {
    // production envrionment
    define('DATABASE', '/var/www/webcam/photos/photos.db');
    define('CELLDATABASE', '/var/www/webcam/db/cell.db');
    define('USERDATABASE', '/var/www/webcam/db/users.db');
    define('PHOTODIR', '/var/www/webcam/photos/');
}

define('CORS', true);

// backdoor access in case of lockout
define('BACKDOORUSER', 'user');
define('BACKDOORPASS', 'user00!');

date_default_timezone_set("Europe/Berlin");

?>
