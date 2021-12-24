#!/usr/bin/env python3

#    Gawake. A Linux software to make your PC wake up on a scheduled time. It makes the rtcwake command easier.
#    Copyright (C) 2021 Kelvin Novais
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, considering ONLY the version 3 of the License.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <https://www.gnu.org/licenses/>





# importing dependency for database management
import sqlite3
# importing dependecy to run terminal commands
import os
# importing dependency to finish this script
import sys
# importing dependency to handle with Ctrl + C (finish the script)
import signal
# importing dependency to get Linux user
import getpass

# class that makes possible colored prints
class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
# link to issues page
ISSUES = 'https://github.com/KelvinNovais/Gawake/issues'

def VerifyDB():
    cursor = 0
    # get the linux user and generate the path to the database file
    user = getpass.getuser()
    path = f"/home/{user}/.gawake/database.db"
    # verifying if the database file already exists
    dbValidation = os.path.isfile(path) # true or false
    print('[Gawake] ···> Database exists? ', dbValidation)

    # if database doesn't exist, create it
    if (not dbValidation):
        # case the directory doesn't exists, generate it
        dirValidation = os.path.isdir(f"/home/{user}/.gawake/")
        if(not dirValidation):
            os.system(f"mkdir /home/{user}/.gawake/")
        try:
            # creating connection to the batabase
            connection = sqlite3.connect(path)
            print ('[Gawake] ···> Seting up database...')
            # creating a cursor
            cursor = connection.cursor()

            # create tables
            cursor.execute("""
                CREATE TABLE rules (
                    id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                    rule_name   TEXT NOT NULL,
                    time        TEXT NOT NULL,

                    sun         INTEGER NOT NULL,
                    mon         INTEGER NOT NULL,
                    tue         INTEGER NOT NULL,
                    wed         INTEGER NOT NULL,
                    thu         INTEGER NOT NULL,
                    fri         INTEGER NOT NULL,
                    sat         INTEGER NOT NULL,

                    command     TEXT,
                    mode        TEXT NOT NULL
                );    
            """)

            cursor.execute("""
                CREATE TABLE config (
                    id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
                    options     TEXT,
                    db_time     TEXT
                );    
            """)

            # creating first register
            cursor.execute("""
                INSERT INTO rules (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command, mode) VALUES (
                    'Example',
                    '100000',
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    0,
                    'echo \"An exemple...\"',
                    'mem'
                )
            """)

            cursor.execute("INSERT INTO config (options, db_time) VALUES ('-a', 'localtime')")

            # saving to database
            connection.commit()

            # get db_time
            db_time = cursor.execute("SELECT db_time FROM config").fetchone()

            print('[Gawake] ···> Database created and connected!')

        except sqlite3.Error as err:
            print(f"{bcolors.FAIL}It wasn't possible to create the database. Error: {err}\nIf it continues, consider reporting the bug ({ISSUES}).{bcolors.ENDC}")
            input('Press ENTER to continue ')
        
    # if database already exists, just connect to it
    else:
        try:
            # creating connection to the batabase
            connection = sqlite3.connect(path)
            print ('[Gawake] ···> Database connected!')

            # creating a cursor
            cursor = connection.cursor();
            # get db_time
            db_time = cursor.execute("SELECT db_time FROM config").fetchone()
        except sqlite3.Error as err:
            print(f"{bcolors.FAIL}It wasn't possible to create the database. Error: {err}\nIf it continues, consider reporting the bug ({ISSUES}).{bcolors.ENDC}")
            input('Press ENTER to continue ')

    # the variables "cursor" and "connection" are only available on the function "VerifyDB", so to make them visible to others functions, they must be returned
    return cursor, connection, db_time[0]

def ConnectDB():
    # these variables must be global, because they will be requested on other functions
    global returnedCursor, returnedConnection, db_time
    # the return from the function "VerifyDB" will be saved on the previous variables
    returnedCursor, returnedConnection, db_time = VerifyDB()


def CloseDB(connection):
    print('[Gawake] ···> Closing database...')
    connection.close()
    print('[Gawake] ···> Database closed!')

