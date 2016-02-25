#!/usr/bin/env python2
# -*- coding: utf-8 -*-

#    Home Net Control - Monitorizes devices connected to net and notify
#    according with its type. Also monitorizes the bandwidth.
#    Copyright (C) 2016  bercianor  (https://github.com/bercianor)
#    
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
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

from sys import exit

def main():
    # Check if run as root
    from os import getuid
    if getuid() != 0:
        exit("[!] Please run me as root")

    # Import needed libraries
    from logging import getLogger, INFO, FileHandler, StreamHandler, Formatter  # For logger
    from argparse import ArgumentParser                                         # For argument's parser
    from time import time, sleep                                                # To get actual time
    from datetime import datetime                                               # To get actual time
    from threading import Thread                                                # To manage threads
    from os import path                                                         # For check file exists
    from configparser import ConfigParser                                       # For language
    from locale import getdefaultlocale                                         # For language
    from pkg_resources import resource_filename, resource_exists
    from codecs import open
    
    
    #Import custom modules
    from comm import telegram_bot
    from check_devices import check_devices
    from check_connection import check_connection
    
    # Set language messages to use
    if resource_exists("homenetcontrol", "langs/" + getdefaultlocale()[0][:2] + ".lang"):
        langfile = resource_filename("homenetcontrol", "langs/" + getdefaultlocale()[0][:2] + ".lang")
    else:
        langfile = resource_filename("homenetcontrol", "langs/en.lang")
    lang = ConfigParser()
    lang.readfp(open(langfile, "r", "utf8"))
    
    # Set argument's parser
    parser = ArgumentParser(description=lang.get("main","msg_desc"))
    parser.add_argument("-l", "--log", action="store_true", help=lang.get("main","msg_log"))
    args = parser.parse_args()
    
    # Set logger
    logger = getLogger("homenetcontrol")
    logger.setLevel(INFO)
    if args.log:
        lh = FileHandler("/var/log/homenetcontrol.log")
    else:
        lh = StreamHandler()
    lh.setLevel(INFO)
    formatter = Formatter("%(asctime)s - %(name)s - %(funcName)s - %(levelname)s - %(message)s", "%Y-%m-%d %H:%M:%S")
    lh.setFormatter(formatter)
    logger.addHandler(lh)
    
    # Create threads
    tb = Thread(target=telegram_bot, name='TelegramBot')
    cd = Thread(target=check_devices, name='CheckDevices')
    cc = Thread(target=check_connection, name='CheckConnection')
    
    check_devices_time = 0
    while True:
        try:
            if not(tb.is_alive()):
                tb = Thread(target=telegram_bot, name='TelegramBot')
                tb.start()
            if not(cd.is_alive()) and ((time()) >= check_devices_time+90): #CADA MINUTO Y MEDIO
                check_devices_time = time()
                cd = Thread(target=check_devices, name='CheckDevices')
                cd.start()
            if not(cc.is_alive()) and (datetime.today().minute == 0) and (datetime.today().second <= 30): #CADA HORA EN PUNTO
                cc = Thread(target=check_connection, name='CheckConnection')
                cc.start()
            sleep(1)
        except KeyboardInterrupt:
            break

if __name__ == '__main__':
    main()