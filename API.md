
The Chuimuino is able to receive and emit data through bluetooth. 

Here are the messages which can be sent and received: 

# global syntax

several basic messages can be exchanged throught bluetooth. 

## actions
action demands are in the form

	> DO <ACTION>\n

The Chuimuino acknowledges it understood the demand and applied it by answering

	< DOING <ACTION>\n

## settings
Asking for the value of parameters (settings) and setting their value is done in the form:

	> GET <SETTING>\n
	< <SETTING> IS <VALUE>\n

	> SET <SETTING> VALUE\n
	< <SETTING> SET\n

## state
Asking for the value of a state or characteristic of the Chimuino.

	> GET <SOMETHING>\n
	< <SOMETHING> IS <VALUE>\n


Below you'll find the complete list of actions, settings and states.


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

	> DO CHIME [LIGHT|MEDIUM|STRONG]
	< DOING CHIME 

	> GET ACTION
	< DOING CHIME 



# information
## version

Returns the version of the firmware running the arduino.

	> GET VERSION
	< VERSION IS alpha_2018_05_10

Returns the uptime in minutes

	> GET UPTIME
	< UPTIME IS 454232


# settings
## ambiance

Get and set the fact the chime plays sound from time to time to create a nice atmosphere. 

Enabled is 1 or 0

	> SET AMBIANCE 1
	< AMBIANCE SET

	> GET AMBIANCE
	< AMBIANCE IS 1

## datetime

Get and set the current datetime of the Chimuino, that is the clock time.
The format of the date is TODO. 

    > SET DATETIME 2018-01-24 22:31:01
    < DATETIME SET

    > GET DATETIME 
    < DATETIME IS 2018-01-24 22:31:01


## alarms

There are two alarms which can be activated in the Chuimino.
These alarms mean the Chuimuino will enter alarm mode. 

The format is made of:

    <HOUR>:<MINUTES> <DURATION SOFT MIN><DURATION STRONG MIN>  <ENABLED> <SUNDAY><MONDAY><TUESDAY><WEDNESDAY><THURSDAY><FRIDAY><SATERDAY>

	> GET ALARM1
	< ALARM1 IS 09:11 10 15 0 0111110

	> SET ALARM1 09:11 10 15 1 0111110
	< ALARM1 SET

same with "ALARM2"

Snooze asks to report a bit the current alarm (NOT YET IMPLEMENTED)

	> DO SNOOZE
	< DOING SNOOZE

if error (no alarm ongoing)

	> DO SNOOZE
	< FAILED SNOOZE


## sound level

Gets the sound level measured by the Chuimuino. 
This sound level is used for the Chuimuino to decide to act or not. 
The sound level is an int between 0 and 100. It corresponds to the actual raw value in the sound envelope.

level is either NOISY or QUIET
the sound threshold is a value between 0 and 100. 


	> GET SOUNDLEVEL
	< SOUNDLEVEL IS 110 NOISY

	> GET SOUNDTHRESHOLD
	< SOUNDTHRESHOLD IS 75 

	> SET SOUNDTHRESHOLD 75
	< SOUNDTHRESHOLD SET

	> GET SOUNDENVELOPE
	< SOUNDENVELOPE IS 50 165

## light level and threshold

Gets the light level measured by the Chimuino. 

Detected status is either DARK or LIT

	> GET LIGHTLEVEL
	< LIGHTLEVEL IS 72 LIT 

	> GET LIGHTTHRESHOLD
	< LIGHTTHRESHOLD IS 80 

	> SET LIGHTTHRESHOLD 72
	< LIGHTTHRESHOLD IS 72

	> GET LIGHTENVELOPE
	< LIGHTENVELOPE IS 50 165