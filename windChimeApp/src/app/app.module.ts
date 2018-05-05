import { NgModule, ErrorHandler } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { IonicApp, IonicModule, IonicErrorHandler } from 'ionic-angular';
import { IonicStorageModule } from '@ionic/storage';
import { MyApp } from './app.component';

import { InstallPage } from '../pages/install/install';
import { AboutPage } from '../pages/about/about';
import { ContactPage } from '../pages/contact/contact';
import { AlarmPage } from '../pages/alarm/alarm';
import { HomePage } from '../pages/home/home';
import { TabsPage } from '../pages/tabs/tabs';

import { StatusBar } from '@ionic-native/status-bar';
import { SplashScreen } from '@ionic-native/splash-screen';
import { BLE } from '@ionic-native/ble';
import { BluetoothSerial } from '@ionic-native/bluetooth-serial';

@NgModule({
  declarations: [
    MyApp,
    InstallPage,
    AboutPage,
    AlarmPage,
    ContactPage,
    HomePage,
    TabsPage
  ],
  imports: [
    BrowserModule,
    IonicModule.forRoot(MyApp),
    IonicStorageModule.forRoot()
  ],
  bootstrap: [IonicApp],
  entryComponents: [
    MyApp,
    InstallPage,
    AboutPage,
    ContactPage,
    AlarmPage,
    HomePage,
    TabsPage
  ],
  providers: [
    StatusBar,
    SplashScreen,
    BluetoothSerial,
    BLE,
    {provide: ErrorHandler, useClass: IonicErrorHandler}
  ]
})
export class AppModule {}
