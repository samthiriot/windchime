import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { Storage } from '@ionic/storage';
import { ChimuinoProvider } from '../../providers/chimuino/chimuino';

@Component({
  selector: 'page-alarm',
  templateUrl: 'alarm.html'
})
export class AlarmPage {

  _alarm1hour:string = "07:30";
  _alarm1enabled:boolean = false;
  _alarm1soft:number = 10;
  _alarm1strong:number = 15;
  _alarm1days:string = "mon,tue,wed,thu,fri";
  
  _alarm2hour:string = "09:30";
  _alarm2enabled:boolean = false;
  _alarm2soft:number = 10;
  _alarm2strong:number = 15;
  _alarm2days:string = "sun,sat";
  
  constructor(public navCtrl: NavController,
  			   private storage: Storage,
			     private chimuino: ChimuinoProvider) {

  	this.storage.get('alarm-1-hour').then(     (val) => { this._alarm1hour = val;    } );
  	this.storage.get('alarm-1-enabled').then(  (val) => { this._alarm1enabled = val; } );
    this.storage.get('alarm-1-soft').then(     (val) => { this._alarm1soft = val;    } );
    this.storage.get('alarm-1-strong').then(   (val) => { this._alarm1strong = val;  } );
    this.storage.get('alarm-1-days').then(     (val) => { this._alarm1days = val;    } );

  	this.storage.get('alarm-2-hour').then(     (val) => { this._alarm2hour = val;    } );
  	this.storage.get('alarm-2-enabled').then(  (val) => { this._alarm2enabled = val; } );
    this.storage.get('alarm-2-soft').then(     (val) => { this._alarm2soft = val;    } );
    this.storage.get('alarm-2-strong').then(   (val) => { this._alarm2strong = val;  } );
    this.storage.get('alarm-2-days').then(     (val) => { this._alarm2days = val;    } );    
  }

  decodeDaysFromString(value:string) {
    return {
      "sun": value.includes("sun"),
      "mon": value.includes("mon"),
      "tue": value.includes("tue"),
      "wed": value.includes("wed"),
      "thu": value.includes("thu"),
      "fri": value.includes("fri"),
      "sat": value.includes("sat")
    };
  }
  
  updateChimuinoAlarm1() {
  	var tokens = this._alarm1hour.split(":");
    var days = this.decodeDaysFromString(this._alarm1days);
  	this.chimuino.setAlarm1(
      parseInt(tokens[0]), parseInt(tokens[1]), 
      10, 15,
      this._alarm1enabled,
      days.sun, days.mon, days.tue, days.wed, days.thu, days.fri, days.sat
      );
  }
  
  updateChimuinoAlarm2() {
    var tokens = this._alarm2hour.split(":");
    var days = this.decodeDaysFromString(this._alarm2days);
    this.chimuino.setAlarm1(
      parseInt(tokens[0]), parseInt(tokens[1]), 
      10, 15,
      this._alarm2enabled,
      days.sun, days.mon, days.tue, days.wed, days.thu, days.fri, days.sat
      );
  }

  set alarm1enabled(value:boolean) {
  	this._alarm1enabled = value;
  	this.storage.set('alarm-1-enabled', value);
  	this.updateChimuinoAlarm1();
  }
  set alarm1hour(value:string) {
  	this._alarm1hour = value;
  	this.storage.set('alarm-1-hour', value);
  	this.updateChimuinoAlarm1();
  }
  set alarm1soft(value:number) {
    this._alarm1soft = value;
    this.storage.set('alarm-1-soft', value);
    this.updateChimuinoAlarm1();
  }
  set alarm1strong(value:number) {
    this._alarm1strong = value;
    this.storage.set('alarm-1-strong', value);
    this.updateChimuinoAlarm1();
  }
  set alarm1days(value:string) {
    this._alarm1days = value;
    this.storage.set('alarm-1-days', value);
    this.updateChimuinoAlarm1();
  }

  set alarm2enabled(value:boolean) {
  	this._alarm2enabled = value;
   	this.storage.set('alarm-2-enabled', value);
    this.updateChimuinoAlarm2();
  }
  set alarm2hour(value:string) {
  	this._alarm2hour = value;
  	this.storage.set('alarm-2-hour', value);
    this.updateChimuinoAlarm2();
  }
  set alarm2soft(value:number) {
    this._alarm2soft = value;
    this.storage.set('alarm-2-soft', value);
    this.updateChimuinoAlarm2();
  }
  set alarm2strong(value:number) {
    this._alarm2strong = value;
    this.storage.set('alarm-2-strong', value);
    this.updateChimuinoAlarm2();
  }


  get alarm1enabled():boolean {
  	return this._alarm1enabled;
  }
  get alarm1hour():string {
  	return this._alarm1hour;
  }
  get alarm1soft():number {
    return this._alarm1soft;
  }
  get alarm1strong():number {
    return this._alarm1strong;
  }

  get alarm2enabled():boolean {
   	return this._alarm2enabled;
  }
  get alarm2hour():string {
  	return this._alarm2hour;
  }
  get alarm2soft():number {
    return this._alarm2soft;
  }
  get alarm2strong():number {
    return this._alarm2strong;
  }


}
