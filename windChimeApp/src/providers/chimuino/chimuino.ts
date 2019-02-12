//import { HttpClient } from '@angular/common/http';

// good entry point:
// https://medium.com/@yostane/using-the-at-09-ble-module-with-the-arduino-3bc7d5cb0ac2
// http://don.github.io/slides/2017-07-20-ionic-madison/#/71

import { Injectable } from '@angular/core';
import { ToastController } from 'ionic-angular';
import { Storage } from '@ionic/storage';
import { BLE } from '@ionic-native/ble';

// https://ionicframework.com/docs/v3/native/bluetoothle/
// https://github.com/randdusing/cordova-plugin-bluetoothle
//import { BluetoothLE } from '@ionic-native/bluetooth-le';

import {Platform} from 'ionic-angular';


/**
 * Mediates all the interactions between the arduino on the chimuino through bluetooth.
 * Searches Bluetooth Low Energy for the Chimuino device; connects it and listens to 
 * events of interest; read the info; keeps it as attributes and publishes getters and 
 * setters which do save these parameters into the chime using BLE. 
 * 
 * TODO

 * The connection steps for this provider are (in case of success):
 * - created -> connect
 * - connect -> reactDeviceFound
 * - reactDeviceFound -> reactDeviceConnected
 * - reactDeviceConnected -> events.publish("connected", true)
 *
 */
@Injectable()
export class ChimuinoProvider {

	// switch to true to have plenty of toast messages
	// displaying what happens with bluetooth at every step.
	public DEBUG:boolean = true;

	public _isConnected:boolean = false;
	public _device = null;
	private DURATION_SCAN:number = 5;

	private _firstConnection:boolean = true;

	private static readonly SERVICE_CHIMUINO:string = "5550";
	private static readonly CHARACTERISTIC_TIME:string = "2A08";
	private static readonly CHARACTERISTIC_ALARM1:string = "5551";
	private static readonly CHARACTERISTIC_ALARM2:string = "5552"; // TODO 

	private static readonly SERVICE_SENSING:string = "181A";
	private static readonly CHARACTERISTIC_TEMPERATURE:string = "2A6E";


	/**********************************************************************
	 * Ambiance data storage and access
	 **********************************************************************/

	public isAmbianceLoaded = false;
	private _isChimeEnabled:boolean = true;
	private _chimeLevel:number = 50;

	public isLightThresholdLoaded:boolean = false;
	private _lightThreshold:number = 50;

	public isSoundThresholdLoaded:boolean = false;
	private _soundThreshold:number = 50;
	public soundLevel:number = 50;
	public isQuiet:boolean = false;
	public soundMinMax:String = "?:?";

	public isLightLevelLoaded:boolean = false;
	public lightLevel:number = 50;
	public isDark:boolean = false;
	public lightMinMaxStr:String = "?:?";


	set chimeEnabled(value:boolean) {
		this._isChimeEnabled = value;
		this.storage.set('chime-activated', value);
		this.setAmbiance(this._isChimeEnabled);
	}

	get chimeEnabled():boolean {
		return this._isChimeEnabled;
	}

	set chimeLevel(value:number) {
		this._chimeLevel = value;
		this.storage.set('chime-level', value);
	}

	get chimeLevel():number {
		return this._chimeLevel;
	}

	get lightThreshold():number {
		return this._lightThreshold;
	}
	set lightThreshold(value:number) {
		this._lightThreshold = value;
		this.setLightThreshold(this._lightThreshold);
	}

	get soundThreshold():number {
		return this._soundThreshold;
	}
	set soundThreshold(value:number) {
		this._soundThreshold = value;
		this.setSoundThreshold(this._soundThreshold);
	}

	/**********************************************************************
	 * Alarm 1 data storage and access
	 **********************************************************************/

	// is data related to alarm 1 loaded?
	alarm1loaded:boolean = false;

	// data for alarm 1
	_alarm1hour:number = 7;
	_alarm1minutes:number = 25;
	_alarm1enabled:boolean = false;
	_alarm1soft:number = 10;
	_alarm1strong:number = 15;
	_alarm1sunday:boolean = false;
	_alarm1monday:boolean = true;
	_alarm1tuesday:boolean = true;
	_alarm1wednesday:boolean = true;
	_alarm1thursday:boolean = true;
	_alarm1friday:boolean = true;
	_alarm1saturday:boolean = false;

