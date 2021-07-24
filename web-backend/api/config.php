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
    define('SETTINGSDATABASE', __ROOT__.'/db/settings.db');
    define('PHOTODIR', __ROOT__.'/photos/');
    define('PHOTOPUBLICDIR', __ROOT__);
} else {
    // production envrionment
    define('DATABASE', '/var/www/webcam/photos/photos.db');
    define('CELLDATABASE', '/var/www/webcam/db/cell.db');
    define('SETTINGSDATABASE', '/var/www/webcam/db/settings.db');
    define('PHOTODIR', '/var/www/webcam/photos/');
    define('PHOTOPUBLICDIR', __ROOT__);
}

define('CORS', true);

// text to add to photo
define ('PHOTOTEXT_TOP', 'Murg oberhalb Hundsbachmündung');
define ('PHOTOTEXT_BOTTOM', 'Wildwasserverein Schwarzwald e.V. - wwv-schwarzwald.de');

// backdoor access in case of lockout
define('BACKDOORUSER', 'user');
define('BACKDOORPASS', 'user00!');

date_default_timezone_set("Europe/Berlin");

?>
