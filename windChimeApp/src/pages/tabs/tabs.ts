import { Component } from '@angular/core';

import { AboutPage } from '../about/about';
import { ChimePage } from '../chime/chime';
//import { HomePage } from '../home/home';
import { InstallPage } from '../install/install';
import { AlarmPage } from '../alarm/alarm';

@Component({
  templateUrl: 'tabs.html'
})
export class TabsPage {

  //tab1Root = HomePage;
  tab2Root = AboutPage;
  tabChime = ChimePage;
  tabInstall = InstallPage;
  tabAlarm = AlarmPage;

  constructor() {

  }
  
}
