#!/usr/bin/python

#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
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
# import dependency to finish this script
import sys

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

# CLI's menu
def Menu():
    header = f"{bcolors.OKCYAN}[Gawake MENU] ···> Choose an option: {bcolors.ENDC}"
    print('')
    print(header)
    menuInformation = [
        'a\tAdd rule',
        'd\tDelete rule by ID',
        'i\tPrint menu information',
        'p\tPrint database',
        'r\tReset database',
        's\tSchedule wakeup',
        'q\tQuit',
        ''
    ]
    for i in menuInformation:
        print(i)
    # var to keep the loop active
    repeat = True
    while(repeat):
        option = input(f'{bcolors.OKCYAN}[MENU] Enter your choice: {bcolors.ENDC}')
        option = option.lower()
        def Switch(value):
            if (value == 'a'):
                AddRule(returnedCursor, returnedConnection)
                # printing menu again
                print('')
                print(header)
                for i in menuInformation:
                    print(i)
            elif (value == 'd'):
                print('Here is your database: ')
                PrintDB(returnedCursor)
                DeleteRule(returnedCursor, returnedConnection)
            elif (value == 'i'):
                for i in menuInformation:
                    print(i)
            elif (value == 'p'):
                PrintDB(returnedCursor)
            elif (value == 'r'):
                print('')
                reset = input(f"{bcolors.FAIL}{bcolors.BOLD}ATTENTION! DO YOU REALLY WANT TO RESET YOUR DATABASE? (y/n) {bcolors.ENDC}{bcolors.ENDC}")
                reset = reset.lower()
                if(reset == 'y'):
                    os.system('rm database.db')
                    VerifyDB()
                    for i in menuInformation:
                        print(i)
            elif (value == 's'):
                repeat = False
                print('')
                schedule = input(f"{bcolors.WARNING}{bcolors.BOLD}WARNING! Your PC will turn off now, continue? (y/n) {bcolors.ENDC}{bcolors.ENDC}")
                schedule = schedule.lower()
                if(schedule == 'y'):
                    CalculateDay(returnedCursor)
            elif (value == 'q'):
                repeat = False
                # closes database
                CloseDB(returnedConnection)
                # fineshes the py script
                sys.exit()
            else:
                print('Choose a valid option!')
                option = ""
        Switch(option)


