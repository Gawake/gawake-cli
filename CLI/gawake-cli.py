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

# CLI's menu
def Menu():
    menuInformation = [
        f"\n{bcolors.OKCYAN}[Gawake MENU] ···> Choose an option: {bcolors.ENDC}",
        f"a\t{bcolors.BOLD}A{bcolors.ENDC}dd rule",
        f"c\tAdvanced {bcolors.BOLD}C{bcolors.ENDC}onfig",
        f"d\t{bcolors.BOLD}D{bcolors.ENDC}elete rule by ID",
        f"i\t{bcolors.BOLD}I{bcolors.ENDC}nformation about Gawake",
        f"p\t{bcolors.BOLD}P{bcolors.ENDC}rint database",
        f"r\t{bcolors.BOLD}R{bcolors.ENDC}eset database",
        f"s\t{bcolors.BOLD}S{bcolors.ENDC}chedule wakeup",
        f"q\t{bcolors.BOLD}Q{bcolors.ENDC}uit"
    ]
    # var to keep the loop active
    repeat = True
    menu = True
    while(repeat):
        if(menu):
            for i in menuInformation:
                print(i)
        option = input(f'{bcolors.OKCYAN}[MENU] Enter your choice: {bcolors.ENDC}')
        option = option.lower()
        def Switch(value):
            repeat = True
            menu = True
            if (value == 'a'):
                AddRule(returnedCursor, returnedConnection)
                return repeat, menu
            elif (value == 'c'):
                Config(returnedCursor, returnedConnection)
                return repeat, menu
            elif (value == 'd'):
                print('Here is your database: ')
                PrintDB(returnedCursor)
                DeleteRule(returnedCursor, returnedConnection)
                return repeat, menu
            elif (value == 'i'):
                print('Gawake page: https://github.com/KelvinNovais/Gawake')
                print('\nGawake Copyright (C) 2021 Kelvin Novais\nThis program comes with ABSOLUTELY NO WARRANTY; for details type "show w".\nThis is free software, and you are welcome to redistribute it\nunder certain conditions; type "show c" for details.\n')
                answer = input('Type an option or ENTER to quit: ').lower()
                if(answer == 'show w'):
                    print("""
                        THERE IS NO WARRANTY FOR THE PROGRAM, TO THE EXTENT PERMITTED BY
                        APPLICABLE LAW.  EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT
                        HOLDERS AND/OR OTHER PARTIES PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY
                        OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO,
                        THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
                        PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM
                        IS WITH YOU.  SHOULD THE PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF
                        ALL NECESSARY SERVICING, REPAIR OR CORRECTION.
                    """)
                elif(answer == 'show c'):
                    print("Please, see the whole GNU General Public License version 3 on the file \"LICENSE\" together the original script. You also can find the lisense at <https://www.gnu.org/licenses/>")
                else:
                    print('Returning to menu')
                return repeat, menu
            elif (value == 'p'):
                PrintDB(returnedCursor)
                menu = False
                repeat = True
                return repeat, menu
            elif (value == 'r'):
                reset = input(f"{bcolors.FAIL}{bcolors.BOLD}ATTENTION! DO YOU REALLY WANT TO RESET YOUR DATABASE? (y/n) {bcolors.ENDC}{bcolors.ENDC}")
                reset = reset.lower()
                if(reset == 'y'):
                    CloseDB(returnedConnection)
                    # get user and path
                    user = getpass.getuser()
                    path = f"/home/{user}/.gawake/database.db"
                    # remove db file
                    os.system(f"rm {path}")
                    print('[Gawake] ···> "database.db" removed successfully...')
                    ConnectDB()
                return repeat, menu
            elif (value == 's'):
                # aks user confirmation
                schedule = input(f"{bcolors.WARNING}{bcolors.BOLD}WARNING! Your PC will turn off now, continue? (y/n) {bcolors.ENDC}{bcolors.ENDC}").lower()
                if(schedule == 'y'):
                    CalculateDay(returnedCursor, db_time)
                else:
                    print('Returning to menu')
                    return repeat, menu
                # if the user says "yes", and the function "CalculateDay" can't schedule because there isn't any rule, the script will return here
                print(f"{bcolors.FAIL}{bcolors.BOLD}FAIL! Couldn\'t schedule, make sure there is at least one rule on the database.{bcolors.ENDC}{bcolors.ENDC}")
                input('Press ENTER to continue ')
                return repeat, menu
            elif (value == 'q'):
                repeat = False
                # closes database
                CloseDB(returnedConnection)
                # fineshes the py script
                sys.exit()
                # on any case, it's here to avoid repeating
                return repeat, menu
            else:
                print(f"{bcolors.WARNING}Choose a valid option!{bcolors.ENDC}")
                option = ''
                menu = False
                return repeat, menu
        repeat, menu = Switch(option)

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