	// getters for alarm1 data
	get alarm1enabled():boolean 	{ return this._alarm1enabled; 		}
	get alarm1time():string 		{ return (this._alarm1hour<10?"0":"")+this._alarm1hour+":"+(this._alarm1minutes<10?"0":"")+this._alarm1minutes;  }
	get alarm1soft():number 		{ return this._alarm1soft; 			}
	get alarm1strong():number 		{ return this._alarm1strong;		}
	get alarm1sunday():boolean 		{ return this._alarm1sunday; 		}
	get alarm1monday():boolean 		{ return this._alarm1monday; 		}
	get alarm1tuesday():boolean 	{ return this._alarm1tuesday; 		}
	get alarm1wednesday():boolean 	{ return this._alarm1wednesday; 	}
	get alarm1thursday():boolean 	{ return this._alarm1thursday; 		}
	get alarm1friday():boolean 		{ return this._alarm1friday; 		}
	get alarm1saturday():boolean 	{ return this._alarm1saturday; 		}

	// setters for alarm1 data
	set alarm1enabled(value:boolean) 	{	this._alarm1enabled = value;   	this.storage.set('alarm-1-enabled', value);  	this.updateChimuinoAlarm1();	}
	set alarm1time(value:string) 		{ 	
		let tokens = value.split(":");
		this._alarm1hour = parseInt(tokens[0]);
		this._alarm1minutes = parseInt(tokens[1]);
		if (isNaN(this._alarm1hour) || isNaN(this._alarm1minutes)) {      return; 	}
		this.updateChimuinoAlarm1(); 	}
	set alarm1soft(value:number) 		{	this._alarm1soft = value;		this.storage.set('alarm-1-soft', value);		this.updateChimuinoAlarm1();	}
	set alarm1strong(value:number) 		{	this._alarm1strong = value;		this.storage.set('alarm-1-strong', value);		this.updateChimuinoAlarm1();	}
	set alarm1sunday(value:boolean)     { 	this._alarm1sunday = value;     this.updateChimuinoAlarm1(); }
	set alarm1monday(value:boolean)     { 	this._alarm1monday = value;     this.updateChimuinoAlarm1(); }
	set alarm1tuesday(value:boolean)    { 	this._alarm1tuesday = value;    this.updateChimuinoAlarm1(); }
	set alarm1wednesday(value:boolean)  { 	this._alarm1wednesday = value;  this.updateChimuinoAlarm1(); }
	set alarm1thursday(value:boolean)   { 	this._alarm1thursday = value;   this.updateChimuinoAlarm1(); }
	set alarm1friday(value:boolean)     { 	this._alarm1friday = value;     this.updateChimuinoAlarm1(); }
	set alarm1saturday(value:boolean)   { 	this._alarm1saturday = value;   this.updateChimuinoAlarm1(); }
  
  	// on set, update the alarm1 by bluetooth
	private updateChimuinoAlarm1() {
		this.setAlarm1(
	      this._alarm1hour, this._alarm1minutes, 
	      this._alarm1soft, this._alarm1strong,
	      this._alarm1enabled,
	      this._alarm1sunday, this._alarm1monday, this._alarm1tuesday, this._alarm1wednesday, this._alarm1thursday, this._alarm1friday, this._alarm1saturday
	      );
	}

	// send the data to Arduino by bluetooth
	private setAlarm1(hour:number, minutes:number, durationSoft:number, durationStrong:number, enabled:boolean, 
			sunday:boolean, monday:boolean, tuesday:boolean, wednesday:boolean, thursday:boolean, friday:boolean, saturday:boolean) {
		
		let buffer = new ArrayBuffer(12);
		let dv = new DataView(buffer);
		dv.setUint8(0, enabled?1:0); 
		dv.setUint8(1, hour); 
		dv.setUint8(2, minutes);
		dv.setUint8(3, sunday?1:0);
		dv.setUint8(4, monday?1:0);
		dv.setUint8(5, tuesday?1:0);
		dv.setUint8(6, wednesday?1:0);
		dv.setUint8(7, thursday?1:0);
		dv.setUint8(8, friday?1:0);
		dv.setUint8(9, saturday?1:0);
		dv.setUint8(10, durationSoft);
		dv.setUint8(11, durationStrong);
		

		this.write(ChimuinoProvider.SERVICE_CHIMUINO, ChimuinoProvider.CHARACTERISTIC_ALARM1, dv.buffer);
		/*
		this.getConnectedBle().then(
				(connected) => { 
					return this.ble.write(this._device.id, ChimuinoProvider.SERVICE_CHIMUINO, ChimuinoProvider.CHARACTERISTIC_ALARM1, dv.buffer);
				})
			.catch(
				(error) => { this.displayToastMessage("unable to update alarm1: "+error); } );
		*/

	}

