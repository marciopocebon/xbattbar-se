`xbattbar-se`
=============

**Yet another Battery/UPS/AC/DC power watcher for X11** (suckless edition).

This directory contains source for the *suckless edition* of
[`xbattbar`](https://gitlab.com/chinarulezzz/xbattbar) tool. This means that the
project is aimed at simplicity, clarity, and frugality.

There's nothing here but a battery panel that takes the necessary data from
`/sys/class/power_supply/*`. I think `*BSD family` also has kernel interfaces
that provide information about the battery status, I don't know and haven't tested.
You're welcome.

`xbattbar-se` requires the following data:

| Name in config.def.h      | Expected | Description                   |
|---------------------------|----------|-------------------------------|
| BattFull                  | `\d+`    | full capacity value           |
| BattNow                   | `\d+`    | momentary/instantaneous value |
| PowerState                | `(0\|1)` | power supply online status    |

See `config.def.h` and `/usr/src/linux/Documentation/power/power_supply_class.txt`
for more details and examples.

Note that this can be obtained using scripts and written to certain files,
which can then be fed to `xbattbar-se`.

_The program does not work with multiple batteries_, but you can configure and run
multiple instances of `xbattbar-se`, where each instance will indicate a separate
battery.

In the current directory you will find some _patches_ that extend the basic
functionality.

**Version**

0.1.2

**Requirements**

- basic configuration
  - `Xlib`
- `0001-add-popup-diagnosis-window.patch`: add popup window with battery remaining percent
  - `Xlib`
- `0002-add-libnotify-support.patch`: display notification when battery low
  - `libnotify` (requires `gdk-pixbuf-2.0`, `gio-2.0`, `gobject-2.0`, `glib-2.0`)

**Configuration**

Configuration is done with `config.h`:

	make config.h

Read the comments in the generated `config.h` to edit it according to your needs.
Defaults are stored in `config.def.h`.

**Installation**

Edit config.mk to match your local setup (*xbattbar-se is installed into
the /usr/local namespace by default*).

Afterward, enter the following command to build and install xbattbar-se
(*if necessary as root*):

    make clean install

**Authors**

- Copyright (c) 1998-2001  Suguru Yamaguchi  <suguru@wide.ad.jp>
- Copyright (c) 2018-2018  Alexandr savca    <alexandr.savca89@gmail.com>

