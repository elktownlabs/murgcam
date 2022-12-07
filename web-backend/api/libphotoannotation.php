<?php

require_once("config.php");
require_once("imghelpers.php");

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


function annotatePhotoVisual(&$imgdata, $timestamp)
{
    // write some text onto the photo for the public version
    $img = imagecreatefromstring($imgdata);
    $white = imagecolorallocate($img, 255, 255, 255);
    $black = imagecolorallocate($img, 0, 0, 0);
    $height = imagesy($img);
    $width = imagesx($img);
    imagettfstroketext($img, 15, 0, 10, 20+10, $white, $black, PUBLICTITLEFONT, PUBLICTITLE, 2);
    imagettfstroketext($img, 15, 0, 10, $height-10, $white, $black, PUBLICTITLEFONT, PUBLICSUBTITLE, 2);
    setlocale(LC_TIME, "de_DE");
    $datestr = $timestamp->format("Y-m-d H:i:s T");
    date_default_timezone_set("UTC");
    $boundingbox = imagettfbbox(15, 0, PUBLICTITLEFONT, $datestr);
    imagettfstroketext($img, 15, 0, $width-$boundingbox[4]-10, 20+10, $white, $black, PUBLICTITLEFONT, $datestr, 2);
    $output = fopen("php://memory", "w+");
    imagejpeg($img, $output);
    rewind($output);
    $imgdata = stream_get_contents($output);
}



function annotatePhotoExif(&$imgdata, $documentname, $timestamp)
{
    // add exif information to archive photo
    $data = new PelDataWindow($imgdata);
    $jpeg = new PelJpeg();
    $jpeg->load($data);
    $exif = new PelExif();
    $jpeg->setExif($exif);
    $tiff = new PelTiff();
    $exif->setTiff($tiff);
    $ifd0 = new PelIfd(PelIfd::IFD0);
    $tiff->setIfd($ifd0);
    $entry = new PelEntryAscii(PelTag::DOCUMENT_NAME, $documentname);
    $ifd0->addEntry($entry);
    $entry = new PelEntryAscii(PelTag::ARTIST, "WWV Schwarzwald e.V.");
    $ifd0->addEntry($entry);
    $entry = new PelEntryAscii(PelTag::MAKE, "Elktown Labs.");
    $ifd0->addEntry($entry);
    $entry = new PelEntryAscii(PelTag::MODEL, "Murgcam");
    $ifd0->addEntry($entry);
    $entry = new PelEntryAscii(PelTag::IMAGE_DESCRIPTION, "Photo of the one and only river Murg! Come and enjoy some of the best whitewater in Germany! (".  date_format($timestamp, 'c') . " UTC)");
    $ifd0->addEntry($entry);
    $entry = new PelEntryShort(PelTag::ORIENTATION, 1);
    $ifd0->addEntry($entry);
    $entry = new PelEntryTime(PelTag::DATE_TIME, $timestamp->getTimestamp());
    $ifd0->addEntry($entry);
    $entry = new PelEntryAscii(PelTag::COPYRIGHT, "Copyright WWV Schwarzwald e.V., " . $timestamp->format("Y") . ". All rights reserved.");
    $ifd0->addEntry($entry);
    $entry = new PelEntryAscii(PelTag::SOFTWARE, "{$timestamp->getTimestamp()}");
    $ifd0->addEntry($entry);
    $ifdexif = new PelIfd(PelIfd::EXIF);
    $ifd0->addSubIfd($ifdexif);
    $entry = new PelEntryAscii(PelTag::IMAGE_UNIQUE_ID, "{$documentname}");
    $ifdexif->addEntry($entry);
    $imgdata = $jpeg->getBytes();
}


?>