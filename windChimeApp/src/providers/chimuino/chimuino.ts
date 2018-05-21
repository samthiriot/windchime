//import { HttpClient } from '@angular/common/http';

// good entry point:
// https://medium.com/@yostane/using-the-at-09-ble-module-with-the-arduino-3bc7d5cb0ac2
// http://don.github.io/slides/2017-07-20-ionic-madison/#/71

import { Injectable } from '@angular/core';
import { ToastController } from 'ionic-angular';
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
	public _device = null;
	private SERVICE:string = "FFE0";
	private CHARACTERISTIC:string = "FFE1";
	private DURATION_SCAN:number = 5;

	constructor(//public http: HttpClient,
  			  private ble: BLE,
			  //private bluetooth: BluetoothSerial,
  			  private storage: Storage,
  			  private toastCtrl: ToastController
  			  ) {

	    console.log('Hello ChimuinoProvider Provider');

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

	displayToastMessage(message:string) {

		let toast = this.toastCtrl.create({
	      message: message,
	      duration: 3000,
	      position: 'top'
	    });
	    toast.present();
	}
	/**
	 * Sends a message to the Chimuino
	 */
	sendMessage(messageRaw:string, tryagain:boolean=true) {

  		let message = messageRaw.trim()+'\n';

		//this.ble.stopScan();
		this.displayToastMessage("should send "+message+"to"+this._device.id+"...");
		
		// convert message to string
		var buf = new ArrayBuffer(message.length*2);
	    var bufView = new Uint8Array(buf);
	    for (var i = 0, strLen = message.length; i < strLen; i++) {
	      bufView[i] = message.charCodeAt(i);
	    }
		this.ble.writeWithoutResponse(this._device.id, this.SERVICE, this.CHARACTERISTIC, buf).then(
				(success) => {
					this.displayToastMessage('sent info :-)');

				},
				(failure) => {
					if (tryagain) {
						this.displayToastMessage("failure: "+failure+", retrying...");
					    //this.ble.disconnect(this._device.id);
					    // try to reconncet
					    this.ble.connect(this._device.id).subscribe(
							(data) => {
								this.displayToastMessage("connected to device "+this._device.name+" with message "+data);
								this.sendMessage(message, false); 
							});
					} else {
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

				if (str.startsWith("GET") || str.startsWith("SET") || str.startsWith("DO") ) {
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
        this.ble.startNotification(this._device.id, this.SERVICE, this.CHARACTERISTIC).subscribe(
			this.onDataNotified, this.onNotificationFailure
        	);

		// send information to the Chimuino
		this.sendDatetime();
		
	}

	onDataNotified(buffer) {
		var data = new Uint8Array(buffer);

		var str = String.fromCharCode.apply(null, data);

		this.displayToastMessage("received message from bluetooth: "+str);
	}

	onNotificationFailure() {
		this.displayToastMessage("notification failure :-(");
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
		  	this._busy = true;

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
