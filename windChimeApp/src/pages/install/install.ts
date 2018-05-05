import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { Storage } from '@ionic/storage';

// https://github.com/don/BluetoothSerial
import { BluetoothSerial } from '@ionic-native/bluetooth-serial';

// https://ionicframework.com/docs/native/ble/
import { BLE } from '@ionic-native/ble';

@Component({
  selector: 'page-install',
  templateUrl: 'install.html'
})
export class InstallPage {

  public deviceid:string = "";
  public devicename:string = "";

  public pairedDevices = [];
  public discoveredDevices = [];
  
  public buttonScanEnabled:boolean = false;
  public isScanning:boolean = false;


  addDevice(device){
  	if (!device.name) 
  		return;
  	// skip when already open.
  	for (var i in this.discoveredDevices) {
  		var past = this.discoveredDevices[i];
  		if ((past.name == device.name) || (past.id == device.id))
  			return;
  	}
  	var inter = this.discoveredDevices;
	inter.push(device);
	/*
	inter = inter.filter(function(item, pos, self) {
		return (self.indexOf(item) == pos) && (item.name);
	});
	*/
	this.discoveredDevices = inter;
  }

  constructor(public navCtrl: NavController, 
  			  private bluetooth: BluetoothSerial,
  			  private ble: BLE,
  			  private storage: Storage) {
	
	this.isScanning = false;

	// device
	this.storage.get('bluetooth-id').then((val) => { this.deviceid = val; } );
	this.storage.get('bluetooth-name').then((val) => { this.devicename = val; } );
	
	// enable bluetootk
	this.bluetooth.enable();

	// append the paired devices
	this.bluetooth.isEnabled().then((data) => {
		// add paired devices
		this.bluetooth.list().then( (allDevices) => {
			this.pairedDevices = allDevices;
			//this.availableDevices.concat(allDevices);
			//this.addDevices(allDevices);
		});
	});
	
	//this.ble.enable();
	this.ble.isEnabled().then((data) => {
		this.buttonScanEnabled = true;
	});

   
  }

  /**
   *  called when a device was selected by the user.
   */
  connectDevice(device){
  	this.storage.set('bluetooth-name', 	device.name);
  	this.storage.set('bluetooth-id', 	device.id);
  	this.deviceid = device.id;
  	this.devicename = device.name;
  }

  listDevices(){

	this.ble.enable();
	this.ble.isEnabled().then((data) => {
		//this.buttonScanEnabled = true;
		this.isScanning = true;

		this.ble.scan([], 5).subscribe(
	      device => this.addDevice(device)//, 
	      //error => this.scanError(error)
	    );
		var self = this;
        setTimeout(function() {self.isScanning = false}, 5000);

	});
	
	/*
	this.bluetooth.isEnabled().then((data) => {
	       
	    //console.log(data);

		this.isScanning = true;

		this.bluetooth.discoverUnpaired().then((allDevices) => {
			this.addDevices(allDevices);
			
			//var inter = this.availableDevices;
			//inter.concat(allDevices);
			//self.availableDevices = jQuery.unique(inter);

    		
		});
		
	});
	*/
  }

}

