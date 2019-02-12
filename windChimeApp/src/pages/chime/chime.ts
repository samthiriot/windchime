import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { Storage } from '@ionic/storage';
import { ChimuinoProvider } from '../../providers/chimuino/chimuino';

@Component({
  selector: 'page-chime',
  templateUrl: 'chime.html'
})
export class ChimePage {

  constructor(public navCtrl: NavController,
  			      private storage: Storage,
              private chimuino: ChimuinoProvider) {

  }

  ionViewWillEnter() {
  }


}