	private onAlarm1Received(buffer:any) {

		let dv = new DataView(buffer);
		
		let active:boolean = dv.getUint8(0) > 0;
		let hour:number = dv.getUint8(1);
		let minutes:number = dv.getUint8(2);
		let sunday:boolean = dv.getUint8(3) > 0;
		let monday:boolean = dv.getUint8(4) > 0;
		let tuesday:boolean = dv.getUint8(5) > 0;
		let wednesday:boolean = dv.getUint8(6) > 0;
		let thursday:boolean = dv.getUint8(7) > 0;
		let friday:boolean = dv.getUint8(8) > 0;
		let saturday:boolean = dv.getUint8(9) > 0;
		let durationSoft:number = dv.getUint8(10);
		let durationStrong:number = dv.getUint8(11);
          
        this._alarm1hour = hour;
        this._alarm1minutes = minutes;
   		this._alarm1enabled = active;
    	this._alarm1soft = durationSoft;
    	this._alarm1strong = durationStrong;
    	this._alarm1sunday = sunday;
  		this._alarm1monday = monday;
  		this._alarm1tuesday = tuesday;
  		this._alarm1wednesday = wednesday;
    	this._alarm1thursday = thursday;
    	this._alarm1friday = friday;
   		this._alarm1saturday = saturday;
   		this.alarm1loaded = true;
		//this.displayDebug("received alarm1: "+active+" "+hour+":"+minutes+" "+sunday+monday+tuesday+wednesday+thursday+friday+saturday+" "+durationSoft+" "+durationStrong);

	}


	/**********************************************************************
	 * Alarm 2 data storage and access
	 **********************************************************************/

	// is data related to alarm 1 loaded?
	alarm2loaded:boolean = false;

	// data for alarm 1
	_alarm2hour:number = 7;
	_alarm2minutes:number = 25;
	_alarm2enabled:boolean = false;
	_alarm2soft:number = 10;
	_alarm2strong:number = 15;
	_alarm2sunday:boolean = false;
	_alarm2monday:boolean = true;
	_alarm2tuesday:boolean = true;
	_alarm2wednesday:boolean = true;
	_alarm2thursday:boolean = true;
	_alarm2friday:boolean = true;
	_alarm2saturday:boolean = false;

	// getters for alarm2 data
	get alarm2enabled():boolean 	{ return this._alarm2enabled; 		}
	get alarm2time():string 		{ return (this._alarm2hour<10?"0":"")+this._alarm2hour+":"+(this._alarm2minutes<10?"0":"")+this._alarm2minutes;  }
	get alarm2soft():number 		{ return this._alarm2soft; 			}
	get alarm2strong():number 		{ return this._alarm2strong;		}
	get alarm2sunday():boolean 		{ return this._alarm2sunday; 		}
	get alarm2monday():boolean 		{ return this._alarm2monday; 		}
	get alarm2tuesday():boolean 	{ return this._alarm2tuesday; 		}
	get alarm2wednesday():boolean 	{ return this._alarm2wednesday; 	}
	get alarm2thursday():boolean 	{ return this._alarm2thursday; 		}
	get alarm2friday():boolean 		{ return this._alarm2friday; 		}
	get alarm2saturday():boolean 	{ return this._alarm2saturday; 		}

