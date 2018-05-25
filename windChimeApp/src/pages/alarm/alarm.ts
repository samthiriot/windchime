import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { Storage } from '@ionic/storage';
import { Events } from 'ionic-angular';
import { ChimuinoProvider } from '../../providers/chimuino/chimuino';

// TODO after testing the GUI for alarm1, do the same for alarm2

@Component({
  selector: 'page-alarm',
  templateUrl: 'alarm.html'
})
export class AlarmPage {

  alarm1loaded:boolean = false;
  _alarm1hour:string = "07:25";
  _alarm1enabled:boolean = false;
  _alarm1soft:number = 10;
  _alarm1strong:number = 15;
  _alarm1sunday:boolean = false;
  _alarm1monday:boolean = true;
  _alarm1tuesday:boolean = true;
  _alarm1wednesday:boolean = true;
  _alarm1thursday:boolean = true;
  _alarm1friday:boolean = true;
  _alarm1saterday:boolean = false;

  alarm2loaded:boolean = false;
  _alarm2hour:string = "07:25";
  _alarm2enabled:boolean = false;
  _alarm2soft:number = 10;
  _alarm2strong:number = 15;
  _alarm2sunday:boolean = false;
  _alarm2monday:boolean = true;
  _alarm2tuesday:boolean = true;
  _alarm2wednesday:boolean = true;
  _alarm2thursday:boolean = true;
  _alarm2friday:boolean = true;
  _alarm2saterday:boolean = false;


  
  constructor(public navCtrl: NavController,
    			    private storage: Storage,
  			      private chimuino: ChimuinoProvider,
              private events: Events
              ) {

    // when bluetooth informs us of the availability of information 
    // from the actual chime, then display it
    this.events.subscribe(
      'get-alarm1',   
      (hours, minutes, 
        durationSoft, durationStrong, 
        enabled, 
        sunday, monday, tuesday, 
        wednesday, thursday, friday, 
        saterday) => { 
          this._alarm1hour = (hours<10?"0":"")+hours+":"+(minutes<10?"0":"")+minutes; 
          //this.chimuino.displayDebug("alarm set to "+this._alarm1hour);
          this._alarm1enabled = enabled;
          this._alarm1soft = durationSoft;
          this._alarm1strong = durationStrong;
          this._alarm1sunday = sunday;
          this._alarm1monday = monday;
          this._alarm1tuesday = tuesday;
          this._alarm1wednesday = wednesday;
          this._alarm1thursday = thursday;
          this._alarm1friday = friday;
          this._alarm1saterday = saterday;
          this.alarm1loaded = true;
      });
    this.events.subscribe(
      'get-alarm2',   
      (hours, minutes, 
        durationSoft, durationStrong, 
        enabled, 
        sunday, monday, tuesday, 
        wednesday, thursday, friday, 
        saterday) => { 
          this._alarm2hour = (hours<10?"0":"")+hours+":"+(minutes<10?"0":"")+minutes; 
          //this.chimuino.displayDebug("alarm set to "+this._alarm2hour);
          this._alarm2enabled = enabled;
          this._alarm2soft = durationSoft;
          this._alarm2strong = durationStrong;
          this._alarm2sunday = sunday;
          this._alarm2monday = monday;
          this._alarm2tuesday = tuesday;
          this._alarm2wednesday = wednesday;
          this._alarm2thursday = thursday;
          this._alarm2friday = friday;
          this._alarm2saterday = saterday;
          this.alarm2loaded = true;
      });   
    
     // when bluetooth will be connected, then load info
    this.events.subscribe(
      'connected',
      (isConnected) => {
        if (isConnected) {
          this.loadInfoFromChimuino();
        } else {
          this.alarm1loaded = false;
          this.alarm2loaded = false;
       }
      });  

    // maybe the Chimuino is connected already? 
    if (this.chimuino.isConnected()) {  
      // in this case, let's load the info right now.
      this.loadInfoFromChimuino();
    }
   
  }

  ionViewWillEnter() {
    if (!this.isFullyLoaded()) {
      this.loadInfoFromChimuino();
    }
  }

  isFullyLoaded():boolean {
    return this.alarm2loaded && this.alarm2loaded;
  }

