<?php

require("config.php");
require("imghelpers.php");

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

openlog("webcam-uploader", LOG_PID | LOG_PERROR, LOG_LOCAL0);
syslog(LOG_NOTICE, "upload initiated");

// check that database exists
if (!is_file(DATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
	syslog(LOG_ERR, "Photo database not found: ".DATABASE);
	closelog();
        return;
}
if (!is_file(APPDATABASE)) {
	header("HTTP/1.1 500 Internal Server Error");
	syslog(LOG_ERR, "App database not found: ".APPDATABASE);
	closelog();
        return;
}
$db = new SQLite3(DATABASE);
if (is_null($db)) {
	header("HTTP/1.1 500 Internal Server Error");
	syslog(LOG_ERR, "Photo database could not be opened: ".DATABASE);
	closelog();
        return;
}
$appdb = new SQLite3(APPDATABASE);
if (is_null($appdb)) {
	header("HTTP/1.1 500 Internal Server Error");
	syslog(LOG_ERR, "App database could not be opened: ".APPDATABASE);
	closelog();
        return;
}

// check that photo directory exists
if (!is_dir(PHOTODIR)) {
	header("HTTP/1.1 500 Internal Server Error");
	syslog(LOG_ERR, "Photo target directory does not exist: ".PHOTODIR);
	$db->close();
	$appdb->close();
	closelog();
	return;
}

$photo_uniqueid = uniqid('cam_');
$target_filename =  $photo_uniqueid . ".jpg";
$upload_timestamp = new DateTime('now');
$db_timestamp = $upload_timestamp->getTimestamp();


// check that photo is actually there
if (!array_key_exists("photo",  $_FILES)) {
	$results = $db->exec("INSERT INTO photos (timestamp, error) VALUES ({$db_timestamp}, 1);");
	header("HTTP/1.1 400 Bad Request");
	syslog(LOG_ERR, "Submitted data does not include a photo");
	$db->close();
	$appdb->close();
	closelog();
	return;
}

// check for upload errors
if ($_FILES["photo"]["error"] != UPLOAD_ERR_OK) {
	$results = $db->exec("INSERT INTO photos (timestamp, error) VALUES ({$db_timestamp}, 2);");
	header("HTTP/1.1 400 Bad Request");
	syslog(LOG_ERR, "Submitted data signalled an error: ".$_FILES["photo"]["error"]);
	$db->close();
	$appdb->close();
	closelog();
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
	syslog(LOG_ERR, "Could not move $photo_tmpname to $photo_finalname");
	$db->close();
	$appdb->close();
	closelog();
	return;
}

// rotate image and write text
$archive_photo = fopen("php://memory", "w+");
$public_photo = fopen("php://memory", "w+");
$small_public_photo = fopen("php://memory", "w+");
$imgdata = imagecreatefromjpeg($photo_finalname);
if ($imgdata) {
	imageflip($imgdata, IMG_FLIP_VERTICAL);
	// the flipped image without any additions is used for the archive
	imagejpeg($imgdata, $archive_photo);
	rewind($archive_photo);

	// write some text onto the photo for the public version
	$white = imagecolorallocate($imgdata, 255, 255, 255);
	$black = imagecolorallocate($imgdata, 0, 0, 0);
	$height = imagesy($imgdata);
	$width = imagesx($imgdata);
	imagettfstroketext($imgdata, 15, 0, 10, 20+10, $white, $black, PUBLICTITLEFONT, PUBLICTITLE, 2);
	imagettfstroketext($imgdata, 15, 0, 10, $height-10, $white, $black, PUBLICTITLEFONT, PUBLICSUBTITLE, 2);
	setlocale(LC_TIME, "de_DE");
	date_default_timezone_set("Europe/Berlin");
	$datestr = strftime("%Y-%m-%d %H:%M:%S %Z");
	date_default_timezone_set("UTC");
	$boundingbox = imagettfbbox(15, 0, PUBLICTITLEFONT, $datestr);
	imagettfstroketext($imgdata, 15, 0, $width-$boundingbox[4]-10, 20+10, $white, $black, PUBLICTITLEFONT, $datestr, 2);
	imagejpeg($imgdata, $public_photo);
	rewind($public_photo);

	// create smaller version of photo
	$small_public_imgdata = imagecreatetruecolor(800, 600);
	imagecopyresampled($small_public_imgdata, $imgdata, 0, 0, 0, 0, 800, 600, $width, $height);
	imagejpeg($small_public_imgdata, $small_public_photo);
	rewind($small_public_photo);

	// add exif information to archive photo
	$data = new PelDataWindow(stream_get_contents($archive_photo));
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

	// add exif information to public photo
	$data = new PelDataWindow(stream_get_contents($public_photo));
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
	$jpeg->saveFile(PUBLICIMGLOCATION.".part");
	rename(PUBLICIMGLOCATION.".part", PUBLICIMGLOCATION);
	
	
	// add exif information to small public photo
	$data = new PelDataWindow(stream_get_contents($small_public_photo));
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
	$jpeg->saveFile(PUBLICIMGLOCATIONSMALL.".part");
	rename(PUBLICIMGLOCATIONSMALL.".part", PUBLICIMGLOCATIONSMALL);

} else {
        syslog(LOG_ERR, "Image seems to be invalid");
}

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
if ($metadata !== null) {
	$metadata_decoded = json_decode($metadata, true);
	if ($metadata_decoded !== null) {
		if (array_key_exists("settings", $metadata_decoded)) {
			$settings = $metadata_decoded["settings"];
			$settings_encoded = json_encode($settings);
			$query = $appdb->prepare("UPDATE cam_config SET active_config=?, active_timestamp=? WHERE cam_id=1");
			$query->bindParam(1, $settings_encoded, SQLITE3_TEXT);
			$query->bindParam(2, $db_timestamp, SQLITE3_TEXT);
			$result = $query->execute();
			syslog(LOG_NOTICE, "Metadata submitted: ".$metadata);
		}
	} else {
		syslog(LOG_WARNING, "Metadata is not valid json: ".$metadata);
	}
}

// add photo to database
$db_filename = SQLite3::escapeString($target_filename);
$db_meta = SQLite3::escapeString($metadata);
$db->exec("INSERT INTO photos (filename, timestamp, meta) VALUES ('{$db_filename}', {$db_timestamp}, '{$db_meta}');");


// calculate delta of operational parameters to be updated
$config_to_send = array();
$result = $appdb->querySingle("SELECT active_config, modified_config FROM cam_config WHERE cam_id=1", true);
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


$transient_secs_determined = false;
// figure out if we have to pass a transient delay value
$result = $appdb->querySingle("SELECT server_config from cam_config WHERE cam_id=1", true);
if ($result !== false) {
	$server_config = json_decode($result["server_config"], true);

	// figure out if we have a developer override for transient time to next photo
	$val = $server_config["secs_between_photos_override"];
	if (is_numeric($val)) {
		$val = intval($val);
		if ($val < 60) $val = 60;
		$transient_secs_determined = true;
		$config_to_send["sys_secs_between_photos_transient"] = $val;
	}

	// figure out if we are in time out (after last photo of the day before first photo of the next day
	if (!$transient_secs_determined) {
		$current_time = new DateTime("now", new DateTimeZone("Europe/Berlin"));
		$start_time = DateTime::createFromFormat("H:i", $server_config["start_time"], new DateTimeZone("Europe/Berlin"));
		$end_time = DateTime::createFromFormat("H:i", $server_config["end_time"], new DateTimeZone("Europe/Berlin"));
		if ($start_time !== null && $end_time == null) {
			$end_time = DateTime::createFromFormat("H:i", "00:00", new DateTimeZone("Europe/Berlin"));
			$end_time->modify("+1 day");
		}
		if ($start_time == null && $end_time !== null) {
			$start_time = DateTime::createFromFormat("H:i", "00:00", new DateTimeZone("Europe/Berlin"));
		}
		if ($start_time > $end_time) $end_time->modify("+1 day");
		if ($start_time !== null && $end_time !== null) {
			$active = ($current_time->getTimestamp() >= $start_time->getTimestamp()) && ($current_time->getTimestamp() <= $end_time->getTimestamp());
			if (!$active) {
				$interval = $start_time->getTimestamp() - $current_time->getTimestamp();
				if ($interval < 0) {
					$start_time->modify("+1 day");
					$interval = $start_time->getTimestamp() - $current_time->getTimestamp();
				}

				// if we are within 10mins of the interval start, assume that this is the first picture
				// of the interval which came a early due to clock drift.
				if ($interval >= 10*60) {
					$config_to_send["sys_secs_between_photos_transient"] = $interval;
					$transient_secs_determined = true;
				}
			}
		}
	}
}


if (!$transient_secs_determined) {
	// figure out if we are in fast or express mode
	$result = $appdb->querySingle("SELECT * from cam_frequency WHERE cam_id=1", true);
	if ($result !== false) {
		$write_back = false;
		$mode_to_use = 0;
		$current_time = time();
		$current_mode = $result["current_mode"];
		$requested_mode = $result["requested_mode"];

		if ($requested_mode !== null) {
			// mode change requested
			if (($result["requested_mode_end"] == null) || ($result["requested_mode_end"] <= $current_time)) {
				// mode has already expired -> go back to regular mode
				$result["requested_mode"] = null;
				$result["requested_mode_end"] = null;
				$result["requested_mode_initiating_time"] = null;
				$result["requested_mode_initiating_user"] = null;
				$result["current_mode"] = 0;
				$result["current_mode_start"] = $current_time;
				$result["current_mode_end"] =  null;
				$result["current_mode_initiating_user"] = null;
				$result["current_mode_initiating_time"] =  null;
				$write_back = true;
				$mode_to_use = $result["current_mode"] = 0;
			} else {
				// switch mode to requested mode
				$result["current_mode"] = $result["requested_mode"];
				$result["current_mode_start"] = $current_time;
				$result["current_mode_end"] =  $result["requested_mode_end"];
				$result["current_mode_initiating_user"] = $result["requested_mode_initiating_user"];
				$result["current_mode_initiating_time"] =  $result["requested_mode_initiating_time"];
				$result["requested_mode"] = null;
				$result["requested_mode_end"] = null;
				$result["requested_mode_initiating_time"] = null;
				$result["requested_mode_initiating_user"] = null;
				$write_back = true;
				$mode_to_use = $result["current_mode"];
			}
		} else if ($current_mode !== null && $current_mode != 0) {
			if  (($result["current_mode_end"] == null) || ($result["current_mode_end"] <= $current_time)) {
				// current mode elapsed or an invalid end time has been specified -> go back to regular mode
				$result["requested_mode"] = null;
				$result["requested_mode_end"] = null;
				$result["requested_mode_initiating_time"] = null;
				$result["requested_mode_initiating_user"] = null;
				$result["current_mode"] = 0;
				$result["current_mode_start"] = $current_time;
				$result["current_mode_end"] =  null;
				$result["current_mode_initiating_user"] = null;
				$result["current_mode_initiating_time"] =  null;
				$write_back = true;
				$mode_to_use = $result["current_mode"] = 0;
			} else {
				// current mode is still active
				$write_back = false;
				$mode_to_use = $result["current_mode"];
			}
		}

		if ($write_back) {
			// write changes back to db
			$query = $appdb->prepare(
				"UPDATE cam_frequency SET
					current_mode=?,
					current_mode_start=?,
					current_mode_end=?,
					current_mode_initiating_user=?,
					current_mode_initiating_time=?,
					requested_mode=?,
					requested_mode_end=?,
					requested_mode_initiating_time=?,
					requested_mode_initiating_user=?
				WHERE cam_id=1");
			$query->bindParam(1, $result["current_mode"], SQLITE3_INTEGER);
			$query->bindParam(2, $result["current_mode_start"], SQLITE3_INTEGER);
			$query->bindParam(3, $result["current_mode_end"], SQLITE3_INTEGER);
			$query->bindParam(4, $result["current_mode_initiating_user"], SQLITE3_TEXT);
			$query->bindParam(5, $result["current_mode_initiating_time"], SQLITE3_INTEGER);
			$query->bindParam(6, $result["requested_mode"], SQLITE3_INTEGER);
			$query->bindParam(7, $result["requested_mode_end"], SQLITE3_INTEGER);
			$query->bindParam(8, $result["requested_mode_initiating_time"], SQLITE3_INTEGER);
			$query->bindParam(9, $result["requested_mode_initiating_user"], SQLITE3_TEXT);
			$result = $query->execute();
		}

		switch ($mode_to_use) {
		case 1:
			$config_to_send["sys_secs_between_photos_transient"] = 30*60;
			$transient_secs_determined = true;
			break;
		case 2:
			$config_to_send["sys_secs_between_photos_transient"] = 10*60;
			$transient_secs_determined = true;
			break;
		default:
		}
	}
}


header('Content-type:application/json;charset=utf-8');
print(json_encode($config_to_send, JSON_FORCE_OBJECT));
$db->close();
$appdb->close();
closelog();
