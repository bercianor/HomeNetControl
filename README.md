######Español
El propósito de este paquete de python es comprobar los dispositivos conectados a la red. Avisará mediante un bot de telegram si algún dispositivo no permitido se conecta y cuando algún dispositivo configurado para seguimiento se conecta o desconecta.
######English
The purpose for this python packet is to check devices connected to net. It notify via telegram bot if devices not allowed are connected to net, and when some watched devices connects or disconnects from net. (I want to apologize for my english in this text, if you want to correct something, please contact me.)

Instalación - Install
=====================
Requisitos - Requirements
-------------------------
######Español
Para descargar el código con ```git clone``` necesitaras ```git```(obviamente). Además de los paquetes para compilar de tu distribución (generalmente ```build-essential```), python2 y pip. Puedes descargarlos e instalarlos con ```apt-get install git build-essential python2 pip``` (en ubuntu/debian) o con la herramienta de gestión de paquetes disponible, comprueba la documentación de tu distribución para más información.
######English
To download the source code with ```git clone``` you need ```git``` (obviously). Also compile software from your distribution (commonly ```build-essential```), python2 and pip. You can download and install them with ```apt-get install git build-essential python2 pip``` (in ubuntu/debian) or with the packet manager available, check your linux distribution manual for more information.

Instalar/Desinstalar - Install/Uninstall
----------------------------------------
######Español
Para instalar el programa, descárgalo o clónalo de github: ```git clone https://github.com/bercianor/HomeNetControl.git```. Una vez descargado ejecutamos ```./make install```. Para actualizar ejecuta el mismo comando, se mantendrán tus archivos de configuración, y las bases de datos. Para desinstalarlo ejecutamos ```./make uninstall```. Ambos comandos se deben ejecutar como superusuario.
######English
To install the software, download o clone it from github: ```git clone https://github.com/bercianor/HomeNetControl.git```. Once downloaded run ```./mkae install```. To update run the same command, it will keep your configuration files and databases. To uninstall run ```./make uninstall```. Both commands shall be run as superuser.

Configuración - Configuration
-----------------------------
######Español
Lo primero de nada será configurar 2 archivos .ini ubicados en /etc/homenetcontrol:
En config.ini deberemos configurar en la sección [defaults] nuestra red en el ítem NET en formato x.x.x.x/x, por ejemplo, si nuestra ip es 192.168.1.2 y nuestra máscara de red es 255.255.255.0, la red será 192.168.1.0/24. Para más información sobre esto, podéis encontrar multitud de manuales por Internet. También debemos configurar los dispositivos permitidos en [allowed_devices] de la manera que se indica en el fichero, indicando un nombre para el dispositivo, seguido de dos puntos (:) y a continuación la dirección MAC del dispositivo (en minúsculas). Por último en la sección [watch_users] debemos indicar un nombre (para la persona que usa el dispositivo por ejemplo) seguido de dos puntos (:) y a continuación el nombre del dispositivo que indicamos en el apartado anterior. Esta última sección puede quedar vacía y el programa no nos avisará de ninguna conexión o desconexión, excepto de las conexiones de los dispositivos no autorizados.
En bot.ini debemos configurar el TOKEN para nuestro bot de telegram (ver más información en <https://core.telegram.org/bots#botfather>). A continuación debemos especificar el o los usuarios que queramos permitir que reciban las notificaciones (el rol es simplemente una palabra por si queremos diferenciarlos por grupos, no puede estar vacío, pero no es importante el texto que indiquemos). Eso es todo, las otras secciones se rellenaran automáticamente.
######English
First we need to configure 2 .ini files located in /etc/homenetcontrol:
In config.ini we need to configure in the section [defaults] our net in the NET item with the format x.x.x.x/x, for example if our ip is 192.168.1.2 and our netmask is 255.255.255.0, the net should be 192.168.1.0/24. For more information about this, you can find manuals through Internet. We also need to configure the allowed devices in section [allowed_devices] in the way showed in the file, with a device name following by a ':' and ended with the device's MAC address (in lowercase). At the ends we need to configure the [watch_users] section, where we need to assign a name (for example the user's name of that device) follow by a ':' and the device's name given in the previous section. This last section can be empty, and the app doesn't notify us about any connection or disconnection except the connections from the not allowed devices.
At bot.ini we need to configure the telegram's bot TOKEN (for more information see <https://core.telegram.org/bots#botfather>). Next we need to specify the telegram users allowed to receive the notifications (rol is simply a word for separate users in groups for example, it can't be empty, but it's not important what we put in here). That's all, the rest of sections will be filled automatically.

USO - USAGE
===========
######Español
La primera vez que vayas a usar el programa, debemos registrar al menos un usuario para recibir las notificaciones (al añadir al fichero bot.ini solo le hemos dado permiso para poder registrarse). Para registrar un usuario, hay que ejecuta el bot de telegram, y enviarle el comando ```/register```. Esto debemos hacerlo cada vez que queramos que un nuevo usuario reciba las notificaciones, siempre después de haberlo incluido en el fichero bot.ini.
Una vez realizado el registro en telegram, para ejecutar el programa lanzamos ```homenetcontrol``` como superusuario. Podemos añadir la opción '-l' para que guarde el registro de eventos en el fichero /var/log/homenetcontrol.log (si no lo añadimos mostrará los eventos por pantalla). De esta manera el programa se ejecutará hasta que salgamos con ctrl+c. Al instalar el programa se ha creado un script de inicio en ```/etc/init.d/homenetcontrol``` que podemos utilizar para automatizar el inicio al encender el equipo, acudid al manual de vuestra distribución para más información sobre cómo activarlo.
######English
The first time you need to register a telegram user to send the notifications (adding it to the bot.ini file we only give it possibility to register). This needs to be done every time we need to register a new user after adding it to bot.ini.
Once a user is register in the bot we can run the application with ```homenetcontrol``` as superuser. We can add the '-l' option to register events in /var/log/netcontrol.log (if don't, events will be shown in screen). The program the will be in execution until we push ctrl+c to stop it. During install a init script ```/etc/init.d/homenetcontrol``` was created, we can use to automatize the software when boot, check your distribution manual for more info about how to enable it.

CREDITOS - CREDITS
==================
######Español
Este software ha sido hecho por mi (bercianor) en su mayoría. El manejo de los paquetes ARP tiene algo de código e ideas de P. David Buchan (<http://www.pdbuchan.com/rawsock/rawsock.html>).
Si tienes alguna idea para implementar o puedes ayudar/mejorar la traducción de este mensaje o la aplicación, abre una incidencia (issue en github) o lanza un pull-request.
######English
This software was made by me (bercianor). ARP packets management has some code and ideas from P. David Buchan (<http://www.pdbuchan.com/rawsock/rawsock.html>).
If you have some idea to implement or can help with translate of this message or the app, open a issue or make a pull-request via github.

LICENCIA - LICENSE
==================
Home Net Control - Monitorizes devices connected to net and notify
according with its type. Also monitorizes the bandwidth.
Copyright (C) 2016  bercianor  (<https://github.com/bercianor>)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.