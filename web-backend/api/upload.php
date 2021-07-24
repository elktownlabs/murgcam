<?php
/* Copyright (C) Elktown Labs. - All Rights Reserved
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * Proprietary and confidential
 * Written by Tobias Frodl <toby@elktown-labs.com>, 2021
 */

require("config.php");
require("helpers.php");

date_default_timezone_set("UTC");
setlocale(LC_ALL, '');

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

// function for writing outlined text
function imagettfstroketext(&$image, $size, $angle, $x, $y, &$textcolor, &$strokecolor, $fontfile, $text, $px) {
    for($c1 = ($x-$px); $c1 <= ($x+$px); $c1++) {
        $c2 = $y + round(sqrt($px*$px - ($x-$c1)*($x-$c1)));
        imagettftext($image, $size, $angle, $c1, $c2, $strokecolor, $fontfile, $text);
        $c3 = $y - round(sqrt($px*$px - ($x-$c1)*($x-$c1)));
        imagettftext($image, $size, $angle, $c1, $c3, $strokecolor, $fontfile, $text);
    }
   return imagettftext($image, $size, $angle, $x, $y, $textcolor, $fontfile, $text);
}


$photo_uniqueid = uniqid('cam_');
$target_filename =  $photo_uniqueid . ".jpg";
$upload_timestamp = new DateTime('now');
$db_timestamp = $upload_timestamp->getTimestamp();

// check that photo directory exists
if (!is_dir(PHOTODIR)) {
	header("HTTP/1.1 500 Internal Server Error");
	return;
}

// check that databases exists
if (!is_file(DATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
}
if (!is_file(SETTINGSDATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
}


$photodb = new SQLite3(DATABASE);
if (is_null($photodb)) {
	header("HTTP/1.1 500 Internal Server Error");
}
$settingsdb = new SQLite3(SETTINGSDATABASE);
if (is_null($photodb)) {
	header("HTTP/1.1 500 Internal Server Error");
}

// check that photo is actually there
if (!array_key_exists("photo",  $_FILES)) {
	$results = $photodb->exec("INSERT INTO photos (timestamp, error) VALUES ({$db_timestamp}, 1);");
	header("HTTP/1.1 400 Bad Request");
	$photodb->close();
	$settingsdb->close();
	return;
}

// check for upload errors
if ($_FILES["photo"]["error"] != UPLOAD_ERR_OK) {
	$results = $photodb->exec("INSERT INTO photos (timestamp, error) VALUES ({$db_timestamp}, 2);");
	header("HTTP/1.1 400 Bad Request");
	$photodb->close();
	$settingsdb->close();
	return;
}

// move photo to new location
$photo_tmpname = $_FILES["photo"]["tmp_name"];
$photo_basename = basename($_FILES["photo"]["name"]);
$photo_finalname = PHOTODIR . "/" . $target_filename;
$res = move_uploaded_file($photo_tmpname, $photo_finalname);
if (!$res) {
	$results = $photodb->exec("INSERT INTO photos (timestamp, error) VALUES ({$db_timestamp}, 3);");
	header("HTTP/1.1 500 Internal Server Error");
	$photodb->close();
	$settingsdb->close();
	return;
}
$photodb->close();


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

// check if we have settings
if ($metadata != null) {
	$metadata_decoded = json_decode($metadata, true);
	if ($metadata_decoded != null) {
		if (array_key_exists("settings", $metadata_decoded)) {
			$settings = $metadata_decoded["settings"];
			$settings_encoded = json_encode($settings);
			$query = $settingsdb->prepare("UPDATE config SET active_config=?, active_timestamp=? WHERE cam_id=1");
			$query->bindParam(1, $settings_encoded, SQLITE3_TEXT);
			$query->bindParam(2, $db_timestamp, SQLITE3_TEXT);
			$result = $query->execute();
		}
	}
}
$settingsdb->close();

// add photo to database
$db_filename = SQLite3::escapeString($target_filename);
$db_meta = SQLite3::escapeString($metadata);
$db->exec("INSERT INTO photos (filename, timestamp, meta) VALUES ('{$db_filename}', {$db_timestamp}, '{$db_meta}');");


// create public version of photo with embedded text
$img = imagecreatefromjpeg($photo_finalname);
$img = imagerotate($img, 180, 0);
$white = imagecolorallocate($img, 255, 255, 255);
$black = imagecolorallocate($img, 0, 0, 0);
$font = "./CascadiaMono-SemiBold.otf";
$height = imagesy($img);
$width = imagesx($img);
imagettfstroketext($img, 20, 0, 10, 20+10, $white, $black, $font, PHOTOTEXT_TOP, 2);
imagettfstroketext($img, 20, 0, 10, $height-10, $white, $black, $font, PHOTOTEXT_BOTTOM, 2);
setlocale(LC_TIME, "de_DE");
date_default_timezone_set('Europe/Berlin');
$datestr = strftime("%Y-%m-%d %H:%M:%S %Z");
date_default_timezone_set("UTC");
setlocale(LC_ALL, '');
$boundingbox = imagettfbbox(20, 0, $font, $datestr);
imagettfstroketext($img, 20, 0, $width-$boundingbox[4]-10, 20+10, $white, $black, $font, $datestr, 2);
imagejpeg($img, PHOTOPUBLICDIR."/webcam_1.jpg.part", 100);
rename(PHOTOPUBLICDIR."/webcam_1.jpg.part", PHOTOPUBLICDIR."/webcam_1.jpg");


// calculate delta of operational parameters to be updated
$config_to_send = array();
$result = $db->querySingle("SELECT active_config, modified_config FROM config WHERE cam_id=1", true);
if ($result !== false) {
	$active_config = json_decode($result["active_config"], true);
	$modified_config = json_decode($result["modified_config"], true);
	foreach($modified_config as $key => $value) {
		if (!array_key_exists($key, $active_config)) {
			echo $key;
		} else {
			if ($active_config[$key] != $modified_config[$key]) {
				$config_to_send[$key] = $value;
			}
		}
	}
}

$config_to_send["sys_secs_between_photos_transient"] = 1800;
//$config_to_send["sys_secs_between_photos_transient"] = 120;

header('Content-type:application/json;charset=utf-8');
print(json_encode($config_to_send, JSON_FORCE_OBJECT));
$db->close();
