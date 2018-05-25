import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { ChimuinoProvider } from '../../providers/chimuino/chimuino';
import { Events } from 'ionic-angular';

@Component({
  selector: 'page-about',
  templateUrl: 'about.html'
})
export class AboutPage {

  public versionLoaded:boolean = false;
  public firmwareVersion:String = "???";

  public temperatureLoaded:boolean = false;
  public temperature:String = "???";
  
  public lightLevelLoaded:boolean = false;
  public lightLevel:String = "???";
  public isDark:String = "???";
  public lightMinMaxLoaded:boolean = false;
  public lightMinMax:String = "?:?";

  public soundLevelLoaded:boolean = false;
  public soundLevel:String = "???";
  public isQuiet:String = "???";
  public soundMinMaxLoaded:boolean = false;
  public soundMinMax:String = "?:?";
  
	constructor(public navCtrl: NavController,
				      private chimuino: ChimuinoProvider,
              private events: Events) {

    // when bluetooth informs us of the availability of information 
    // from the actual chime, then display it
    this.events.subscribe(
      'get-version',   
      (version) => { 
          this.firmwareVersion = version;
          this.versionLoaded = true;
      });

    this.events.subscribe(
      'get-temperature',   
      (temperature) => { 
          this.temperature = String(temperature);
          this.temperatureLoaded = true;
      });

    this.events.subscribe(
      'get-lightlevel',   
      (level,isDark) => { 
          this.lightLevel = String(level);
          this.isDark = (isDark?"DARK":"LIT");
          this.lightLevelLoaded = true;
      });
    this.events.subscribe(
      'get-lightenvelope',   
      (min,max) => { 
          this.lightMinMax = ""+min+":"+max;
          this.lightMinMaxLoaded = true;
      });
    this.events.subscribe(
      'get-soundlevel',   
      (level,isQuiet) => { 
          this.soundLevel = String(level);
          this.isQuiet = (isQuiet?"QUIET":"NOISY");
          this.soundLevelLoaded = true;
      });
    this.events.subscribe(
      'get-soundenvelope',   
      (min,max) => { 
          this.soundMinMax = ""+min+":"+max;
          this.soundMinMaxLoaded = true;
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

  ionViewWillEnter() {
    if (!this.isFullyLoaded()) {
      this.loadInfoFromChimuino();
    }
  }

  isFullyLoaded():boolean {
    return this.versionLoaded && 
           this.lightLevelLoaded && this.lightMinMaxLoaded &&
           this.soundLevelLoaded && this.soundMinMaxLoaded && 
           this.temperatureLoaded;
  }

  loadInfoFromChimuino() {
    if (!this.lightLevelLoaded) { this.chimuino.askLightLevel(); }
    if (!this.lightMinMaxLoaded) { this.chimuino.askLightEnvelope(); }
    if (!this.soundLevelLoaded) { this.chimuino.askSoundLevel(); }
    if (!this.soundMinMaxLoaded) { this.chimuino.askSoundEnvelope(); }
    if (!this.temperatureLoaded) { this.chimuino.askTemperature(); }
    if (!this.versionLoaded) { this.chimuino.askVersion(); }
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
