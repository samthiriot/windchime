import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { Storage } from '@ionic/storage';
import { ChimuinoProvider } from '../../providers/chimuino/chimuino';
import { Events } from 'ionic-angular';

@Component({
  selector: 'page-chime',
  templateUrl: 'chime.html'
})
export class ChimePage {

  public isAmbianceLoaded = false;
  private _isChimeEnabled:boolean = true;
  private _chimeLevel:number = 50;

  public isLightThresholdLoaded:boolean = false;
  private _lightThreshold:number = 50;
  
  public isSoundThresholdLoaded:boolean = false;
  private _soundThreshold:number = 50;
  
  public lightLevel:number = 50;

  public soundLevel:number = 50;

  constructor(public navCtrl: NavController,
  			      private storage: Storage,
              private chimuino: ChimuinoProvider,
              private events: Events) {

    // when bluetooth informs us of the availability of information 
    // from the actual chime, then display it
    this.events.subscribe(
      'get-ambiance',   
      (enabled) => { 
          this._isChimeEnabled = enabled;
          this.isAmbianceLoaded = true;
      });
    this.events.subscribe(
      'get-lightthreshold',   
      (value) => { 
          this._lightThreshold = value;
          this.isLightThresholdLoaded = true;
      });
    this.events.subscribe(
      'get-soundthreshold',   
      (value) => { 
          this._soundThreshold = value;
          this.isSoundThresholdLoaded = true;
      });
    this.events.subscribe(
      'get-lightlevel',
      (value) => {
        this.lightLevel = value;
      });
    this.events.subscribe(
      'get-soundlevel',
      (value) => {
        this.soundLevel = value;
      });

    // when bluetooth will be connected, then load info
    this.events.subscribe(
      'connected',
      (isConnected) => {
        if (isConnected) {
          // ask through bluetooth the current settings 
          this.loadInfoFromChimuino();
        } else {
          this.isAmbianceLoaded = false;
          this.isLightThresholdLoaded = false;
          this.isSoundThresholdLoaded = false;
        }
      });  
    
    if (this.chimuino.isConnected()) {
      // maybe the Chimuino is connected already? 
      // in this case, let's load the info right now.
      this.loadInfoFromChimuino();
    }

  }

  loadInfoFromChimuino() {
    this.chimuino.askAmbiance();
    this.chimuino.askLightThreshold();
    this.chimuino.askLightLevel();
    this.chimuino.askSoundThreshold();
    this.chimuino.askSoundLevel();
  }

  set chimeEnabled(value:boolean) {
  	this._isChimeEnabled = value;
  	this.storage.set('chime-activated', value);
    this.chimuino.setAmbiance(this._isChimeEnabled);
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
    this.chimuino.setLightThreshold(this._lightThreshold);
  }

  get soundThreshold():number {
    return this._soundThreshold;
  }
  set soundThreshold(value:number) {
    this._soundThreshold = value;
    this.chimuino.setSoundThreshold(this._soundThreshold);
  }

}