def VerifyDB():
    cursor = 0
    # verifying is the database file already exists
    dbValidation = os.path.isfile('database.db') # true or false
    print('[Gawake] ···> Database exists? ', dbValidation)

    # if database doesn't exist, create it
    if (not dbValidation):
        # creating connection to the batabase
        connection = sqlite3.connect('database.db')
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
                path        TEXT
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
                1,
                1,
                0,
                'echo \"An exemple...\"',
                'mem'
            )
        """)
        
        # saving to database
        connection.commit()

        print('[Gawake] ···> Database created and connected!')

    # if database already exists, just connect to it
    else:
        # creating connection to the batabase
        connection = sqlite3.connect('database.db')
        print ('[Gawake] ···> Database connected!')

        # creating a cursor
        cursor = connection.cursor();
    # the variables "cursor" and "connection" are only available on the function "VerifyDB", so to make them visible to others functions, they must be returned
    return cursor, connection;

def CloseDB(connection):
    print('[Gawake] ···> Closing database...')
    connection.close()
    print('[Gawake] ···> Database closed!')

def PrintDB(cursor):
    try:
        # printing values on terminal:
        cursor.execute("""
            SELECT * FROM rules;
        """)
        
        print('')
        print(f"{bcolors.OKCYAN}ID\tName\tTime\tSun\tMon\tTue\tWed\tThu\tFri\tSat\tCommand{bcolors.ENDC}")

        for line in cursor.fetchall():
            length = len(line)
            for x in range(length):
                print(f"{line[x]}\t", end="")
            print('\n')
    except sqlite3.Error as err:
        err = str(err)
        print('')
        print(f"{bcolors.FAIL}Fail! It wasn't possible to print the database. Error: {err}\nPlease try again. If it continues, consider reporting the bug.{bcolors.ENDC}")
        input('Press ENTER to continue ')


def AddRule(cursor, connection):
    # declaring variables
    print('')
    rule_name = ''
    time = ''
    sun = ''
    mon = ''
    tue = ''
    wed = ''
    thu = ''
    fri = ''
    sat = ''
    command = ''
    mode = ''
    # listing modes
    MODES = [
        'standby',
        'mem',
        'suspend',
        'off'
    ]

    # function to validade days input
    def ValidateDayInput(day):
        repeat = True
        message = f"{bcolors.WARNING}Please, insert a valid value: 0 for false and 1 for true...{bcolors.ENDC}"
        while(repeat):
            userInput = input(f"{bcolors.OKCYAN}Insert if the rule will be aplied to {day} (0 or 1): {bcolors.ENDC}")
            length = len(userInput)
            try:
                userInput = int(userInput)
                if(length == 1 and (userInput == 0 or userInput == 1)):
                    repeat = False
                    return userInput
                else:
                    print(message)
            except ValueError:
                    print(message)

    # receiving rule name
    repeat = True
    while(repeat):
        rule_name = input(f"{bcolors.OKCYAN}Insert the rule name: {bcolors.ENDC}")
        if(rule_name == ''):
            print(f"{bcolors.WARNING}Please enter a rule name!{bcolors.ENDC}")
        else:
            repeat = False
    # receiving time
    repeat = True
    message = f"{bcolors.WARNING}Please, insert a valid value, following the format HHMMSS (example: for 10:45:30, it will be 104530).\nTime must be > 000000 and < 240000.{bcolors.ENDC}"
    while(repeat):
        time = input(f"{bcolors.OKCYAN}Insert the time (HHMMSS): {bcolors.ENDC}")
        length = len(time)
        try:
            time = int(time)
            if(length == 6 and (time > 0 and time < 240000)):
                repeat = False
            else:
                print(message)
        except ValueError:
                print(message)
    # receiving days
    sun = ValidateDayInput('SUNDAY')
    mon = ValidateDayInput('MONDAY')
    tue = ValidateDayInput('TUESDAY')
    wed = ValidateDayInput('WEDNESDAY')
    thu = ValidateDayInput('THURSDAY')
    fri = ValidateDayInput('FRIDAY')
    sat = ValidateDayInput('SATURDAY')
    # receiving command
    command = input(f"{bcolors.OKCYAN}Insert a command (OPTIONAL): {bcolors.ENDC}")
    # receiving mode
    repeat = True
    while(repeat):
        print('')
        print(f"{bcolors.OKCYAN}Insert a mode: {bcolors.ENDC}")
        for m in MODES:
            print('\t', m)
        mode = input(f"{bcolors.OKCYAN}···> {bcolors.ENDC}")
        if((mode == 'standby') or (mode == 'mem') or (mode == 'suspend') or (mode == 'off')):
            repeat = False
        else:
            print(f"{bcolors.WARNING}Please, enter a valid value!{bcolors.ENDC}")

    try:
        cursor.execute(f"INSERT INTO rules (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command, mode) VALUES ('{rule_name}', {time}, {sun}, {mon}, {tue}, {wed}, {thu}, {fri}, {sat}, '{command}', '{mode}')")
        connection.commit()
        print('')
        print(f"{bcolors.OKGREEN}Rule added successfully{bcolors.ENDC}")
    except sqlite3.Error as err:
        err = str(err)
        print('')
        print(f"{bcolors.FAIL}Fail! The rule wasn't added. Error: {err}\nPlease try again. If it continues, consider reporting the bug.{bcolors.ENDC}")
        input('Press ENTER to continue ')

def DeleteRule(cursor, connection):
    # get the user input
    userInput = input(f"{bcolors.OKCYAN}Insert the ID to DELETE the rule: {bcolors.ENDC}")
    try:
        userInput = int(userInput)
        # loop to compare the user input with the database ids
        for dbIds in (cursor.execute(f"SELECT id FROM rules").fetchall()):
            dbIds = int(''.join(map(str, dbIds)))
            # if they match, then try to delete
            if(userInput == dbIds):
                # ask user the confirmation
                confirmation = input(f"{bcolors.WARNING}{bcolors.BOLD}WARNING! Are you sure to delete this rule? (y/n) {bcolors.ENDC}{bcolors.ENDC}")
                confirmation = confirmation.lower()
                if(confirmation == 'y'):
                    try:
                        cursor.execute(f"DELETE FROM rules WHERE id = {userInput}")
                        connection.commit()
                        print('')
                        print(f"{bcolors.OKGREEN}Rule deleted successfully{bcolors.ENDC}")
                        print('')
                    except sqlite3.Error as err:
                        err = str(err)
                        print('')
                        print(f"{bcolors.FAIL}Fail! The rule wasn't deleted, make sure you enterd a valid ID!\n. Error: {err}. If the problem isn't an invalid ID, consider reporting this bug.{bcolors.ENDC}")
                        input('Press ENTER to continue ')
                    # breaks the comparing loop
                    break
                else:
                    print(f"{bcolors.WARNING}Returning to menu.{bcolors.ENDC}")
                    print('')

    except:
        print(f"{bcolors.WARNING}Please, enter a valid value!\nReturning to menu.{bcolors.ENDC}")

def CalculateDay(cursor):
        # get the current day, using sqlite
        today = cursor.execute("SELECT strftime('%Y%m%d')").fetchone() #YYYYMMDD
        # converting "today" from string to int
        today = int(''.join(map(str, today)))
        # get the current time, using sqlite
        now = cursor.execute("SELECT strftime('%H%M%S', 'now', 'localtime')").fetchone() # note: the return of SQLite is in UTC, so there's the need of "localtime" #HHMMSS
        # converting "now" from string to int
        now = int(''.join(map(str, now)))
        # get the day of the week
        numberOfDay = cursor.execute("SELECT strftime('%w', 'now', 'localtime')").fetchone() # 0 ~ 6, witch 0 = sunday
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
                print(f"{bcolors.FAIL}[ERROR] Something went wrong with the switch function :/{bcolors.ENDC}")
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
            for x in range(length and lock == False):
                # converting time from string to int
                convertedTime = int(''.join(map(str, times[x])))
                if(now < convertedTime and lock == False):
                    print('')
                    print(f"{bcolors.OKGREEN}[Gawake] ···> Scheduling for today, at {convertedTime} (HHMMSS){bcolors.ENDC}")
                    print('')
                    # activate the lock
                    lock = True
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
                    # converting time from string to int
                    convertedTime = int(''.join(map(str, times[z])))
                    # if the time to be schedule ins't null, and the lock wasn't already activated, realize the schedule
                    if((convertedTime != None) and (lock == False)):
                        # Calculate the exact day and month to printo to the user
                        dayScheduled = int(''.join(map(str, cursor.execute(f"SELECT strftime('%m%d', 'now', 'localtime', '+{plusDay} day');").fetchone())))

                        print('')
                        print(f"{bcolors.OKGREEN}[Gawake] ···> Scheduling for {day}, {dayScheduled}(MMDD) at {convertedTime}(HHMMSS){bcolors.ENDC}")
                        print('')

                        # calculating the wakeup time for the command line, with the format YYYYMMDDhhmmss
                        # to concatenate two integers (YYYYMMDD + hhmmss), it'll use the following syntax: z = int(str(x) + str(y))    ~ and trust me, this syntax was needed!
                        timestamp = int(
                            str(int(''.join(map(str,cursor.execute(f"SELECT strftime('%Y%m%d', 'now', 'localtime', '+{plusDay} day');").fetchone()))))
                            +
                            str(convertedTime)
                        )

                        # activate the lock
                        lock = True
                        # calls the RunCommand function
                        RunCommand(timestamp, 'mem')
                        # breaks this loop and returns the timestamp
                        return timestamp
                # this condition breaks the first loop (with the "y" var)
                if(lock):
                    break

        StepOne()

def RunCommand(timestamp, mode):
    # TODO: add mode!
    CloseDB(returnedConnection)
    os.system(f"echo \"Soon it will be a powerful linux command bro! Wait for it...\"")

# Here is where everything starts!
print('[Gawake] ···> Initializing Gawake...')
# the return from the function "VerifyDB" will be saved on these variables, to be used on other functions
returnedCursor, returnedConnection = VerifyDB()
# first of all, the database is verified, and then call the menu
Menu()

# rtcwake documentation:        https://man7.org/linux/man-pages/man8/rtcwake.8.html
