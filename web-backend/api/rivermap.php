<?php
// api used by rivermap.org

require("config.php");
require("libphotoannotation.php");

// configuration values used for just this api
define ('DEFAULTRANGE', "P10D"); /* 2 days - format according to PHP DateTimeInterval */
define ('PHOTOBASEURL', "https://wwv-schwarzwald.de/webcam/api/v1/rivermap?photo=%s");


if (CORS) {
        header('Access-Control-Allow-Origin: *');
}
header("Access-Control-Allow-Methods: GET, OPTIONS");
header("Access-Control-Max-Age: 3600");
header("Access-Control-Allow-Headers: Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With, origin");


// ignore options requests
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    return 0;
}


////////////////////////////////////
// -- API subfunction json_cal -- //
////////////////////////////////////
// Returns a list of reference images

if(isset($_GET['json_cal'])) {
    $refImages = array(
        "min" => "https://wwv-schwarzwald.de/webcam/api/v1/rivermap?refphoto=min",
        "med" => "https://wwv-schwarzwald.de/webcam/api/v1/rivermap?refphoto=med",
        "max" => "https://wwv-schwarzwald.de/webcam/api/v1/rivermap?refphoto=max"
    );


    header('Content-Type: application/json');
    echo json_encode($refImages)."\n";
    exit(0);
}



////////////////////////////////
// -- API subfunction json -- //
////////////////////////////////
// Returns a list of all files matching the provided search criteria

if(isset($_GET['json'])) {

    if (isset($_GET['newest'])) {
        // get date of newest photo from call arguments
        $newest_photo = new DateTime($_GET['newest']);
    } else {
        // get date of newest photo from default
        $newest_photo = new DateTime();
    }

    if(isset($_GET['oldest'])) {
        // get date of oldest photo from call arguments
        $oldest_photo = new DateTime($_GET['oldest']);
    } else {
        // calc oldest photo from default range
        $oldest_photo = clone $newest_photo;
        $oldest_photo->sub(new DateInterval(DEFAULTRANGE));
    };


    // check that database exists
    if (!is_file(DATABASE)) {
	    header("HTTP/1.1 500 Internal Server Error");
    	exit(0);
    }
    // connect to database
    $db = new SQLite3(DATABASE);
    if (is_null($db)) {
    	header("HTTP/1.1 500 Internal Server Error");
	    exit(0);
    }

    $query = $db->prepare("SELECT * FROM photos WHERE Timestamp>=? AND Timestamp<? ORDER BY Timestamp ASC");
    $query->bindParam(1, $oldest_photo->getTimestamp(), SQLITE3_INTEGER);
    $query->bindParam(2, $newest_photo->getTimestamp(), SQLITE3_INTEGER);
    $result = $query->execute();
    $queryresult = [];
    while($row = $result->fetchArray(SQLITE3_ASSOC)) {
        $id = substr($row["filename"], 4, 13);
        $filename = sprintf(PHOTOBASEURL, $id);
        $timestamp = $row["timestamp"];
        $queryresult[$timestamp] = $filename;
    }
    $db->close();

    header('Content-Type: application/json');
    echo json_encode($queryresult);
    exit(0);
  }


/////////////////////////////////
// -- API subfunction photo -- //
/////////////////////////////////
// Returns a list of all files matching the provided search criteria

if(isset($_GET['photo'])) {

    // check that database exists
    if (!is_file(DATABASE)) {
	    header("HTTP/1.1 500 Internal Server Error");
    	exit(0);
    }
    // connect to database
    $db = new SQLite3(DATABASE);
    if (is_null($db)) {
    	header("HTTP/1.1 500 Internal Server Error");
	    exit(0);
    }

    // sanity check for id
    $id = $_GET['photo'];
    if (strlen($id) != 13) {
    	header("HTTP/1.1 500 Internal Server Error");
	    exit(0);
    }


    $filename = sprintf("cam_%s.jpg", $id);
    $query = $db->prepare("SELECT timestamp FROM photos WHERE filename=? LIMIT 1");
    $query->bindParam(1, $filename, SQLITE3_TEXT);
    $result = $query->execute();
    $photoexists = false;
    $timestamp = null;

	while($row = $result->fetchArray(SQLITE3_NUM)) {
        $photoexists = true;
        $timestamp = new DateTime("@".$row[0]);
        $timestamp->setTimeZone(new DateTimeZone('Europe/Berlin'));
	}

    if ($photoexists) {
        // there is a row in the database. Everything is ok. Return photo
        header('Content-Type: image/jpeg');
        header('Content-Disposition: inline');
        $imgdata = file_get_contents(PHOTODIR."/".$filename);
        annotatePhotoVisual($imgdata, $timestamp);
        annotatePhotoExif($imgdata, $id, $timestamp);
        print($imgdata);
    } else {
        // no rows in database
        header("HTTP/1.1 404 Not Found");
    }
    $db->close();
    exit(0);
  }


///////////////////////////////////
// -- API subfunction current -- //
///////////////////////////////////
// Returns a list of all files matching the provided search criteria

if(isset($_GET['current'])) {

    // check that database exists
    if (!is_file(DATABASE)) {
	    header("HTTP/1.1 500 Internal Server Error");
    	exit(0);
    }
    // connect to database
    $db = new SQLite3(DATABASE);
    if (is_null($db)) {
    	header("HTTP/1.1 500 Internal Server Error");
	    exit(0);
    }

    $query = $db->prepare("SELECT timestamp, filename FROM photos ORDER BY timestamp DESC LIMIT 1");
    $result = $query->execute();
    $photoexists = false;
    $timestamp = null;
    $filename = "";

	while($row = $result->fetchArray(SQLITE3_NUM)) {
        $photoexists = true;
        $timestamp = new DateTime("@".$row[0]);
        $timestamp->setTimeZone(new DateTimeZone('Europe/Berlin'));
        $filename = $row[1];
	}

    if ($photoexists) {
        // there is a row in the database. Everything is ok. Return photo
        header('Content-Type: image/jpeg');
        header('Content-Disposition: inline');
        $imgdata = file_get_contents(PHOTODIR."/".$filename);
        annotatePhotoVisual($imgdata, $timestamp);
        annotatePhotoExif($imgdata, $id, $timestamp);
        print($imgdata);
    } else {
        // no rows in database
        header("HTTP/1.1 404 Not Found");
    }
    $db->close();
    exit(0);
  }


////////////////////////////////////
// -- API subfunction refphoto -- //
////////////////////////////////////
// Returns a list of all files matching the provided search criteria

if(isset($_GET['refphoto'])) {
    $reflabel = $_GET['refphoto'];
    header('Content-Type: image/jpeg');
    header('Content-Disposition: inline');
    print(file_get_contents("testcard.jpg"));
    exit(0);
  }


?>

