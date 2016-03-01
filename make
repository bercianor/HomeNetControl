#!/bin/sh
### Install/Uninstall file

PACKAGE="homenetcontrol"
NAME="HomeNetControl"

case "$1" in
    install)
        /bin/rm -rf /usr/local/lib/python2.7/dist-packages/$NAME*
        python setup.py install
        mkdir -p /var/local/$PACKAGE
        /bin/rm -rf build/ dist/ $NAME.egg-info/
        ;;
    uninstall)
        /bin/rm -rf /etc/init.d/$PACKAGE
        python setup.py develop -u
        /bin/rm -rf /usr/local/lib/python2.7/dist-packages/$NAME*
        /bin/rm -rf /usr/local/bin/$PACKAGE
        /bin/rm -rf /etc/$PACKAGE
        /bin/rm -rf /var/local/$PACKAGE
        /bin/rm -rf /var/log/$PACKAGE.log
        /bin/rm -rf /root/.python-eggs/$NAME*
        ;;
    *)
        echo -n "Usage: $0 {install|update|uninstall}"
        ;;
esac