def PrintDB(cursor):
    try:
        # printing values on terminal:
        cursor.execute("""
            SELECT * FROM rules;
        """)

        print('')
        # specifies the columns width for the .format() function
        tab = "{:<3} {:<20} {:^7} {:^4} {:^4} {:^4} {:^4} {:^4} {:^4} {:^4} {:^40} {:<10}"
        # here is the header
        print(tab.format('ID','Name','Time','Sun','Mon','Tue','Wed','Thu','Fri','Sat','Command','Mode'))
        # loop to alternate between the lines, and print the columns inside them
        for column in cursor.fetchall():
            print(tab.format(f"{column[0]}", f"{column[1]}", f"{column[2]}", f"{column[3]}", f"{column[4]}", f"{column[5]}", f"{column[6]}", f"{column[7]}", f"{column[8]}", f"{column[9]}", f"{column[10]}", f"{column[11]}"))
        print('*Notice that the first rule is an example, and it is inert.\n')
    except sqlite3.Error as err:
        err = str(err)
        print(f"{bcolors.FAIL}Fail! It wasn't possible to print the database. Error: {err}\nPlease try again or reset the database. If it continues, consider reporting the bug ({ISSUES}).{bcolors.ENDC}")
        input('Press ENTER to continue ')

def AddRule(cursor, connection):
    # declaring variables
    rule_name = ''
    time = ''
    command = ''
    mode = ''
    # list with all the possibilities for the days dict
    keys = [
        'sun',
        'mon',
        'tue',
        'wed',
        'thu',
        'fri',
        'sat'
    ]
    # listing modes
    MODES = [
        'standby',
        'freeze',
        'mem',
        'disk',
        'off',
        'no',
        'on',
        'disable'
    ]

    # function to validade days input
    def ValidateDayInput(day):
        repeat = True
        message = f"{bcolors.WARNING}Please, insert a valid value: 0 for false and 1 for true...{bcolors.ENDC}"
        while(repeat):
            userInput = input(f"{bcolors.OKCYAN}Insert if the rule will be aplied to {day}\t(0 or 1): {bcolors.ENDC}")
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
    message = f"{bcolors.WARNING}Please, insert a valid value, following the format HHMMSS (example: for 10:45:30, it will be 104530).\nTime must be > 000000 and < 240000; minutes anh hours must be < 60.{bcolors.ENDC}"
    while(repeat):
        time = input(f"{bcolors.OKCYAN}Insert the time (HHMMSS): {bcolors.ENDC}")
        # get length for the loop
        length = len(time)
        # the try/except verifies if the user typed characters
        try:
            # converting to int
            timeInt = int(time)
            # first, make sure it's within the corect interval of 24 hours
            if(length == 6 and (timeInt > 0 and timeInt < 240000)):
                # second, verify if the minutes are less than 59
                # the following sintaxes use Python substring, converted to int and analysed
                if(int(time[2:4]) < 60):
                    # finally, check if the seconds are less than 59
                    if(int(time[4:]) < 60):
                        repeat = False

            # if the user input wasn't approved, then print the message
            if(repeat == True): print(message)
        except ValueError:
                print(message)
    # receiving days
    days = {
            "sun": ValidateDayInput('SUNDAY'),
            "mon": ValidateDayInput('MONDAY'),
            "tue": ValidateDayInput('TUESDAY'),
            "wed": ValidateDayInput('WEDNESDAY'),
            "thu": ValidateDayInput('THURSDAY'),
            "fri": ValidateDayInput('FRIDAY'),
            "sat": ValidateDayInput('SATURDAY')
    }
    # receiving command
    command = input(f"{bcolors.OKCYAN}Insert a command to run before shuting down(OPTIONAL): {bcolors.ENDC}")
    # receiving mode
    repeat = True
    while(repeat):
        print(f"{bcolors.OKCYAN}Insert a mode: {bcolors.ENDC}")
        for m in MODES:
            print('\t', m)
        mode = input(f"{bcolors.OKCYAN}···> {bcolors.ENDC}")
        mode = mode.lower()
        if((mode == 'standby') or (mode == 'off') or (mode == 'freeze') or (mode == 'mem') or (mode == 'disk') or (mode == 'no') or (mode == 'on') or (mode == 'disable') or (mode == 'show')):
            repeat = False
        else:
            print(f"{bcolors.WARNING}Please, enter a valid value!{bcolors.ENDC}")
    # validate if ther will be two rules with the same day and time
    lock = False
    for v in range(7):
        # select one key from the list; it correspond to one day of the week
        key = keys[v]
        # if the day was set as 1 (true) by the user, then compare it with the database, to make sure there won't be conflicts
        if(days[key] == 1):
            # select all times from the day in question
            dbTime = cursor.execute(f"SELECT time FROM rules WHERE {key} = 1").fetchall()
            # get the length to make the loop
            length = len(dbTime)
            for c in range(length):
                # if there's a time on the database, that matches with user's input time, the block (lock) the rule addition
                if(int(''.join(map(str, dbTime[c]))) == time):
                    print(f"{bcolors.WARNING}There's already a rule on {key} with the time {time}, please try again with other values!{bcolors.ENDC}")
                    lock = True

    if(not lock):
        try:
            # to avoid that sqlite remove the zeros on the time, this var must have '' to consider it a string!         '{time}'
            cursor.execute(f"INSERT INTO rules (rule_name, time, sun, mon, tue, wed, thu, fri, sat, command, mode) VALUES ('{rule_name}', '{time}', {days['sun']}, {days['mon']}, {days['tue']}, {days['wed']}, {days['thu']}, {days['fri']}, {days['sat']}, '{command}', '{mode}')")
            connection.commit()
            print(f"{bcolors.OKGREEN}Rule added successfully{bcolors.ENDC}")
        except sqlite3.Error as err:
            err = str(err)
            print(f"{bcolors.FAIL}Fail! The rule wasn't added. Error: {err}\nPlease try again. If it continues, consider reporting the bug ({ISSUES}).{bcolors.ENDC}")
            input('Press ENTER to continue ')