def CalculateDay(cursor, db_time):
        # get the current day, using sqlite
        today = cursor.execute("SELECT strftime('%Y%m%d')").fetchone() #YYYYMMDD
        # converting "today" from string to int
        today = int(''.join(map(str, today)))
        # get the current time, using sqlite
        now = cursor.execute(f"SELECT strftime('%H%M%S', 'now', '{db_time}')").fetchone() 
        # notice: the return of SQLite is in UTC, so there's the need of "localtime"(on the var "db_time"); it is the pattern of the Gawake, but can be cahnged; return in format HHMMSS
        # converting "now" from string to int
        now = int(''.join(map(str, now)))
        # get the day of the week
        numberOfDay = cursor.execute(f"SELECT strftime('%w', 'now', '{db_time}')").fetchone() # 0 ~ 6, witch 0 = sunday
        # get the time(s) that the PC should turn on, sorted ascending; "?" = day of the week that contains a time scheduled
        # converting "numberOfDay" from string to int
        numberOfDay = int(''.join(map(str, numberOfDay)))
        
        # a switch function to insert a value into "stringDay", according to the day of the week
        def Switch(value):
            if (value == 0):
                day = 'sun'
            elif (value == 1):
                day = 'mon'
            elif (value == 2):
                day = 'tue'
            elif (value == 3):
                day = 'wed'
            elif (value == 4):
                day = 'thu'
            elif (value == 5):
                day = 'fri'
            elif (value == 6):
                day = 'sat'
            elif (value == 7):
                day = 'sun'
            elif (value == 8):
                day = 'mon'
            elif (value == 9):
                day = 'tue'
            elif (value == 10):
                day = 'wed'
            elif (value == 11):
                day = 'thu'
            elif (value == 12):
                day = 'fri'
            elif (value == 13):
                day = 'sat'
            else:
                print('')
                print(f"{bcolors.FAIL}[ERROR] Something went wrong with the switch function :/\nPlease consider reporting this error ({ISSUES}).{bcolors.ENDC}")
                input('Press ENTER to continue ')
                sys.exit()
            return day
        
        # the "day" var will replace the "?" on SQLite command
        day = Switch(numberOfDay)

        # this first step tries to schedule for today
        def StepOne():
            print('[Gawake] ···> Trying to schedule for today...')
            times = cursor.execute(f"SELECT time FROM rules WHERE {day} = 1 ORDER BY Time Asc").fetchall()
            # get the "times" length to make the loop logic (notice that "times" is a listt)
            length = len(times)
            # stablishing a lock to avoid scheduling to all greater times and days
            lock = False
            # loop that compare all "times" in the rule with the current time
            for x in range(length):
                # converting time from string to int; it's needed to compare with other numbers
                intTime = int(''.join(map(str, times[x])))
                # however, we need a string version of the time to preserve the leading zeros
                # so it's converted again to string, and with the function zfill(), a digit with 6 numbers is generated
                strTime = str(intTime).zfill(6)
                if(now < intTime and lock == False):
                    print(f"{bcolors.OKGREEN}[Gawake] ···> Scheduling for today, at {strTime} (HHMMSS){bcolors.ENDC}")
                    # activate the lock
                    lock = True
                    # generates the timestamp
                    timestamp = f"{today}{strTime}" #YYYYMMDDhhmmss
                    # get the mode
                    mode = cursor.execute(f"SELECT mode FROM rules WHERE {day} = 1 AND time = '{strTime}'").fetchone()
                    # get command
                    prescript = cursor.execute(f"SELECT command FROM rules WHERE {day} = 1 AND time = '{strTime}'").fetchone()
                    # calls the function to run the command
                    RunCommand(timestamp, mode[0], returnedCursor, prescript[0])
                    # breaks this loop
                    break
                # this condition breaks the first loop (with the "x" var)
                if(lock):
                    break

            # if the scheduling wasn't solved previously, call the step two
            if(lock == False):
                StepTwo()

        # if it wasn't possible to schedule to the current day, try the next one:
        def StepTwo():
            print('[Gawake] ···> Any time matched. Trying to schedule for tomorrow or later...')
            # stablishing a lock to avoid scheduling to all greater times and days
            lock = False
            # loop to add one more day from today
            for y in range(7):
                plusDay = (y + 1)
                day = Switch(numberOfDay + (y + 1))
                times = cursor.execute(f"SELECT time FROM rules WHERE {day} = 1 ORDER BY Time Asc").fetchall()
                # get the "times" length to make the loop logic (notice that "times" is a listt)
                length = len(times)
                # loop that compare all "times" in the rule with the current time
                for z in range(length):
                    # converting time from string to int; it's needed to compare with other numbers
                    intTime = int(''.join(map(str, times[z])))
                    # however, we need a string version of the time to preserve the leading zeros
                    # so it's converted again to string, and with the function zfill(), a digit with 6 numbers is generated
                    strTime = str(intTime).zfill(6)
                    # if the time to be schedule ins't null, and the lock wasn't already activated, realize the schedule
                    if((intTime != None) and (lock == False)):
                        # calculate the exact day and month to printo to the user
                        dayScheduled = int(''.join(map(str, cursor.execute(f"SELECT strftime('%m%d', 'now', '{db_time}', '+{plusDay} day');").fetchone())))
                        # get the mode of the rule, notice that the day must be true (contain a rule) and the time must match (it's not possible to have two rules with the same time and day)
                        # notice that in "mode" and "prescript" the time MUST be passed with '', otherwise sqlite will consider a number and remove the leading zeros!
                        mode = cursor.execute(f"SELECT mode FROM rules WHERE {day} = 1 AND time = '{strTime}'").fetchone()
                        # get command
                        prescript = cursor.execute(f"SELECT command FROM rules WHERE {day} = 1 AND time = '{strTime}'").fetchone()
                        print(f"{bcolors.OKGREEN}[Gawake] ···> Scheduling for {day}, {dayScheduled}(MMDD) at {strTime}(HHMMSS){bcolors.ENDC}")

                        # calculating the wakeup time for the command line, with the format YYYYMMDDhhmmss
                        # to concatenate the date with the time, the following sintax is needed
                        # the return from cursor is a list, so the "int(''.join(map(str, var)))" removes all unused characters; after, it converts from int to str
                        timestamp = (
                            str(int(''.join(map(str,cursor.execute(f"SELECT strftime('%Y%m%d', 'now', '{db_time}', '+{plusDay} day');").fetchone()))))
                            +
                            strTime
                        )

                        # activate the lock
                        lock = True
                        # calls the RunCommand function
                        RunCommand(timestamp, mode[0], returnedCursor, prescript[0])
                # this condition breaks the first loop (with the "y" var)
                if(lock):
                    break

        StepOne()

