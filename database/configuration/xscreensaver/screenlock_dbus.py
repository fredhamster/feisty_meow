#!/usr/bin/python

import dbus
import dbus.service
import dbus.glib
import gobject
import os
import sys

class ScreenDbusObj(dbus.service.Object):
    def __init__(self):
        # stuff the logging into a temporary and hidden directory.
        try:
            os.chdir(os.environ['HOME'] + "/.zz_temp_screenlock_menu")
            print (os.environ['HOME'] + "/.zz_temp_screenlock_menu" + " was already there")
            sys.stdout.flush()
        except:
            print (os.environ['HOME'] + "/.zz_temp_screenlock_menu" + " is not there yet")
            sys.stdout.flush()
            os.mkdir(os.environ['HOME'] + "/.zz_temp_screenlock_menu")
            os.chdir(os.environ['HOME'] + "/.zz_temp_screenlock_menu")
            print (os.environ['HOME'] + "/.zz_temp_screenlock_menu" + " is there now")
            sys.stdout.flush()
        print ("about to create a dbus")
        sys.stdout.flush()
        session_bus = dbus.SessionBus()
        print ("about to setup a bus name")
        sys.stdout.flush()
        bus_name=dbus.service.BusName("org.gnome.ScreenSaver",bus=session_bus)
        print ("about to init dbus service")
        sys.stdout.flush()
        dbus.service.Object.__init__(self,bus_name, '/org/gnome/ScreenSaver')
        print ("after init dbus service")
        sys.stdout.flush()

    @dbus.service.method("org.gnome.ScreenSaver")
    def Lock(self):
        print ("saw the command to lock the screen, about to lock")
        sys.stdout.flush()
        os.system( "xscreensaver-command -lock" )
        print ("issued xscreensaver request to lock the screen")
        sys.stdout.flush()

if __name__ == '__main__':
    print ("into main of screeny")
    sys.stdout.flush()
    object=ScreenDbusObj()
    print ("after creating the screen dbus object, about to go into loop")
    sys.stdout.flush()
    gobject.MainLoop().run()
    print ("after loop, exiting from app")
    sys.stdout.flush()

