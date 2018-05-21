import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { ChimuinoProvider } from '../../providers/chimuino/chimuino';

@Component({
  selector: 'page-about',
  templateUrl: 'about.html'
})
export class AboutPage {

	public firmwareVersion:String = "???";

  	constructor(public navCtrl: NavController,
  				private chimuino: ChimuinoProvider) {

          /*
  		this.chimuino.getVersion()
					 .then(
					 	(version) => { this.firmwareVersion = version; } 
					 	);
            */
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
