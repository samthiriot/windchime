//import { HttpClient } from '@angular/common/http';

// good entry point:
// https://medium.com/@yostane/using-the-at-09-ble-module-with-the-arduino-3bc7d5cb0ac2
// http://don.github.io/slides/2017-07-20-ionic-madison/#/71

import { Injectable } from '@angular/core';
import { ToastController } from 'ionic-angular';
import { Events } from 'ionic-angular';
import { Storage } from '@ionic/storage';
//import { BluetoothSerial } from '@ionic-native/bluetooth-serial';
import { BLE } from '@ionic-native/ble';

/*
  Generated class for the ChimuinoProvider provider.

  See https://angular.io/guide/dependency-injection for more info on providers
  and Angular DI.
*/
@Injectable()
export class ChimuinoProvider {

	private _busy:boolean = false;
	private _isConnected:boolean = false;
	public _device = null;
	private SERVICE:string = "FFE1";
	private CHARACTERISTIC:string = "FFE2";
	private DURATION_SCAN:number = 5;
	private _pendingCommands:string[] = [];

	// strings received so far from bluetooth which don't yet end with carriage return
	private currentBuffer = "";

	constructor(//public http: HttpClient,
  			  private ble: BLE,
			  //private bluetooth: BluetoothSerial,
  			  private storage: Storage,
  			  private toastCtrl: ToastController,
  			  private events: Events
  			  ) {

	    console.log('Hello ChimuinoProvider Provider');

	    // publish the events related to connection
	    // ... are we connected or not?
	    this.events.publish("connected", false);
	    // ... date and time
	    this.events.publish("get-date", null);
	    this.events.publish("get-time", null);
	    this.events.publish("set-date", false);
	    this.events.publish("set-time", false);
		// ... ambiance
		this.events.publish("get-ambiance", null);
		this.events.publish("set-ambiance", false);
		// .. temperature
		this.events.publish("get-temperature", null);
		this.events.publish("set-temperature", false);
		// ... alarms
		this.events.publish("get-alarm1", null);
		this.events.publish("set-alarm1", false);
		this.events.publish("get-alarm2", null);
		this.events.publish("set-alarm2", false);
		

	    // try to get a bluetooth connection
		this.connect();

	    // test message
	    // TODO remove
	    let toast = this.toastCtrl.create({
	      message: 'Started ChimuinoProvider',
	      duration: 3000,
	      position: 'top'
	    });
	    toast.present();
	}

	isConnected():boolean {
		return this._isConnected = false;
	}

	displayToastMessage(message:string) {

		let toast = this.toastCtrl.create({
	      message: message,
	      duration: 3000,
	      position: 'top'
	    });
	    toast.present();
	}

	sendPendingMessages() {
		if (this._pendingCommands.length == 0) {
			// nothing to send yet
			return;
		}
		// there is something to send !
		var message:string = this._pendingCommands.shift();
		this.displayToastMessage("processing pending: "+message);
		this.sendMessage(message);
	}

	/**
	 * queues a message in the list of messages to send
	 * when bluetooth will be ready (again?)
	 */
	queueMessage(message:string) {
		// queue this demand
		if (this._pendingCommands.indexOf(message) > -1) {
			// already queued.
			return;
		}
		this._pendingCommands.push(message);
		this.displayToastMessage("queuing "+message);
	}
	/**
	 * Sends a message to the Chimuino
	 */
	sendMessage(messageRaw:string, tryagain:boolean=true) {

  		let message = messageRaw.trim()+'\n';

  		// if we are busy, it's not yet time to send an additional query
  		if ( (this._busy && tryagain) || !this._isConnected) {
  			this.queueMessage(message);
  			return;
  		}

  		if (!this.ble.isEnabled() || !this.ble.isConnected(this._device.id)) {
  			this.queueMessage(message);
  			this.connect();
  			return;
  		}

  		// Okay, we're ready to send a message.

  		this._busy = true;

		//this.ble.stopScan();
		this.displayToastMessage("should send "+message+"to"+this._device.id+"...");
		
		// convert message to string
		var buf = new ArrayBuffer(message.length*2);
	    var bufView = new Uint8Array(buf);
	    for (var i = 0, strLen = message.length; i < strLen; i++) {
	      bufView[i] = message.charCodeAt(i);
	    }

	    // actually send it 
		this.ble.writeWithoutResponse(this._device.id, this.SERVICE, this.CHARACTERISTIC, buf).then( 
				(success) => {
					this._busy = false;
					// nota: we will be notified by another method of the actual success
					//this.displayToastMessage('sent info :-) '+success);
					this.sendPendingMessages();
				},
				(failure) => {
					this._busy = false;
					if (tryagain) {
						this.displayToastMessage("failure: "+failure+", will retry later...");
						this.queueMessage(message);
						this.connect();
					} else {
						this._busy = false;
						this.displayToastMessage("failure, not trying again.");
					}
				}
			);
	
  	}

