import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { Storage } from '@ionic/storage';

@Component({
  selector: 'page-alarm',
  templateUrl: 'alarm.html'
})
export class AlarmPage {

  _alarm1hour:string;
  _alarm1enabled:boolean = false;
  _alarm2hour:string;
  _alarm2enabled:boolean = false;

  constructor(public navCtrl: NavController,
  			  private storage: Storage) {

  	this.storage.get('alarm-1-hour').then((val) => { this._alarm1hour = val; } );
	this.storage.get('alarm-1-enabled').then((val) => { this._alarm1enabled = val; } );
	this.storage.get('alarm-2-hour').then((val) => { this._alarm2hour = val; } );
	this.storage.get('alarm-2-enabled').then((val) => { this._alarm2enabled = val; } );

  }



  set alarm1enabled(value:boolean) {
  	this.storage.set('alarm-1-enabled', value);
  }
  set alarm1hour(value:string) {
  	this.storage.set('alarm-1-hour', value);
  }
  set alarm2enabled(value:boolean) {
   	this.storage.set('alarm-2-enabled', value);
  }
  set alarm2hour(value:string) {
  	this.storage.set('alarm-2-hour', value);
  }

  get alarm1enabled():boolean {
  	return this._alarm1enabled;
  }
  get alarm1hour():string {
  	return this._alarm1hour;
  }
  get alarm2enabled():boolean {
   	return this._alarm2enabled;
  }
  get alarm2hour():string {
  	return this._alarm2hour;
  }


}
