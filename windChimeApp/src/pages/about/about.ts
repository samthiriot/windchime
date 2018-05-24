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
    this.chimuino.getVersion();
  }


	writeSomething() {
		this.chimuino.writeVersion();
	}

  sendDate()  {
  this.chimuino.sendDatetime();
  }

  listen() {
    this.chimuino.readResult();
  }

	doChime() {
	  	this.chimuino.sendMessage("DO CHIME");
	}



}
