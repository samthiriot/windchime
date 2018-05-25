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
  _alarm2hour:string = "09:30";
  _alarm2enabled:boolean = false;
  _alarm2soft:number = 10;
  _alarm2strong:number = 15;
  _alarm2days:string = "sun,sat";
  
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
          this._alarm1hour = hours+":"+minutes; 
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
          // TODO save into storage?
      });
    this.events.subscribe(
      'get-alarm2',   
      (hours, minutes, 
        durationSoft, durationStrong, 
        enabled, 
        sunday, monday, tuesday, 
        wednesday, thursday, friday, 
        saterday) => { 
          this._alarm2hour = hours+":"+minutes; 
          this._alarm2enabled = enabled;
          this._alarm2soft = durationSoft;
          this._alarm2strong = durationStrong;
          this._alarm2days = ""; 
          this._alarm2days = this.encoreDaysFromBooleans(sunday,monday,tuesday,wednesday,thursday,friday,saterday);
          this.alarm2loaded = true;
          // TODO save into storage?
      });   
    
     // when bluetooth will be connected, then load info
    this.events.subscribe(
      'connected',
      (isConnected) => {
        if (isConnected) {
          // ask through bluetooth the current settings
          if (!this.alarm1loaded && !this.alarm2loaded) { // avoid infinite looping
            this.loadInfoFromChimuino();
          }
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
    /*
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
    */   
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

  encoreDaysFromBooleans(sunday:boolean,monday:boolean,tuesday:boolean,wednesday:boolean,thursday:boolean,friday:boolean,saterday:boolean) {
    var daysBool = [sunday,monday,tuesday,wednesday,thursday,friday,saterday];
    var daysNames = ["sun","mon","tue","wed","thu","fri","sat"];
    var res:string = "";
    for (let i=0; i<daysBool.length; i++) {
      if (!daysBool[i])
        continue;
      if (res.length) { res += ","; };
      res += daysNames[i];
    }
    return res;
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




}