def DeleteRule(cursor, connection):
    # get the user input
    userInput = input(f"{bcolors.OKCYAN}Insert the ID to DELETE the rule: {bcolors.ENDC}")
    try:
        userInput = int(userInput)
        # get the ids from the database
        dbIds = cursor.execute(f"SELECT id FROM rules").fetchall()
        # var to make possible the loop
        length = len(dbIds)
        # validation to print "invalid id" message
        success = False
        # loop to compare the user input with the database ids
        for i in range(length):
            dbId = int(''.join(map(str, dbIds[i])))
            # if they match, then try to delete
            if(userInput == dbId):
                # ask user the confirmation
                confirmation = input(f"{bcolors.WARNING}{bcolors.BOLD}WARNING! Are you sure to delete this rule? (y/n) {bcolors.ENDC}{bcolors.ENDC}")
                confirmation = confirmation.lower()
                if(confirmation == 'y'):
                    try:
                        cursor.execute(f"DELETE FROM rules WHERE id = {userInput}")
                        connection.commit()
                        success = True
                        print(f"{bcolors.OKGREEN}Rule deleted successfully{bcolors.ENDC}")
                    except sqlite3.Error as err:
                        err = str(err)
                        print(f"{bcolors.FAIL}Fail! The rule wasn't deleted, make sure you enterd a valid ID!.\nError: {err}. If the problem isn't an invalid ID, consider reporting this bug ({ISSUES}).{bcolors.ENDC}")
                        input('Press ENTER to continue ')
                    # breaks the comparing loop
                    break
                else:
                    print(f"{bcolors.WARNING}Returning to menu.{bcolors.ENDC}")
        # at the end of the previous for loop, if any id matched and the user didn't canceld the deletion, print the message
        if((success == False) and (not confirmation != 'y')):
            print(f"{bcolors.WARNING}Please, enter a valid value!\nReturning to menu.{bcolors.ENDC}")
    except:
        print(f"{bcolors.WARNING}Please, enter a valid value!\nReturning to menu.{bcolors.ENDC}")

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

