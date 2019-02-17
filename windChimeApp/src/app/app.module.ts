import { NgModule, ErrorHandler } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { IonicApp, IonicModule, IonicErrorHandler } from 'ionic-angular';
import { IonicStorageModule } from '@ionic/storage';
import { MyApp } from './app.component';

import { InstallPage } from '../pages/install/install';
import { AboutPage } from '../pages/about/about';
import { ChimePage } from '../pages/chime/chime';
import { AlarmPage } from '../pages/alarm/alarm';
import { TabsPage } from '../pages/tabs/tabs';

import { StatusBar } from '@ionic-native/status-bar';
import { SplashScreen } from '@ionic-native/splash-screen';
import { BLE } from '@ionic-native/ble';
import { AndroidPermissions } from '@ionic-native/android-permissions/ngx';

import { ChimuinoProvider } from '../providers/chimuino/chimuino';

@NgModule({
  declarations: [
    MyApp,
    InstallPage,
    AboutPage,
    AlarmPage,
    ChimePage,
    TabsPage
  ],
  imports: [
    BrowserModule,
    IonicModule.forRoot(MyApp, {
      preloadModules: true
    }),
    IonicStorageModule.forRoot()
  ],
  bootstrap: [IonicApp],
  entryComponents: [
    MyApp,
    InstallPage,
    AboutPage,
    ChimePage,
    AlarmPage,
    TabsPage
  ],
  providers: [
    StatusBar,
    SplashScreen,
    BLE,
    AndroidPermissions,
    {provide: ErrorHandler, useClass: IonicErrorHandler},
    ChimuinoProvider
  ]
})
export class AppModule {}
