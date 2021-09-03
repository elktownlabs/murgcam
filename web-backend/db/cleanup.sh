#!/bin/bash

# current unix timestamp
CURRENT_TIME=`date +%s`

# sql command to run
COMMAND="DELETE FROM active_logins WHERE expiration < ${CURRENT_TIME}"

echo "About to run command '${COMMAND}' on database app.db"
sqlite3 ./app.db "${COMMAND}"
echo "Sqlite3 returned with result $?"
