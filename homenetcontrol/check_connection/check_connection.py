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
if __name__ == '__main__':    
    exit("[!] This is a module import it!")
else:
    from logging import getLogger
    from os import popen
    from configparser import ConfigParser
    from csv import writer as csvwriter
    from time import time
    from datetime import datetime
    from locale import getdefaultlocale
    from pkg_resources import resource_filename, resource_exists
    from codecs import open
    try:
        from ..comm import notif
    except ValueError:
        from comm import notif
    
    # Set language messages to use
    if resource_exists("homenetcontrol", "langs/" + getdefaultlocale()[0][:2] + ".lang"):
        langfile = resource_filename("homenetcontrol", "langs/" + getdefaultlocale()[0][:2] + ".lang")
    else:
        langfile = resource_filename("homenetcontrol", "langs/en.lang")
    lang = ConfigParser()
    lang.readfp(open(langfile, "r", "utf8"))
    
    logger = getLogger("homenetcontrol")

def check_connection():
    ts = time()
    #https://github.com/sivel/speedtest-cli
    a = popen("python "+resource_filename("homenetcontrol", "check_connection/speedtest-cli.py")+" --simple --server 6474").read()
    lines = a.split('\n')
    if "Cannot" in a:
        d = '0'
        u = '0'
    else:
        d = lines[1][10:14]
        u = lines[2][8:12]
    date = datetime.fromtimestamp(ts).strftime('%Y.%m.%d %H:%M')
    out_file = open("/var/local/homenetcontrol/data.csv", 'a')
    writer = csvwriter(out_file)
    writer.writerow((ts,d,u))
    out_file.close()
    logger.info(lang.get("check_connection","msg_log") % (date, d, u))
    if "Cannot" in a:
        notif(lang.get("check_connection","msg_nocon") % date)
    elif (eval(d)<5) or (eval(u)<0.5):
        notif(lang.get("check_connection","msg_notif") % (date, d, u))