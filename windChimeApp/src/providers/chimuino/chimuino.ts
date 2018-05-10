import { HttpClient } from '@angular/common/http';
import { Injectable } from '@angular/core';
import { ToastController } from 'ionic-angular';
import { Storage } from '@ionic/storage';
import { BluetoothSerial } from '@ionic-native/bluetooth-serial';

/*
  Generated class for the ChimuinoProvider provider.

  See https://angular.io/guide/dependency-injection for more info on providers
  and Angular DI.
*/
@Injectable()
export class ChimuinoProvider {

  private _busy:boolean = false;

  constructor(public http: HttpClient,
  			  private bluetooth: BluetoothSerial,
  			  private storage: Storage,
  			  private toastCtrl: ToastController
  			  ) {
    console.log('Hello ChimuinoProvider Provider');

    // try to get a bluetooth connection
	this.storage.get('bluetooth-id')
		.then((id) => { 
			return this.bluetooth.connect(id); 
		})
		.subscribe(
			(data) => {
			    console.log(data);
		  	},

			(err) => {
			    console.log(err);
			},

			() => {
			    console.log("completed");
			});

    // test message
    // TODO remove
    let toast = this.toastCtrl.create({
      message: 'Started ChimuinoProvider',
      duration: 3000,
      position: 'top'
    });
    toast.present();
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

  }

}
