import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { Storage } from '@ionic/storage';
import { ChimuinoProvider } from '../../providers/chimuino/chimuino';

@Component({
  selector: 'page-chime',
  templateUrl: 'chime.html'
})
export class ChimePage {

  private _isChimeEnabled:boolean = true;
  private _chimeLevel:number = 50;

  constructor(public navCtrl: NavController,
  			   private storage: Storage,
           private chimuino: ChimuinoProvider) {

	this.storage.get('chime-activated')
				.then((val) => { this._isChimeEnabled = val; } );
	this.storage.get('chime-level')
				.then((val) => { this._chimeLevel = val; } );
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
