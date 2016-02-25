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
    from sqlite3 import connect
    from logging import getLogger
    from ConfigParser import ConfigParser
    from locale import getdefaultlocale
    from pkg_resources import resource_filename, resource_exists
    from codecs import open
    try:
        from ..comm import notif
    except ValueError:
        from comm import notif
    try:
        from ..arping import arping, arpingerror
    except ValueError:
        from arping import arping, arpingerror
    
    # Set language messages to use
    if resource_exists("homenetcontrol", "langs/" + getdefaultlocale()[0][:2] + ".lang"):
        langfile = resource_filename("homenetcontrol", "langs/" + getdefaultlocale()[0][:2] + ".lang")
    else:
        langfile = resource_filename("homenetcontrol", "langs/en.lang")
    lang = ConfigParser()
    lang.readfp(open(langfile, "r", "utf8"))
    
    # Get info from config file
    if resource_exists("homenetcontrol", "check_devices/config.ini"):
        configfile = resource_filename("homenetcontrol", "check_devices/config.ini")
    else:
        configfile = "/etc/homenetcontrol/config.ini"
    config = ConfigParser()
    config.optionxform=str
    config.read(configfile)
    
    # Set db and logger
    logger = getLogger("homenetcontrol")
    db = "/var/local/homenetcontrol/check_devices.sqlite3"
    

def devices_status():
    conn = connect(db)
    c = conn.cursor()
    c.execute('SELECT user, present FROM users')
    for user in c.fetchall():
        if user[1]:
            notif(lang.get("check_devices","msg_inhouse") % user[0])
        else:
            notif(lang.get("check_devices","msg_outhouse") % user[0])
    conn.close()

def check_devices():
    allowed_devices = {}
    options = config.options("allowed_devices")
    for option in options:
        allowed_devices[option] = config.get("allowed_devices", option)
    users = {}
    options = config.options("watch_users")
    for option in options:
        users[allowed_devices[config.get("watch_users", option)]] = option
    allDevices = []
    device = {}
    conn = connect(db)
    c = conn.cursor()
    c.execute('CREATE TABLE IF NOT EXISTS not_allowed (mac TEXT NOT NULL, advised INTEGER NOT NULL, number INTEGER NOT NULL)')
    c.execute('CREATE TABLE IF NOT EXISTS users (user TEXT NOT NULL, present INTEGER NOT NULL)')
    if not(users=={}):
        for user in users.values():
            t = (user,)
            c.execute('SELECT * FROM users WHERE user=?', t)
            if c.fetchone() == None:
                c.execute("INSERT INTO users VALUES (?,0)", t)
                conn.commit()
    conn.close()
    try:
        # Get devices connected to net
        allDevices = arping(config.get("defaults","NET"),10,1)
        # Open db connection
        conn = connect(db)
        c = conn.cursor()
        # Check if any watched user is connected
        if not(users=={}):
            for user_mac in users.keys():
                found = 0
                for device in allDevices:
                    if device["MAC"] == user_mac:
                        found = 1
                        break
                t = (users[user_mac],)
                if found:
                    c.execute('SELECT present FROM users WHERE user=?', t)
                    if not(c.fetchone()[0]):
                        logger.info(lang.get("check_devices","msg_arrivehouse") % users[user_mac])
                        c.execute("UPDATE users SET present=1 WHERE user=?", t)
                        conn.commit()
                        notif(lang.get("check_devices","msg_arrivehouse") % users[user_mac])
                else:
                    c.execute('SELECT present FROM users WHERE user=?', t)
                    if c.fetchone()[0]:
                        logger.info(lang.get("check_devices","msg_lefthouse") % users[user_mac])
                        c.execute("UPDATE users SET present=0 WHERE user=?", t)
                        conn.commit()
                        notif(lang.get("check_devices","msg_lefthouse") % users[user_mac])
        # Check if unallowed device is connected
        if not(allowed_devices=={}):
            for device in allDevices:
                if not(device.get('MAC') in allowed_devices.values()) and (device.get('MAC') != '<incomplete>'):
                    t = (device.get('MAC'),)
                    c.execute('SELECT * FROM not_allowed WHERE mac=?', t)
                    dbdevice = c.fetchone()
                    if dbdevice == None:
                        logger.info(lang.get("check_devices","msg_notallowed") % (device.get('IP'), device.get('MAC')))
                        c.execute("INSERT INTO not_allowed VALUES (?,0,1)", t)
                        conn.commit()
                        notif(lang.get("check_devices","msg_notallowed") % (device.get('IP'), device.get('MAC')))
        conn.close()
    except arpingerror as msg:
        logger.error("Error msg: %s"%str(msg))