def Config(cursor, connection):
    print(f"{bcolors.WARNING}[Config MENU] ···> This menu allows to to change the following:{bcolors.ENDC}")
    print('t\tSwitch between utc and localtime\no\tChoose rtcwake options')
    print('\nNote: if you have problems with Gawake time, you probably must change the default "localtime" to "UTC", on the first option.')
    option = input(f"{bcolors.WARNING}Enter your choice: {bcolors.ENDC}").lower()
    if(option == 't'):
        time = cursor.execute("SELECT db_time FROM config").fetchone()
        print(f"Currently, the time is: {time[0]}")
        try:
            if(time[0] == 'localtime'):
                confirmation = input('Switch to utc? (y/n) ').lower()
                if(confirmation == 'y'):
                    cursor.execute(f"UPDATE config SET db_time = 'utc'")
                    connection.commit()
                    print(f"{bcolors.OKGREEN}Done!{bcolors.ENDC}")
            else:
                confirmation = input('Switch to localtime? (y/n) ').lower()
                if(confirmation == 'y'):
                    cursor.execute(f"UPDATE config SET db_time = 'localtime'")
                    connection.commit()
                    print(f"{bcolors.OKGREEN}Done!{bcolors.ENDC}")
        except sqlite3.Error as err:
            print(f"\n{bcolors.FAIL}It wasn't possible to update the options. If it continues, please consider reporting this bug ({ISSUES}).\nError: {err}{bcolors.ENDC}")
            input('Press ENTER to continue ')

    elif(option == 'o'):
        print(f"{bcolors.WARNING}ATTENTION!{bcolors.ENDC}")
        print('Here you can manage a string with all the options you want to add to the rtcwake command. Please, make sure you won\'t break the command!')
        print('Before anything, it\'s recommended that you read the documentation: https://man7.org/linux/man-pages/man8/rtcwake.8.html')
        print('On Gawake, the rtcwake command follows this pattern: \"sudo rtcwake --date [YYYYMMDDhhmmss]\", and your options will be added at the end.\n')
        options = cursor.execute("SELECT options FROM config").fetchone()
        print(f"Currently, the option string is: {options[0]}")
        confirmation = input(f"Do you really want to update the options? (y/n) ").lower()
        if(confirmation == 'y'):
            userInput = input(f"{bcolors.WARNING}Insert the new options: {bcolors.ENDC}")
            try:
                cursor.execute(f"UPDATE config SET options = '{userInput}'")
                connection.commit()
                options = cursor.execute("SELECT options FROM config").fetchone()
                print(f"{bcolors.OKGREEN}Done! Updated to \"{options[0]}\"{bcolors.ENDC}")
            except sqlite3.Error as err:
                print(f"\n{bcolors.FAIL}It wasn't possible to update the options. If it continues, please consider reporting this bug ({ISSUES}).\nError: {err}{bcolors.ENDC}")
                input('Press ENTER to continue ')
        else:
            print('Nothing will be changed. Returning to menu.')
    else:
        print('Choose a valid option. Returnig to menu')

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
Menu()
#####################################







# rtcwake documentation:        https://man7.org/linux/man-pages/man8/rtcwake.8.html
