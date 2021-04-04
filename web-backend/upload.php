<?php

date_default_timezone_set("UTC");
setlocale(LC_ALL, '');

define('DATABASE', '/var/www/webcam/photos/photos.db');
define('PHOTODIR', '/var/www/webcam/photos/');

function __autoload($class)
{
    $parts = explode('\\', $class);
    require implode('/', $parts) . '.php';
}

use lsolesen\pel\Pel;
use lsolesen\pel\PelConvert;
use lsolesen\pel\PelDataWindow;
use lsolesen\pel\PelEntryAscii;
use lsolesen\pel\PelEntryShort;
use lsolesen\pel\PelEntryTime;
use lsolesen\pel\PelEntryCopyright;
use lsolesen\pel\PelExif;
use lsolesen\pel\PelIfd;
use lsolesen\pel\PelJpeg;
use lsolesen\pel\PelTag;
use lsolesen\pel\PelTiff;

$photo_uniqueid = uniqid('cam_');
$target_filename =  $photo_uniqueid . ".jpg";
$upload_timestamp = new DateTime('now');
$db_timestamp = $upload_timestamp->getTimestamp();

// check that photo directory exists
if (!is_dir(PHOTODIR)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}

// check that database exists
if (!is_file(DATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
}
$db = new SQLite3(DATABASE);
if (is_null($db)) {
	header("HTTP/1.1 500 Internal Server Error");
}

// check that photo is actually there
if (!array_key_exists("photo",  $_FILES)) {
	$results = $db->exec("INSERT INTO photos (timestamp, error) VALUES ({$db_timestamp}, 1);");
	header("HTTP/1.1 400 Bad Request");
	$db->close();
	return;
}

// check for upload errors
if ($_FILES["photo"]["error"] != UPLOAD_ERR_OK) {
	$results = $db->exec("INSERT INTO photos (timestamp, error) VALUES ({$db_timestamp}, 2);");
	header("HTTP/1.1 400 Bad Request");
	$db->close();
	return;
}

// move photo to new location
$photo_tmpname = $_FILES["photo"]["tmp_name"];
$photo_basename = basename($_FILES["photo"]["name"]);
$photo_finalname = PHOTODIR . "/" . $target_filename;
$res = move_uploaded_file($photo_tmpname, $photo_finalname);
if (!$res) {
	$results = $db->exec("INSERT INTO photos (timestamp, error) VALUES ({$db_timestamp}, 3);");
	header("HTTP/1.1 500 Internal Server Error");
	$db->close();
	return;
}


// add exif information
$data = new PelDataWindow(file_get_contents($photo_finalname));
$jpeg = new PelJpeg();
$jpeg->load($data);
$exif = new PelExif();
$jpeg->setExif($exif);
$tiff = new PelTiff();
$exif->setTiff($tiff);
$ifd0 = new PelIfd(PelIfd::IFD0);
$tiff->setIfd($ifd0);
$entry = new PelEntryAscii(PelTag::DOCUMENT_NAME, $target_filename);
$ifd0->addEntry($entry);
$entry = new PelEntryAscii(PelTag::ARTIST, "WWV Schwarzwald e.V.");
$ifd0->addEntry($entry);
$entry = new PelEntryAscii(PelTag::MAKE, "Elktown Labs.");
$ifd0->addEntry($entry);
$entry = new PelEntryAscii(PelTag::MODEL, "Murgcam");
$ifd0->addEntry($entry);
$entry = new PelEntryAscii(PelTag::IMAGE_DESCRIPTION, "Photo of the one and only river Murg! Come and enjoy some of the best whitewater in Germany! (".  date_format($upload_timestamp, 'c') . " UTC)");
$ifd0->addEntry($entry);
$entry = new PelEntryShort(PelTag::ORIENTATION, 1);
$ifd0->addEntry($entry);
$entry = new PelEntryTime(PelTag::DATE_TIME, $upload_timestamp->getTimestamp());
$ifd0->addEntry($entry);
$entry = new PelEntryAscii(PelTag::COPYRIGHT, "Copyright WWV Schwarzwald e.V., " . $upload_timestamp->format("Y") . ". All rights reserved.");
$ifd0->addEntry($entry);
$entry = new PelEntryAscii(PelTag::SOFTWARE, "{$upload_timestamp->getTimestamp()}");
$ifd0->addEntry($entry);
$ifdexif = new PelIfd(PelIfd::EXIF);
$ifd0->addSubIfd($ifdexif);
$entry = new PelEntryAscii(PelTag::IMAGE_UNIQUE_ID, "{$photo_uniqueid}");
$ifdexif->addEntry($entry);
$jpeg->saveFile($photo_finalname);

// check if we have meta data
$metadata = null;
if (array_key_exists("meta",  $_FILES)) {
	if ($_FILES["meta"]["error"] == UPLOAD_ERR_OK) {
		$filepath = $_FILES["meta"]["tmp_name"];
		$metadata = file_get_contents($filepath, false);
		unlink($filepath);
	}
}

// add photo to database
$db_filename = SQLite3::escapeString($target_filename);
$db_meta = SQLite3::escapeString($metadata);
$db->exec("INSERT INTO photos (filename, timestamp, meta) VALUES ('{$db_filename}', {$db_timestamp}, '{$db_meta}');");

$data = array(
	"sys_secs_between_photos" => 1800,
	"sys_secs_between_photos_transient" => 300,
	"cam_light_mode" => 2
);

header('Content-type:application/json;charset=utf-8');
print(json_encode($data));
$db->close();