  loadInfoFromChimuino() {
    if (!this.alarm1loaded) { this.chimuino.askAlarm1(); }
    if (!this.alarm2loaded) { this.chimuino.askAlarm2(); }
  }

  private updateChimuinoAlarm1() {
  	var tokens = this._alarm1hour.split(":");
    var hour:number = parseInt(tokens[0]);
    var minutes:number = parseInt(tokens[1]);
    if (isNaN(hour) || isNaN(minutes)) {
      // warn ?!
      return;
    }
  	this.chimuino.setAlarm1(
      hour, minutes, 
      this._alarm1soft, this._alarm1strong,
      this._alarm1enabled,
      this._alarm1sunday, this._alarm1monday, this._alarm1tuesday, this._alarm1wednesday, this._alarm1thursday, this._alarm1friday, this._alarm1saterday
      );
  }
  
  private updateChimuinoAlarm2() {
    var tokens = this._alarm2hour.split(":");
    var hour:number = parseInt(tokens[0]);
    var minutes:number = parseInt(tokens[1]);
    if (isNaN(hour) || isNaN(minutes)) {
      // warn ?!
      return;
    }
    this.chimuino.setAlarm2(
      hour, minutes, 
      this._alarm2soft, this._alarm2strong,
      this._alarm2enabled,
      this._alarm2sunday, this._alarm2monday, this._alarm2tuesday, this._alarm2wednesday, this._alarm2thursday, this._alarm2friday, this._alarm2saterday
      );
  }
  
  demoAlarmPrereveil() {
    this.chimuino.doChimeMedium();
  }
  demoAlarmReveil() {
    this.chimuino.doChimeStrong();
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
  set alarm1sunday(value:boolean)     { this._alarm1sunday = value;     this.updateChimuinoAlarm1(); }
  set alarm1monday(value:boolean)     { this._alarm1monday = value;     this.updateChimuinoAlarm1(); }
  set alarm1tuesday(value:boolean)    { this._alarm1tuesday = value;    this.updateChimuinoAlarm1(); }
  set alarm1wednesday(value:boolean)  { this._alarm1wednesday = value;  this.updateChimuinoAlarm1(); }
  set alarm1thursday(value:boolean)   { this._alarm1thursday = value;   this.updateChimuinoAlarm1(); }
  set alarm1friday(value:boolean)     { this._alarm1friday = value;     this.updateChimuinoAlarm1(); }
  set alarm1saterday(value:boolean)   { this._alarm1saterday = value;   this.updateChimuinoAlarm1(); }

  
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
  set alarm2sunday(value:boolean)     { this._alarm2sunday = value;     this.updateChimuinoAlarm2(); }
  set alarm2monday(value:boolean)     { this._alarm2monday = value;     this.updateChimuinoAlarm2(); }
  set alarm2tuesday(value:boolean)    { this._alarm2tuesday = value;    this.updateChimuinoAlarm2(); }
  set alarm2wednesday(value:boolean)  { this._alarm2wednesday = value;  this.updateChimuinoAlarm2(); }
  set alarm2thursday(value:boolean)   { this._alarm2thursday = value;   this.updateChimuinoAlarm2(); }
  set alarm2friday(value:boolean)     { this._alarm2friday = value;     this.updateChimuinoAlarm2(); }
  set alarm2saterday(value:boolean)   { this._alarm2saterday = value;   this.updateChimuinoAlarm2(); }


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
  get alarm1sunday():boolean { return this._alarm1sunday; }
  get alarm1monday():boolean { return this._alarm1monday; }
  get alarm1tuesday():boolean { return this._alarm1tuesday; }
  get alarm1wednesday():boolean { return this._alarm1wednesday; }
  get alarm1thursday():boolean { return this._alarm1thursday; }
  get alarm1friday():boolean { return this._alarm1friday; }
  get alarm1saterday():boolean { return this._alarm1saterday; }


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
  get alarm2sunday():boolean { return this._alarm2sunday; }
  get alarm2monday():boolean { return this._alarm2monday; }
  get alarm2tuesday():boolean { return this._alarm2tuesday; }
  get alarm2wednesday():boolean { return this._alarm2wednesday; }
  get alarm2thursday():boolean { return this._alarm2thursday; }
  get alarm2friday():boolean { return this._alarm2friday; }
  get alarm2saterday():boolean { return this._alarm2saterday; }




}