	// setters for alarm2 data
	set alarm2enabled(value:boolean) 	{	this._alarm2enabled = value;   	this.updateChimuinoAlarm2();	}
	set alarm2time(value:string) 		{ 	
		let tokens = value.split(":");
		this._alarm2hour = parseInt(tokens[0]);
		this._alarm2minutes = parseInt(tokens[1]);
	    if (isNaN(this._alarm2hour) || isNaN(this._alarm2minutes)) {      return; 	}
		this.updateChimuinoAlarm2(); 	}
	set alarm2soft(value:number) 		{	this._alarm2soft = value;		this.updateChimuinoAlarm2();	}
	set alarm2strong(value:number) 		{	this._alarm2strong = value;		this.updateChimuinoAlarm2();	}
	set alarm2sunday(value:boolean)     { 	this._alarm2sunday = value;     this.updateChimuinoAlarm2(); }
	set alarm2monday(value:boolean)     { 	this._alarm2monday = value;     this.updateChimuinoAlarm2(); }
	set alarm2tuesday(value:boolean)    { 	this._alarm2tuesday = value;    this.updateChimuinoAlarm2(); }
	set alarm2wednesday(value:boolean)  { 	this._alarm2wednesday = value;  this.updateChimuinoAlarm2(); }
	set alarm2thursday(value:boolean)   { 	this._alarm2thursday = value;   this.updateChimuinoAlarm2(); }
	set alarm2friday(value:boolean)     { 	this._alarm2friday = value;     this.updateChimuinoAlarm2(); }
	set alarm2saturday(value:boolean)   { 	this._alarm2saturday = value;   this.updateChimuinoAlarm2(); }
  
  	// on set, update the alarm2 by bluetooth
	private updateChimuinoAlarm2() {
		this.setAlarm2(
	      this._alarm2hour, this._alarm2minutes, 
	      this._alarm2soft, this._alarm2strong,
	      this._alarm2enabled,
	      this._alarm2sunday, this._alarm2monday, this._alarm2tuesday, this._alarm2wednesday, this._alarm2thursday, this._alarm2friday, this._alarm2saturday
	      );
	}

	// send the data to Arduino by bluetooth
	private setAlarm2(hour:number, minutes:number, durationSoft:number, durationStrong:number, enabled:boolean, 
			sunday:boolean, monday:boolean, tuesday:boolean, wednesday:boolean, thursday:boolean, friday:boolean, saturday:boolean) {
		
		let buffer = new ArrayBuffer(12);
		let dv = new DataView(buffer);
		dv.setUint8(0, enabled?1:0); 
		dv.setUint8(1, hour); 
		dv.setUint8(2, minutes);
		dv.setUint8(3, sunday?1:0);
		dv.setUint8(4, monday?1:0);
		dv.setUint8(5, tuesday?1:0);
		dv.setUint8(6, wednesday?1:0);
		dv.setUint8(7, thursday?1:0);
		dv.setUint8(8, friday?1:0);
		dv.setUint8(9, saturday?1:0);
		dv.setUint8(10, durationSoft);
		dv.setUint8(11, durationStrong);
		
		// ensure bluetooth is still enabled...
		this.write(ChimuinoProvider.SERVICE_CHIMUINO, ChimuinoProvider.CHARACTERISTIC_ALARM2, dv.buffer);


	}


	private onAlarm2Received(buffer:any) {

		let dv = new DataView(buffer);
		
		let active:boolean = dv.getUint8(0) > 0;
		let hour:number = dv.getUint8(1);
		let minutes:number = dv.getUint8(2);
		let sunday:boolean = dv.getUint8(3) > 0;
		let monday:boolean = dv.getUint8(4) > 0;
		let tuesday:boolean = dv.getUint8(5) > 0;
		let wednesday:boolean = dv.getUint8(6) > 0;
		let thursday:boolean = dv.getUint8(7) > 0;
		let friday:boolean = dv.getUint8(8) > 0;
		let saturday:boolean = dv.getUint8(9) > 0;
		let durationSoft:number = dv.getUint8(10);
		let durationStrong:number = dv.getUint8(11);
          
        this._alarm2hour = hour;
        this._alarm2minutes = minutes;
   		this._alarm2enabled = active;
    	this._alarm2soft = durationSoft;
    	this._alarm2strong = durationStrong;
    	this._alarm2sunday = sunday;
  		this._alarm2monday = monday;
  		this._alarm2tuesday = tuesday;
  		this._alarm2wednesday = wednesday;
    	this._alarm2thursday = thursday;
    	this._alarm2friday = friday;
   		this._alarm2saturday = saturday;
   		this.alarm2loaded = true;
		//this.displayDebug("received alarm1: "+active+" "+hour+":"+minutes+" "+sunday+monday+tuesday+wednesday+thursday+friday+saturday+" "+durationSoft+" "+durationStrong);

	}


	/**********************************************************************
	 * About data storage and access
	 **********************************************************************/

