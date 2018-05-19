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

  sendMessage(message:string) {

		//this.ble.stopScan();
		this.ble.connect(this._device.id).subscribe(
			(data) => {
				


			let toast = this.toastCtrl.create({
		      message: 'sending '+message+' to '+this._device.id+'...',
		      duration: 3000,
		      position: 'top'
		    });
		    toast.present();

	    	// convert message to string
	  		var buf = new ArrayBuffer(message.length*2);
		    var bufView = new Uint8Array(buf);
		    for (var i = 0, strLen = message.length; i < strLen; i++) {
		      bufView[i] = message.charCodeAt(i);
		    }

			this.ble.write(this._device.id, "FFE0", "FFE1", buf).then(
				(success) => {
					let toast = this.toastCtrl.create({
				      message: 'sent info :-)',
				      duration: 3000,
				      position: 'top'
				    });
				    toast.present();
				    this.ble.disconnect(this._device.id);

				},
				(failure) => {
					let toast = this.toastCtrl.create({
				      message: 'failure info: '+failure,
				      duration: 3000,
				      position: 'top'
				    });
				    toast.present();	
				    this.ble.disconnect(this._device.id);

				});
			}
		);
  }

  connect() {
  	this.ble.enable().then( (enabled) => {
	  		return this.storage.get('bluetooth-id');	
	  	}).then( (id) => {
	  		let toast = this.toastCtrl.create({
		      message: 'scanning for '+id+'...',
		      duration: 3000,
		      position: 'top'
		    });
		    toast.present();
	  		this.ble.scan(["FFE0"], 5).subscribe(
	  			(device) => {
	  				if (device.name != "CHIMUINO") {
	  					return;
	  				}
	  				this._device = device;

	  				// inform the user
	  				let toast = this.toastCtrl.create({
				      message: 'found device '+device.name+'...',
				      duration: 3000,
				      position: 'top'
				    });
				    toast.present();

				    // react to the first connection
				    this.reactFirstConnection();
	  			}
	  		);
	  	})
 
  }

  reactFirstConnection() {

  	// send information to the Chimuino

  	// .. adapt datetime
  	var now = new Date;
	this.sendMessage("SET DATETIME "+now.getFullYear()+"-"+now.getMonth()+"-"+now.getDate()+" "+now.getHours()+":"+now.getMinutes()+":"+now.getSeconds());
  }

  setAlarm1(hour:number, minutes:number, enabled:boolean) {
  	this.sendMessage("SET ALARM1 "+hour+":"+minutes+" "+(enabled?"1":"0"));
  }

  writeVersion() {
	this.sendMessage("GET VERSION");
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
  setDatetime() {
  	// nothing
  }
 	

}
