
The Chuimuino is able to receive and emit data through bluetooth. 

Here are the messages which can be sent and received: 

# global syntax

several basic messages can be exchanged throught bluetooth. 

## actions
action demands are in the form

	> DO <ACTION>

The Chuimuino acknowledges it understood the demand and applied it by answering

	< DOING <ACTION> 

## parameters
Asking for the value of parameters (settings) and setting their value is done in the form:

	> GET <SETTING>
	< <SETTING> IS <VALUE>

	> SET <SETTING> VALUE
	< <SETTING> SET

## others
Other demands can be sent to the arduino. 
For instance:

	> PLEASE <DEMAND>
	< OK <DEMAND>

Below you'll find the complete list of actions and settings.


# actions
## chime and ring now

The Chimuino has a current **action** it is doing now. This is a state 
which lasts for several minutes.

The current action might be:
* NOTHING: doing nothing special right now
* WAKEUP: in a cycle of wakeup
* CHIME: doing a little chime sound

Getting the current state 

	> GET ACTION
	< DOING NOTHING

Stop any current action 

	> DO NOTHING
	< DOING NOTHING

Do wakeup

	> DO WAKEUP
	< DOING WAKEUP

	> GET ACTION
	< DOING WAKEUP 

Do a strong chime

	> DO CHIME
	< DOING CHIME 

	> GET ACTION
	< DOING CHIME 



# settings
## datetime

Get and set the current datetime of the Chimuino, that is the clock time.
The format of the date is TODO. 

    > GET DATETIME
    < DATETIME IS TODO

    > SET DATETIME TODO
    < DATETIME SET

## alarms

There are two alarms which can be activated in the Chuimino.
These alarms mean the Chuimuino will enter alarm mode. 

	> GET ALARM1
	< ALARM1 IS TRUE 09:11

	> SET ALARM1 TRUE 09:11
	< ALARM1 SET

	> PLEASE SNOOZE
	< OK SNOOZE

same with "ALARM2"

## sound level

Gets the sound level measured by the Chuimuino. 
This sound level is used for the Chuimuino to decide to act or not. 

TODO

## light level and threshold

Gets the light level measured by the Chimuino. 

	> GET LIGHTLEVEL
	< LIGHTLEVEL IS 72

	> GET LIGHTLEVELS
	< LIGHTLEVELS IS [72,70,110,75,100,92] 

	> GET LIGHTTHRESHOLD
	< LIGHTTHRESHOLD IS 80

	> SET LIGHTTHRESHOLD 72
	< LIGHTTHRESHOLD IS 72

Setting the light threshold to AUTO will let the Chuimuino define the light threshold automatically.

	> SET LIGHTTHRESHOLD AUTO
	< LIGHTTHRESHOLD IS AUTO 72

	> GET LIGHTTHRESHOLD 
	< LIGHTTHRESHOLD IS AUTO 80