def RunCommand(timestamp, mode, cursor, prescript):
    # get the rtwake options
    options = cursor.execute("SELECT options FROM config").fetchone()
    # generate the command
    command = f"sudo rtcwake --date {timestamp} {options[0]} -m {mode}"
    # close the database
    CloseDB(returnedConnection)
    # run prescript
    if(prescript != ''):
        print(f"{bcolors.WARNING}Running your command: {prescript}{bcolors.ENDC}")
        os.system(prescript)
    print(f"{bcolors.WARNING}Running rtcwake: {command}{bcolors.ENDC}")
    # run the rtcwake command
    os.system(command)
    # close the Gawake script
    sys.exit()

def Close(signum, frame):
    print('\n[Gawake] ···> Aborting...')
    CloseDB(returnedCursor)
    sys.exit()




############## STARTING ##############
# Here is where everything starts!
# the following line receives user's Ctr + C, to close the script
signal.signal(signal.SIGINT, Close)
print('[Gawake] ···> Initializing Gawake...')
# first of all,the database is verified and connected; then the menu is called, wich give access to all other functions
ConnectDB()
CalculateDay(returnedCursor, db_time)
print(f"{bcolors.FAIL}{bcolors.BOLD}FAIL! Couldn\'t schedule, make sure there is at least one rule on the database.{bcolors.ENDC}{bcolors.ENDC}")
input('Press ENTER to continue ')
#####################################
