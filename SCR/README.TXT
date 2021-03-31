From: fdc@watsun.cc.columbia.edu (Frank da Cruz)
Newsgroups: comp.protocols.kermit.misc
Subject: New modem scripts for MS-DOS Kermit 3.15
Date: 28 Dec 1996 00:06:23 GMT  [Updated 20 Apr 1999]
Organization: Columbia University
Lines: 53

A small group of new modem scripts has been assembled for use with MS-DOS
Kermit 3.15 Beta 12 or later.  These new scripts take advantage of many of
the new features in the scripting language: block structure, MINPUT (INPUT
with multiple simultaneous targets), local variables, and so on.  Some have
been tested, some have not.  If you would like to try them out and report
back to kermit@columbia.edu, here are the ones currently available:

 Name        Remarks
  compaq.ksc  Compaq internal modems -- untested
  hayes.ksc   Hayes 1200 and 2400 -- tested on 1200 only
  generic.ksc Generic high-speed modem (RTS/CTS, fixed interface speed)
  megahz.ksc  Megahertz X-Jack V.34 (XJ3144 or CC6144) -- untested
  sport.ksc   US Robotics Sportster or Courier
  supra.ksc   Supra or SupraSonic modems -- untested
  t3000.ksc   Telebit T3000 (or T1600 or Qblazer) -- tested on T3000

The filenames are (of course) in DOS 8.3 format.  The new KSC filetype
stands for "Kermit SCript".  We can't use .SCR any more because in the world
of Windows, that stands for "SCReen saver".

A nice feature of the new scripts is that various parameters can be set
by defining environment or Kermit variables.  These are:

  DIALPORT    - COM port to use for dialing (COM1, etc, default current port)
  DIALSPEED   - Speed for dialing (default depends on script)
  DIALMETHOD  - TONE or PULSE (modem's default method is used by default)
  DIALRETRIES - Maximum times to redial the call (default 5)
  DIALTIMEOUT - How long to wait for result from modem (default 90 seconds)

If any of these variables is not defined, an appropriate default is used.

To test these scripts, put them in the same directory with MSK315.EXE and
make sure that is your current directory.  Then tell MSK315 to:

  define dial set port 1, take xxxx.ksc

Replace "port 1" with "port 2" or whatever port number your modem is
connected to if it is not port 1, and replace "xxxx.ksc" with the name of
the dialing script.  Then tell Kermit to DIAL in the normal way, e.g.:

  dial 7654321

Read the comments in each script for additional details.

- Frank