	public versionLoaded:boolean = false;
	public firmwareVersion:String = "???";

	public temperatureLoaded:boolean = false;
	_temperature:number = null;
	_temperatureStr:string = "?";
	get temperature():any{ return this._temperatureStr; }
	

	public uptimeLoaded = false;
	public uptime:String = "???";

	constructor(//public http: HttpClient,
  			  private ble: BLE,
  			  private storage: Storage,
  			  private toastCtrl: ToastController,
  			  private platform: Platform
  			  ) {

		this.platform.pause.subscribe(() => {
		  this.onApplicationLeft();
		});

		this.displayDebug("creating a ChimuinoProvider A");


	    this.platform.ready().then(
	    	(readySource) => {

				// when everything is ready, try to connect bluetooth

 				this.displayDebug("try to enable");

 				return this.ble.enable();

 				/*
	 			return new Promise((resolve) => {

			        this.displayDebug("try to initialize !!!");

			        this.bluetoothle.initialize(resolve, { 
			        	request: true, 
			        	//statusReceiver: false, 
			        	restoreKey: "chimuino"
			        });
			    });*/

		  	})
	    	.then( (enabled) => { this.scanForChimuino(); })
		  	.catch(
 					(error) => { this.displayDebug("error when enabling BLE: "+error); }
 					);


	}

	private scanForChimuino() {
		
		this.displayDebug("scanning for chimuino...");

		this.ble
			.scan(
				[  ],  	// TODO add services  ChimuinoProvider.SERVICE_CHIMUINO
				60		// max search time
				)
			.subscribe(
				(data) => { this.reactScanResult(data) }
				);

		// TODO timeout and detect 

	}

	private reactScanResult(data) {

		//this.displayDebug("found "+data.name+" "+data.id)

		if (data.name == "Chimuino2") {

			this._device = data;

			this.displayDebug("found device "+data.name+" "+data.id);

			this.storage.set('device-name', data.name)
				.then(	(cool) => { return this.storage.set('device-id', data.address); } )
				.catch(	(error) => { this.displayDebug("error when storing device info"); } )
			
			// scanning is costly. Stop now.
			this.ble.stopScan(); // TODO catch?

			this.connect();
		}

	}
	// TODO do not always scan !!!
	/*
	private reactScanResult(result:any) {

		if (result.status === "scanStarted") {
			// ignore it. We did expected it to start anyway.
	    } else if (result.status === "scanResult") {


        	this.displayDebug("found "+result.name+" at: "+result.address);

        	this._device = result.address;

			this.storage.set('device-name', result.name)
				.then((cool) => { return this.storage.set('device-id', result.address); } )
				.catch((error) => { this.displayDebug("error when storing device info"); } )
			
			this.connect();
            this.bluetoothle.stopScan(); // stop at the first chance.

	    }
	}

	private reactScanError(error:any) {
		this.displayDebug("bluetooth scan error: "+error);

	}

	private reactBLEactivation(result:any) {

		this.displayDebug("bluetooth activation: "+status);

		if (result.status == "enabled") {
			// 
			// TODO if in storage... this.storage.get()
			this.scanForChimuino();
			// 
		} else if (result.status =="disabled") {
			// TODO what???
		}
	}
	*/

	/**
	 * when the applicaiton is left, just disconnect bluetooth
	 */
	onApplicationLeft() {
		this.disconnect();
	}

	reactQuietlyByIgnoring() {

	}

	reactAnyError(error:any) {
		this.displayDebug("error: "+error);
	}

	disconnect() {
		if (this._device != null && this._isConnected) {
			// disconnect the device

			// TODO

			/*
			new Promise(function (resolve) {
				this.bluetoothle.disconnect(resolve, {address:this._device});

			}).then(this.reactQuietlyByIgnoring)
				.catch(this.reactAnyError);

			*/
			this.ble.disconnect(this._device.id);

			//this.ble.disconnect(this._device.id);
			this._isConnected = false;
		}
	}

	isConnected():boolean {
		return this._isConnected;
	}

	private displayToastMessage(message:string) {

		let toast = this.toastCtrl.create({
	      message: message,
	      duration: 10000,
	      position: 'top'
	    });
	    toast.present();
	}

	//private 
	displayDebug(message:string) {
		if (this.DEBUG) {
			this.displayToastMessage(message);
		}
	}

