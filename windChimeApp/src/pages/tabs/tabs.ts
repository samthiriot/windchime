import { Component } from '@angular/core';

import { AboutPage } from '../about/about';
import { ContactPage } from '../contact/contact';
import { HomePage } from '../home/home';
import { InstallPage } from '../install/install';
import { AlarmPage } from '../alarm/alarm';

@Component({
  templateUrl: 'tabs.html'
})
export class TabsPage {

  tab1Root = HomePage;
  tab2Root = AboutPage;
  tab3Root = ContactPage;
  tabInstall = InstallPage;
  tabAlarm = AlarmPage;

  constructor() {

  }
  
}
