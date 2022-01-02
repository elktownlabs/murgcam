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
from daemon import pidfile
import os

hvz_url = "https://www.hvz.baden-wuerttemberg.de/js/hvz_peg_stmn.js"
hvz_wanted_sources = {
  "Schwarzenberg": 79,
  "Schönmünzach": 170,
}

debug_p = False

def do_work(logf):
    ### This does the "work" of the daemon

    logger = logging.getLogger('hvz-scraper')
    logger.setLevel(logging.INFO)

    fh = logging.FileHandler(logf)
    fh.setLevel(logging.INFO)

    formatstr = '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    formatter = logging.Formatter(formatstr)

    fh.setFormatter(formatter)

    logger.addHandler(fh)

    while True:


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

            for wantedLine in hvz_wanted_sources:
                for line in jsonhvzdata:
                    try:
                        number = int(line[0])
                        if number == hvz_wanted_sources[wantedLine]:
                            line[9] = line[9].replace("MEZ", "CET")
                            line[9] = line[9].replace("MESZ", "CEST")
                            logger.info("Found %s: %s %s at %s", wantedLine, line[7], line[8], line[9])
                            datadate = dateParser.parse(line[9])
                            datatimestap = time.mktime(datadate.timetuple())
                            break
                    except ValueError as err:
                        logger.warning("Invalid gauge data: %s", line[0])
                    except Exception as err:
                        logger.warning("Error reading data: %s", err)

        except requests.exceptions.HTTPError as errh:
            logger.error("Http Error: %s",errh)
        except requests.exceptions.ConnectionError as errc:
            logger.error("Error Connecting: %s",errc)
        except requests.exceptions.Timeout as errt:
            logger.error("Timeout Error: %s",errt)
        except requests.exceptions.RequestException as err:
            logger.error("OOps: Something Else %s",err)
        except Exception as err:
            logger.error("Error: %s", err)




        logger.info("Sleeping for 10mins")
        time.sleep(60*10)


def start_daemon(pidf, logf):
    ### This launches the daemon in its context

    global debug_p

    if debug_p:
        print("eg_daemon: entered run()")
        print("eg_daemon: pidf = {}    logf = {}".format(pidf, logf))
        print("eg_daemon: about to start daemonization")

    ### XXX pidfile is a context
    with daemon.DaemonContext(
        working_directory='.',
        umask=0o002,
        pidfile=pidfile.TimeoutPIDLockFile(pidf),
        ) as context:
        do_work(logf)




if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="hvz-scraper")
    parser.add_argument('-p', '--pid-file', default='/var/run/hvz-scraper.pid')
    parser.add_argument('-l', '--log-file', default='/var/log/hvz-scraper.log')
    args = parser.parse_args()

    start_daemon(pidf=args.pid_file, logf=args.log_file)