	private connect() {

		this.ble
			.connect(this._device.id)
			.subscribe( 
				(device) => { this.reactDeviceConnected(device); },
				(disconnected) => { this.reactDisconnected(); } 
				);

	}

	private reactDisconnected() {

		this._isConnected = false;

		this.displayDebug("we were disconnected from Chime...");

	}

	private read(service:string, characteristic:string, callback) {

		this.ble.read(
			this._device.id, 
			service, 
			characteristic)
		.then(callback)
		.catch(
			(error) => { this.displayDebug("error when reading characteristic "+characteristic+": "+error); } 
			);

	}

	private readAndListen(service:string, characteristic:string, callback) {
		
		this.read(service, characteristic, callback);

		this.ble.startNotification(
			this._device.id, 
			service, 
			characteristic)
		.subscribe( (data) => { callback(data); } );
		
	}

	private reactDeviceConnected() {

		this._isConnected = true;

		// read data, and listen to it 

		this.read(
			ChimuinoProvider.SERVICE_CHIMUINO, 	ChimuinoProvider.CHARACTERISTIC_ALARM1, 
			(data) => { this.onAlarm1Received(data); });
		this.read(
			ChimuinoProvider.SERVICE_CHIMUINO, 	ChimuinoProvider.CHARACTERISTIC_ALARM2, 
			(data) => { this.onAlarm2Received(data); });

		this.readAndListen(
			ChimuinoProvider.SERVICE_SENSING, 	ChimuinoProvider.CHARACTERISTIC_TEMPERATURE, 
			(data) => { this.onTemperatureReceived(data); });

		// send information to the Chimuino
		if (this._firstConnection) {
			this.sendDatetime();
			this._firstConnection = false;
		}

		// start pushing the messages which were pending 
		//this.sendPendingMessages();

	}



	private reactWriteSuccess(data:any) {
		// nothing
	}

	private reactWriteFailure(data:any) {
		this.displayDebug("error writing: "+data);
	}


	private write(service:string, characteristic:string, buffer:any) {

		// TODO enable if disabled?
		if (!this._isConnected) {

			this.ble
				.enable()
				// connect first
				.connect(this._device.id)
				// 
				.subscribe( 
					(device) => {
								this.ble.write(
										this._device.id, service, characteristic, 
										buffer);
								this.reactDeviceConnected(device); 
								},
					(disconnected) => { this.reactDisconnected(); } 
					);
		} else {
			this.ble.write(
				this._device.id, service, characteristic, 
				buffer);	
		}

		
		//	.catch( (error) => { this.displayDebug("error when updating time: "+error); });

	}


	/**
	 * returns a Promise of BLE which should be connected 
	 */
	 /*
	private getConnectedBle() {

			// check bluetooth is enabled
		return this.ble.isEnabled()
			// ... if bluetooth is not enabled, enable it
			.then(
				(enabled) => {
					//this.displayDebug("enabled? "+enabled);

					if (!enabled) return this.ble.enable();
					else return true;
				})
			// check the device is connected
			.then(this.ble.isConnected)
			// ... if not, connect it!
			.then(
				(connected) => {
					this.displayDebug("connected? "+connected);
					if (!connected) return this.ble.connect(this._device.id);
					else return true;
				});
	}*/

	/**
	 * React when the bluetooth device was found; 
	 * this._device is thus set.
	 */
	 /*
	private reactDeviceFound() {

		// display information to the user
		this.displayToastMessage('connecting '+this._device.name+'...');

		this.ble.connect(this._device.id).subscribe(
			(peripherical) => {
				this.displayToastMessage("connected to "+this._device.name);
				this.reactDeviceConnected(); 
			},
			(peripherical) => {
				this.displayToastMessage("disconnected from "+this._device.name+" :-(");
				this._isConnected = false;
			});

	}
	*/

	/**
	 * Called when the temperature changed.
	 * Decodes it from the bytes array and notifies of data
	 */
	private onTemperatureReceived(buffer:any) {

		let dv = new DataView(buffer);
		//this.displayToastMessage("notified by bluetooth: "+ buffer);

		let intval:number = dv.getUint16(0, true)
		let temp:number = intval/100.0;

		this._temperature = temp;
		this._temperatureStr = ""+temp+"°C"; 
		//this.displayDebug("received temperature: "+temp);


	}


