<?php

define('DATABASE', '/var/www/webcam/photos/photos.db');
define('APPDATABASE', '/var/www/webcam/db/app.db');
define('CELLDATABASE', '/var/www/webcam/db/cell.db');
define('PHOTODIR', '/var/www/webcam/photos/');

define('CORS', true);

define('MASTERUSER', 'admin');
define('MASTERPASS', 'supersecretpassword');

define('PUBLICTITLEFONT', '/var/www/webcam/api/CascadiaMono-SemiBold.otf');
define('PUBLICTITLE', 'Murg oberhalb Raumünzachmündung');
define('PUBLICSUBTITLE', 'Wildwasserverein Schwarzwald e.V.');
define('PUBLICIMGLOCATION', '/var/www/webcam/current.jpg');
define('PUBLICIMGLOCATIONSMALL', '/var/www/webcam/current_small.jpg');

date_default_timezone_set("Europe/Berlin");

?>
