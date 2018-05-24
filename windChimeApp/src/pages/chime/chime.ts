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

    // when bluetooth will be connected, then load info
    this.events.subscribe(
      'connected',
      (isConnected) => {
        if (isConnected) {
          // ask through bluetooth the current settings 
          this.loadInfoFromChimuino();
        } else {
          this.isAmbianceLoaded = false;
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

}