  	readResult():string {
  		
  		this.ble.read(this._device.id, this.SERVICE, this.CHARACTERISTIC).then(
  			(buffer) => {
  				var data = new Uint8Array(buffer);

				var str = String.fromCharCode.apply(null, data);

				if (str.startsWith("GET") || str.startsWith("SET") || str.startsWith("DO") || str.startsWith("DEBUG")) {
					// ignore the commands sent by someone
					this.displayToastMessage("ignored from bluetooth: "+str);

				} else {
					this.displayToastMessage("received from bluetooth: "+str);	
				}
  				
  			}
  		);

  		return 'not yet implemented';

  	}

	connect() {
		// enable bluetooth first
		this.ble.enable().then( (enabled) => {
			// then get the expected id of the device
	  		return this.storage.get('bluetooth-id');	
	  	}).then( (id) => {
	  		// then search for our device 
	  		this.displayToastMessage('scanning for '+id+'...');
		    this.ble.scan([this.SERVICE], this.DURATION_SCAN).subscribe(
		    	(device) => {
  					if (device.name != "CHIMUINO") {
  						// TODO reject from ID instead !
				  		this.displayToastMessage('rejected device '+device.name);
	  					return;
	  				}
	  				// found our device
	  				this._device = device;
				    // react to the first connection
		    		this.displayToastMessage('connecting device '+this._device.name+'...');

				    this.reactDeviceFound();
  				}
  				// TODO detection of failures ???
		    );
	  	});

	}

	/**
	 * React when the bluetooth device was found; 
	 * this._device is thus set.
	 */
	reactDeviceFound() {

		// display information to the user
		this.displayToastMessage('connecting device '+this._device.name+'...');

		this.ble.connect(this._device.id).subscribe(
			(data) => {
				this.displayToastMessage("connected to device "+this._device.name+" with message "+data);
				this.reactDeviceConnected(); 
			});

	}

	reactDeviceConnected() {

		// register (=listen) to changes
		// TODO patch for IOS https://github.com/don/cordova-plugin-ble-central#typed-arrays
        this.ble.startNotification(this._device.id, this.SERVICE, this.CHARACTERISTIC).subscribe(
			(data) => { this.onDataNotified(data); }, 
			(error) => { this.onNotificationFailure(); }
		);

		this._isConnected = true;

		// send information to the Chimuino
		this.sendDatetime();
		
		// start pushing the messages which were pending 
		this.sendPendingMessages();

		// inform ours listeners that we are connected now
		this.events.publish("connected", true);


	}

	/**
	 * When a ... SET result is received, it means the chimuino is acknowledging
	 * an action. We update the corresponding SET event with true, which means
	 * "this was updated" (true does not mean anything related to the actual value)
	 */
	onSetAcknowledged(str:string) {

		var code:string = str.trim().toLowerCase();
		this.events.publish("set-"+code, true);
		this.displayToastMessage("acknowledged: set "+code);

	}

	/**
	 *
	 */
	decodeAlarm(name:string, str:string) {
		// input format is: 
		// 09:11 10 15 0 0111110
		var tokens = str.split(' ');
		var tokensTime = tokens[0].split(":");
		var hours:number = parseInt(tokensTime[0]);
		var minutes:number = parseInt(tokensTime[2]);
		var durationSoft:number = parseInt(tokens[1]);
		var durationStrong:number = parseInt(tokens[2]);
		var enabled:boolean = tokens[3]=='1';
		var daysStr = tokens[4];
		var sunday:boolean = daysStr[0]=='1';
		var monday:boolean = daysStr[1]=='1';
		var tuesday:boolean = daysStr[2]=='1';
		var wednesday:boolean = daysStr[3]=='1';
		var thursday:boolean = daysStr[4]=='1';
		var friday:boolean = daysStr[5]=='1';
		var saterday:boolean = daysStr[6]=='1';
		this.events.publish("get-"+name, hours, minutes, 
										 durationSoft, durationStrong, 
										 enabled, 
										 sunday, monday, tuesday, 
										 wednesday, thursday, friday, 
										 saterday);
	}

	/**
	 * Receive the answer with the value of something.
	 * Will propagate an event with the decoded value.
	 */
	onGetReceived(what:string, valueStr:string) {
		var data = null;
		if (what == "AMBIANCE") {
			var enabled:boolean = valueStr[0]=='1';
			this.events.publish("get-ambiance", enabled);
		} else if (what == "ALARM1") {
			this.decodeAlarm("alarm1", valueStr);
		} else if (what == "ALARM2") {
			this.decodeAlarm("alarm2", valueStr);
		} else if (what == "SOUNDTHRESHOLD" || what == "LIGHTTHRESHOLD") {
			// values containing just one int
			var valueInt = parseInt(valueStr);
			this.events.publish("get-"+what.toLowerCase(), valueInt);
		} else if (what == "VERSION") {
			// values decoded as pure strings
			this.events.publish("get-"+what.toLowerCase(), valueStr);
		} else {
			this.displayToastMessage("unknown value received: "+what+" IS "+valueStr);
		}

	}

