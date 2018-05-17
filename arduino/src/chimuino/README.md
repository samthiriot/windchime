# in practice

## requirements

You need the [Arduino IDE](https://www.arduino.cc/en/Main/Software) to compile and upload the sketch into your Arduino (and into your Chimuino).

You have to install into your arduino libraries folder:
* the Streaming library: http://arduiniana.org/libraries/streaming/
* the DS3232RTC library: https://github.com/JChristensen/DS3232RTC
* the TimeLib library: https://github.com/PaulStoffregen/Time


# FAQ
## can't open device "/dev/ttyXXX": Permission denied

Update the permissions of your port.

Temporary solution (less intrusive):

	sudo chmod o+rw /dev/ttyACM*

Permanent solution: add your user to the group which has permission to play with the device. 
For instance in this case:

	> ls -la /dev/ttyACM4
	crwxrwxrwx 1 root dialout 166, 4 mai   10 13:53 /dev/ttyACM4
so users have to belong the "dialout" group to be allowed writing on this device.  

	> whoami
	sam
	> groups
	sam adm cdrom sudo dip plugdev lpadmin sambashare
	> sudo usermod -a -G dialout sam
	> groups
	sam adm cdrom sudo dip plugdev lpadmin sambashare

Notice how the groups were not changed yet...! You'll have to **log out and log in** for changes to take effect.

see also: <http://www.arduino.cc/en/Guide/Troubleshooting#upload>.
