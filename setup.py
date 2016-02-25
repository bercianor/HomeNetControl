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

from ez_setup import use_setuptools
use_setuptools()
from setuptools import setup, find_packages, Extension
import sys

print("""
    Home Net Control  Copyright (C) 2016  bercianor
    This program comes with ABSOLUTELY NO WARRANTY.
    This is free software, and you are welcome to redistribute it
    under certain conditions; open LICENSE for details.
""")

setup(
    name                = 'HomeNetControl',
    description         = 'Monitorizes devices connected to net and notify according with its type. Also monitorizes the bandwidth.',
    long_description    = 'Check README',
    version             = '1.0',
    author              = 'bercianor',
    author_email        = 'https://github.com/bercianor',
    license             = 'GPLv3',
    url                 = 'https://github.com/bercianor/HomeNetControl',
    
    entry_points        = {'console_scripts': ['homenetcontrol = homenetcontrol.homenetcontrol:main']},
    packages            = find_packages(),
    package_data        = {'homenetcontrol': ['langs/*.lang']},
    data_files          = [
                            ('/etc/homenetcontrol', ['conf_files/config.ini', 'conf_files/bot.ini', ]),
                            ('/etc/init.d', ['init/homenetcontrol', ]),
                          ],
    install_requires    = ["python-telegram-bot"],
    ext_modules         = [Extension("homenetcontrol.arping", 
                                    ["src/arping.c"], 
                                    libraries=[],
                                    extra_compile_args=['-std=c11'])
                          ],
)