	onDataNotified(buffer) {

		//this.displayToastMessage("notified by bluetooth: "+ buffer);

		var str:string = String.fromCharCode.apply(null, new Uint8Array(buffer));

		if (str.endsWith("\n")) {
			// end of a complete bluetooth string; process it
			str = this.currentBuffer+str;
			this.currentBuffer = "";
		} else {
			// this input is not complete; let's concatenate it
			this.currentBuffer = this.currentBuffer+str;
			if (this.currentBuffer.length > 255) {
				this.displayToastMessage("ignoring too long message: "+this.currentBuffer);
				this.currentBuffer = "";
			}
			return;
		}

		// process
		if (str.endsWith(" SET")) {
			// received a message in the form "<SOMETHING> SET"
			this.onSetAcknowledged(str.substring(0,str.length-4));
		} else if (str.includes(" IS ")) {
			var idx = str.indexOf(" IS "); 
			var what:string = str.substring(0,idx-1);
			var value:string = str.substring(idx+4);
			this.onGetReceived(what, value);
		} /*else if (str.startsWith("GET ") || str.startsWith("SET ") || str.startsWith("DO ") || str.startsWith("DEBUG")) {
			// ignore the commands sent by someone
			// should not happen
			this.displayToastMessage("ignored notified from bluetooth: "+str);
		} */ else {
			this.displayToastMessage("ignored from bluetooth: "+str);	
		}
	}

	onNotificationFailure() {
		this._isConnected = false;
		this.displayToastMessage("notification failure :-(");
		this.events.publish("connected", false);
	}	

	setSoundThreshold(t:number) {
		this.sendMessage("SET SOUNDTHRESHOLD "+t);
	}

	setLightThreshold(t:number) {
		this.sendMessage("SET LIGHTTHRESHOLD "+t);
	}

	setAmbiance(enabled:boolean) {
		this.sendMessage(
			"SET AMBIANCE "+
			(enabled?"1":"0")
			);
	}

	setAlarm1(hour:number, minutes:number, durationSoft:number, durationStrong:number, enabled:boolean, 
			sunday:boolean, monday:boolean, tuesday:boolean, wednesday:boolean, thursday:boolean, friday:boolean, saterday:boolean) {
		
		this.sendMessage(
			"SET ALARM1 "+hour+":"+minutes+" "+
			durationSoft + " " + durationStrong + " "+
			(enabled?"1":"0")+" "+
			(sunday?"1":"0")+(monday?"1":"0")+(tuesday?"1":"0")+(wednesday?"1":"0")+(thursday?"1":"0")+(friday?"1":"0")+(saterday?"1":"0")
			);
		this.readResult(); // TODO
	}

  	setAlarm2(hour:number, minutes:number, durationSoft:number, durationStrong:number, enabled:boolean, 
  			sunday:boolean, monday:boolean, tuesday:boolean, wednesday:boolean, thursday:boolean, friday:boolean, saterday:boolean) {
	  	
	  	this.sendMessage(
	  		"SET ALARM2 "+hour+":"+minutes+" "+
	  		durationSoft + " " + durationStrong + " "+
	  		(enabled?"1":"0")+" "+
	  		(sunday?"1":"0")+(monday?"1":"0")+(tuesday?"1":"0")+(wednesday?"1":"0")+(thursday?"1":"0")+(friday?"1":"0")+(saterday?"1":"0")
	  		);
	}

	askAlarm1() {
		this.sendMessage("GET ALARM1");
	}
	askAlarm2() {
		this.sendMessage("GET ALARM1");
	}

	writeVersion() {

		this.sendMessage("GET VERSION");
		this.readResult();

	  	/*

		this.bluetooth.write('GET VERSION\n').then(
			(success) => { 
				let toast = this.toastCtrl.create({
			      message: 'success: '+success,
			      duration: 3000,
			      position: 'top'
			    });
			    toast.present();
			},
			(failure) => {
				let toast = this.toastCtrl.create({
			      message: 'failure: '+failure,
			      duration: 3000,
			      position: 'top'
			    });
			    toast.present();
			});
		*/
	}

  	getVersion():Promise<String> {

		return new Promise( (resolve,reject) => {

	 		// define we are busy

			this.sendMessage("DIS MOI TRUC");

		  	/*
			// send question
		  	this.bluetooth.write('GET VERSION\n')
					  	  .then(
							version => { 
							  	this._busy = false;
								resolve("sent mess "+version);
								}, 
							failure =>  { 
								this._busy = false;
								reject("error while writing.");
							}
							);
			*/
	 	});
	}

	/*
	* Gets the time from the Chuimuino,
	* and returns it as a Promise. 
	*/
	getDatetime() {
		
		// define we are busy
		this._busy = true;

		// send the message asking for the date 
		// "GET DATE"
		// wait for the answer 

		// not busy anymore
		this._busy = false;
	}

	/*
	* Sets the time of the chimuino to 
	* the current time of the system.
	*/
	sendDatetime() {
		// .. adapt datetime
		var now = new Date;
		this.sendMessage(
			"SET DATE "+now.getFullYear()+"-"+(now.getMonth()+1)+"-"+now.getDate());
		this.sendMessage(
			"SET TIME "+now.getHours()+":"+now.getMinutes()+":"+now.getSeconds());
	}
 	

}
