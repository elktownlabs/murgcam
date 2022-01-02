#!/bin/env python3
import logging
import requests
from jsmin import jsmin
from dateutil import parser as dateParser;
import time
import json
import sqlite3
import argparse
import daemon
import signal
import sys
from daemon import pidfile

sleep_time = 5 * 60
hvz_url = "https://www.hvz.baden-wuerttemberg.de/js/hvz_peg_stmn.js"
hvz_wanted_sources = {
  "Schwarzenberg": 79,
  "Schönmünzach": 170,
  "Bad Rotenfels": 111,
}


def init_logger(logf, detach, name="hvz-scraper"):

    logger = logging.getLogger(name)
    logger.handlers = []
    logger.setLevel(logging.INFO)

    fh = logging.FileHandler(logf)
    fh.setLevel(logging.INFO)

    formatstr = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    formatter = logging.Formatter(formatstr)

    fh.setFormatter(formatter)
    logger.addHandler(fh)

    if (not detach):
        # create additional logger to console for foreground mode
        ch = logging.StreamHandler(sys.stdout)
        ch.setLevel(logging.INFO)
        ch.setFormatter(formatter)
        logger.addHandler(ch)

    return logger


def do_work(logf, database, detach):
    logger = init_logger(logf, detach)

    while True:

        logger.info("Retrieve latest date from HVZ")

        try:
            r = requests.get(hvz_url,timeout=3)
            r.raise_for_status()

            logger.info("Successfully loaded %s", hvz_url)

            hvzdata = jsmin(r.text, quote_chars="'\"`")
            hvzdata = (hvzdata[hvzdata.find('['):-1])
            hvzdata = hvzdata.replace("'", "\"")
            logger.info("Successfully minified %s", hvz_url)
            jsonhvzdata = json.loads(hvzdata)
            logger.info("Successfully parsed %s", hvz_url)
            gauge_list = []
            for wantedLine in hvz_wanted_sources:
                for line in jsonhvzdata:
                    try:
                        number = int(line[0])
                        if number == hvz_wanted_sources[wantedLine]:
                            line[9] = line[9].replace("MEZ", "CET")
                            line[9] = line[9].replace("MESZ", "CEST")
                            if line[8] != "m³/s":
                                logger.error("Unit of %s should be '%s' but actually is '%s'", wantedLine, "m³/s", line[8])
                            logger.info("Found %s: %s %s at %s", wantedLine, line[7], line[8], line[9])
                            datadate = dateParser.parse(line[9])
                            datatimestamp = time.mktime(datadate.timetuple())
                            gauge_list.append((wantedLine, line[7], datatimestamp))
                            break
                    except ValueError as err:
                        logger.warning("Invalid gauge data: %s", line[0])
                    except Exception as err:
                        logger.warning("Error reading data: %s", err)


            # write to database
            try:
                dbcon = sqlite3.connect(database)
                for gauge in gauge_list:
                    try:
                        dbcon.execute("INSERT INTO gauge_data (gauge_id, flow, timestamp) VALUES (?, ?, ?)", gauge)
                    except sqlite3.IntegrityError as err:
                        logger.info("The value for %s, %s was already inserted", gauge[0], gauge[2])
                        pass
                dbcon.commit()
                dbcon.close()
                logger.info("Data successfully inserted into database")
            except sqlite3.Error as err:
                logger.error("Database error: ",err)

        except requests.exceptions.HTTPError as errh:
            logger.error("Http Error: %s",errh)
        except requests.exceptions.ConnectionError as errc:
            logger.error("Error Connecting: %s",errc)
        except requests.exceptions.Timeout as errt:
            logger.error("Timeout Error: %s",errt)
        except requests.exceptions.RequestException as err:
            logger.error("Ops: Something Else %s",err)
        except Exception as err:
            logger.error("Error: %s", err)
        logger.info("Sleeping for %i secs", sleep_time)
        time.sleep(sleep_time)


def shutdown(signum, frame):  # signum and frame are mandatory
    logger = logging.getLogger('hvz-scraper')

    logger.info("Shutting down due to signal %i.", signum)
    sys.exit(0)


def start_daemon(pidf, logf, database, detach):
    ### This launches the daemon in its context

    logger = logging.getLogger('hvz-scraper')
    logger.info("Starting daemon")

    # if we're running in the forground, keep stdout/stderr as they are
    if (detach):
        out=None
        err=None
    else:
        out=sys.stdout
        err=sys.stderr


    with daemon.DaemonContext(
        working_directory='.',
        umask=0o002,
        pidfile=pidfile.TimeoutPIDLockFile(pidf),
        detach_process=detach,
        stdout=out,
        stderr=err,
        signal_map={
            signal.SIGTERM: shutdown,
            signal.SIGTSTP: shutdown,
            signal.SIGINT: shutdown
        }) as context:
        do_work(logf, database, detach)


if __name__ == '__main__':
    # parse arguments
    parser = argparse.ArgumentParser(description="hvz-scraper")
    parser.add_argument('-p', '--pid-file', default='/var/run/hvz-scraper.pid', help='Pid file')
    parser.add_argument('-l', '--log-file', default='/var/log/hvz-scraper.log', help='Log file')
    parser.add_argument('-d', '--database', default="/var/run/hvz-scraper/gauges.db", help='sqlite3 database to append values to')
    parser.add_argument('-f', '--foreground', action='store_true', help='Keep daemon in foreground')
    args = parser.parse_args()

    # init logger
    init_logger(logf=args.log_file, detach=not args.foreground)

    # finally, start the daemon
    start_daemon(pidf=args.pid_file, logf=args.log_file, database=args.database, detach=(not args.foreground))
