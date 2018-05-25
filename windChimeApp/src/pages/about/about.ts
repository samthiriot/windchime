import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { ChimuinoProvider } from '../../providers/chimuino/chimuino';
import { Events } from 'ionic-angular';

@Component({
  selector: 'page-about',
  templateUrl: 'about.html'
})
export class AboutPage {

  public firmwareVersion:String = "???";
  public temperature:String = "???";
  public lightLevel:String = "???";
  public isDark:String = "???";

	constructor(public navCtrl: NavController,
				      private chimuino: ChimuinoProvider,
              private events: Events) {

    // when bluetooth informs us of the availability of information 
    // from the actual chime, then display it
    this.events.subscribe(
      'get-version',   
      (version) => { 
          this.firmwareVersion = version;
      });

    this.events.subscribe(
      'get-temperature',   
      (temperature) => { 
          this.temperature = String(temperature);
      });

    this.events.subscribe(
      'get-lightlevel',   
      (level,isDark) => { 
          this.lightLevel = String(level);
          this.isDark = (isDark?"DARK":"LIT");
      });

    
    // when bluetooth will be connected, then load info
    this.events.subscribe(
      'connected',
      (isConnected) => {
        if (isConnected) {
          // ask through bluetooth the current settings 
          this.loadInfoFromChimuino();
        } 
      });  
    
    if (this.chimuino.isConnected()) {
      // maybe the Chimuino is connected already? 
      // in this case, let's load the info right now.
      this.loadInfoFromChimuino();
    }

        /*
		this.chimuino.getVersion()
				 .then(
				 	(version) => { this.firmwareVersion = version; } 
				 	);
          */
	}

  loadInfoFromChimuino() {
    this.chimuino.askVersion();
    this.chimuino.askTemperature();
  }


	askVersion() {
		this.chimuino.askVersion();
	}

  sendDate()  {
    this.chimuino.sendDatetime();
  }

	doChime() {
	  	this.chimuino.doChime();
	}




}
