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

from sys import version_info, exit
if __name__ == '__main__':    
    exit("[!] This is a module import it!")
else:
    # Import needed libraries
    from logging import getLogger
    from telegram import Bot, TelegramError, ReplyKeyboardMarkup
    from configparser import ConfigParser
    from time import sleep
    from locale import getdefaultlocale
    from pkg_resources import resource_filename, resource_exists
    from codecs import open
    if version_info.major is 2:
        from urllib2 import URLError
    elif version_info.major is 3:
        from urllib.error import URLError
    
    # Set language messages to use
    if resource_exists("homenetcontrol", "langs/" + getdefaultlocale()[0][:2] + ".lang"):
        langfile = resource_filename("homenetcontrol", "langs/" + getdefaultlocale()[0][:2] + ".lang")
    else:
        langfile = resource_filename("homenetcontrol", "langs/en.lang")
    lang = ConfigParser()
    lang.readfp(open(langfile, "r", "utf8"))
    
    # Get info from config file
    if resource_exists("homenetcontrol", "bot/bot.ini"):
        configfile = resource_filename("homenetcontrol", "bot/bot.ini")
    else:
        configfile = "/etc/homenetcontrol/bot.ini"
    config = ConfigParser()
    config.read(configfile)
    
    # Set bot and logger
    bot = Bot(token=config.get("config","TOKEN"))
    logger = getLogger("homenetcontrol")

# Define function to make a notification to all registered users
def notif(message):
    if config.options('registered') == []:
        logger.error("No chat_id registered to send notifications")
    else:
        try:
            for option in config.options('registered'):
                bot.sendMessage(chat_id=config.get('registered', option), text=message, reply_markup=ReplyKeyboardMarkup([[ '/status' ]], resize_keyboard=True, one_time_keyboard=True))
        except TelegramError as e:
            logger.error(e.message)

# Main function
def telegram_bot():
    try:
        from ..check_devices import devices_status
    except ValueError:
        from check_devices import devices_status
    try:
        messages = bot.getUpdates(offset=config.get('bot', 'update_id'), timeout=10)
        for update in messages:
            chat_id = update.message.chat_id
            config.set('bot', 'update_id', str(update.update_id + 1))
            with open(configfile, 'w') as configfilew:
                config.write(configfilew)
            if update.message.text:
                if update.message.from_user.username in config.options('allowed'):
                    
                    # /start
                    if '/start' in update.message.text:
                        if update.message.from_user.username in config.options('registered'):
                            bot.sendMessage(chat_id=chat_id, text=lang.get("bot","msg_hello") % config.get('allowed', update.message.from_user.username), reply_markup=ReplyKeyboardMarkup([[ '/status' ]], resize_keyboard=True, one_time_keyboard=True))
                        else:
                            bot.sendMessage(chat_id=chat_id, text=lang.get("bot","msg_hello") % config.get('allowed', update.message.from_user.username), reply_markup=ReplyKeyboardMarkup([[ '/register' ]], resize_keyboard=True, one_time_keyboard=True))
                    
                    # /register
                    elif '/register' in update.message.text:
                        bot.sendMessage(chat_id=chat_id, text=lang.get("bot","msg_registered") % config.get('allowed', update.message.from_user.username), reply_markup=ReplyKeyboardMarkup([[ '/status' ]], resize_keyboard=True, one_time_keyboard=True))
                        logger.info("%s registered" % update.message.from_user.username)
                        config.set('registered', update.message.from_user.username, str(update.message.chat_id))
                        with open(configfile, 'w') as configfilew:
                            config.write(configfilew)
                    
                    # /status
                    elif '/status' in update.message.text:
                        devices_status()
                
                # not allowed user
                else:
                    logger.info("@%s: %s" % (update.message.from_user.username, update.message.text))
                    if '/start' in update.message.text or '/help' in update.message.text:
                        bot.sendMessage(chat_id=chat_id, text=lang.get("bot","msg_help"))
                    else:
                        bot.sendMessage(chat_id=chat_id, text=update.message.text)
    except TelegramError as e:
        # These are network problems with Telegram.
        if e.message in ("Bad Gateway", "Timed out"):
            logger.error("There are network problems with Telegram. " + e.message)
        else:
            logger.error(e.message)
    except URLError as e:
        # These are network problems on our end.
        logger.error("There are network problems on our end. " + str(e.reason))
    except KeyboardInterrupt:
        pass
    sleep(1)