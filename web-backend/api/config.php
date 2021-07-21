<?php

if (getenv('ENV') == "dev") {
    define('DATABASE', 'db/photos.db');
    define('CELLDATABASE', 'db/cell.db');
    define('USERDATABASE', '../db/users.db');
    define('PHOTODIR', 'photos/');
} else {
    define('DATABASE', '/var/www/webcam/photos/photos.db');
    define('CELLDATABASE', '/var/www/webcam/db/cell.db');
    define('USERDATABASE', '/var/www/webcam/db/users.db');
    define('PHOTODIR', '/var/www/webcam/photos/');
}

define('CORS', true);

define('BACKDOORUSER', 'user');
define('BACKDOORPASS', 'user00!');

date_default_timezone_set("Europe/Berlin");

?>
