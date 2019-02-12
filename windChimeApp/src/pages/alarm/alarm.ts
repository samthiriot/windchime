import { Component } from '@angular/core';
import { NavController } from 'ionic-angular';
import { Storage } from '@ionic/storage';
import { ChimuinoProvider } from '../../providers/chimuino/chimuino';

// TODO after testing the GUI for alarm1, do the same for alarm2

@Component({
  selector: 'page-alarm',
  templateUrl: 'alarm.html'
})
export class AlarmPage {

  
  constructor(public navCtrl: NavController,
    			    private storage: Storage,
  			      private chimuino: ChimuinoProvider
              ) {
   
  }

  ionViewWillEnter() {

  }
  
  // TODO 
  demoAlarmPrereveil() {
    this.chimuino.doChimeMedium();
  }
  demoAlarmReveil() {
    this.chimuino.doChimeStrong();
  }

}