	/**
	 * Called when the time changed.
	 * Decodes it from the bytes array and notifies of data.
	 * TODO notice this is useless: we don't care about the time on the chime side, we adapt it anywya
	 */
	private onTimeReceived(buffer:any) {

		let dv = new DataView(buffer);
		
		let year:number = dv.getUint16(0, true);
		let month:number = dv.getUint8(2);
		let day:number = dv.getUint8(3);
		let hour:number = dv.getUint8(4);
		let minutes:number = dv.getUint8(5);
		let seconds:number = dv.getUint8(6);
		
		//this.displayDebug("received date: "+year+"/"+month+"/"+day+" "+hour+":"+minutes+":"+seconds);

	}


/*
	private readAndListen(service:string, characteristic:string, callback_data:(data:any) => void, what:string) {
		this.ble.read(this._device.id, service, characteristic).then(
			(data) => { callback_data(data) }, 
			(error) => { this.displayToastMessage("unable to read "+what);  }
			);
		this.ble.startNotification(this._device.id, service, characteristic).subscribe(
			(data) => { callback_data(data) }, 
			(error) => { this.displayToastMessage("unable to listen "+what); }
		);
	}
*/

	/**
	 * Called when the notification demand fails. 
	 * Usually means we are disconnected.
	 */
	private onNotificationFailure() {
		this._isConnected = false;
		this.displayToastMessage("notification failure :-(");
		//this.attemptReconnect();
	}	

	private sendMessage(wathever) {
		// TODO just to deactivate the old protocol
	}

	// TODO to replace. 

	// ask for characteristics
	askAlarm1()			{ this.sendMessage("GET ALARM1"); 			}
	askAlarm2()			{ this.sendMessage("GET ALARM2"); 			}

	askAmbiance()		{ this.sendMessage("GET AMBIANCE"); 		}
	
	askVersion()		{ this.sendMessage("GET VERSION"); 			}
	
	askSoundThreshold()	{ this.sendMessage("GET SOUNDTHRESHOLD"); 	}
	askSoundLevel()		{ this.sendMessage("GET SOUNDLEVEL");		}
	askSoundEnvelope()	{ this.sendMessage("GET SOUNDENVELOPE");	}
	
	askLightThreshold()	{ this.sendMessage("GET LIGHTTHRESHOLD"); 	}
	askLightLevel()		{ this.sendMessage("GET LIGHTLEVEL");		}
	askLightEnvelope()	{ this.sendMessage("GET LIGHTENVELOPE");	}
	
	askDatetime()		{ this.sendMessage("GET DATETIME");			}
	
	askTemperature()	{ this.sendMessage("GET TEMPERATURE");		}

	askUptime()			{ this.sendMessage("GET UPTIME");			}

	// TODO envelopes

	doChimeLight()		{ this.sendMessage("DO CHIME LIGHT");		}
	doChimeMedium()		{ this.sendMessage("DO CHIME MEDIUM");		}
	doChimeStrong()		{ this.sendMessage("DO CHIME STRONG");		}
	doSnooze()			{ this.sendMessage("DO SNOOZE");			}

	setSoundThreshold(t:number) { this.sendMessage("SET SOUNDTHRESHOLD "+t); }
	setLightThreshold(t:number) { this.sendMessage("SET LIGHTTHRESHOLD "+t); }

	// setters for one Boolean
	setAmbiance(enabled:boolean) { this.sendMessage("SET AMBIANCE "+(enabled?"1":"0")); }


	/*
	* Sets the time of the chimuino to 
	* the current time of the system.
	*/
	sendDatetime() {

		let now = new Date;
		let buffer = new ArrayBuffer(7);
		let dv = new DataView(buffer);
		dv.setUint16(0, now.getFullYear(), true); 
		dv.setUint8(2, now.getMonth()+1);
		dv.setUint8(3, now.getDate());
		dv.setUint8(4, now.getHours());
		dv.setUint8(5, now.getMinutes());
		dv.setUint8(6, now.getSeconds());

		// TODO reconnect if disconnected (unlikely as this happens just after connection)
		this.ble.write(
			this._device.id, ChimuinoProvider.SERVICE_CHIMUINO, ChimuinoProvider.CHARACTERISTIC_TIME, 
			dv.buffer)
			.then( (wathever) => { this.displayDebug("updated Chimuino date & time"); } )
			.catch( (error) => { this.displayDebug("error when updating time: "+error); });

		
	}
 	
}
