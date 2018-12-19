## Install Dependencies 
sudo apt-get install libglib2.0-dev
sudo apt-get install libdbus-1-dev

## Update dbus configuration file
sudo cp tbot.conf /etc/dbus-1/system.d/
sudo chmod 777 /etc/dbus-1/system.d/tbot.conf

## Restart dbus service
sudo service dbus restart
