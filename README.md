# C(ontinued)-MaNGOS -- README
[![Linux Build Status](https://travis-ci.org/cmangos/mangos-classic.svg?branch=master)](https://travis-ci.org/cmangos/mangos-classic) [![Windows Build status](https://ci.appveyor.com/api/projects/status/github/cmangos/mangos-classic?branch=master&svg=true)](https://ci.appveyor.com/project/cmangos/mangos-classic/branch/master)

This file is part of the CMaNGOS Project. See [AUTHORS](AUTHORS.md) file for Copyright information

## Welcome to C(ontinued)-MaNGOS

C(ontinued)-MaNGOS is a free project with the following goal:

  **Doing WoW-Emulation Right!**

This means, we want to focus on:

* WoW-Emulation
    This project is about developing a server software that is able to
    emulate a well known MMORPG service.

* Doing
  * This project is focused on developing content!
  * Also there are many other aspects that need to be done and are
    considered equally important.
  * Anyone who wants to do stuff is very welcome to do so!

* Right
  * Our goal must always be to provide the best code that we can.
  * Being 'right' is defined by the behavior of the system
    we want to emulate.
  * Developing things right also includes documenting and discussing
    _how_ to do things better, hence...
  * Learning and Teaching are very important in our view, and must
    always be part of what we do.

To be able to accomplish these goals, we support and promote:

* Freedom
  * of our work: Our work - including our code - is released under the GPL
    So everbody is free to use and contribute to this open source project
  * for our developers and contributers on things that interest them.
    No one here is telling anybody _what_ to do.
    If you want somebody to do something for you, pay him,
    but we are here to enjoy.
  * to have FUN with developing.

* A friendly environment
  * We try to let personal issues behind us.
  * We only argue about content, and not about thin air!
  * We follow the [Netiquette](http://tools.ietf.org/html/rfc1855).

-- The C(ontinued)-MaNGOS Team!

## ScriptDev2
  ScriptDev2 is a script library, an extention of the scripting capabilities
  that comes with cMaNGOS ( http://cmangos.net/ ), written in C++ and is
  compatible with Windows and Linux. SQL needed for database support both
  MySQL and PostgreSQL.

  This script library provides unique scripts for NPCs, gameobjects, events
  and other that need unique implementation.

  Once ScriptDev2 is compiled it is automatically run by MaNGOS on server
  startup.

  For further information on ScriptDev2, please visit our project subforum
  at http://cmangos.net/forum-32.html

  Documentation on various development related topics can be found in the
  ../doc/scripts docs/ sub directory as well as on the subforum.

  The required SQL files for creating the database backend are included in
  the ../sql/ sub directory. If you are updating from an older ScriptDev2
  version, make sure to take a look at the SQL files provided in the
  ../sql/updates/

## Further information

  You can find further information about CMaNGOS at the following places:
  * [CMaNGOS Forum](http://cmangos.net/)
  * CMaNGOS IRC channel: #cmangos on irc.rizon.net
  * [GitHub repositories](https://github.com/cmangos/)
  * [Issue tracker](https://github.com/cmangos/issues/issues)
  * [Pull Requests](https://github.com/cmangos/mangos-classic/pulls)
  * [Current build status on Travis CI](https://travis-ci.org/cmangos/mangos-classic/)
  * [Wiki](https://github.com/cmangos/issues/wiki) with additional information on installation
  * [Contributing Guidelines](CONTRIBUTING.md)
  * Documentation can be found in the doc/ subdirectory and on the github wiki

## License

  CMaNGOS is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


  You can find the full license text in the file [COPYING](COPYING) delivered with this package.

### Exceptions to GPL

  Some third-party libraries CMaNGOS uses have other licenses, that must be
  uphold.  These libraries are located within the dep/ directory

  In addition, as a special exception, the CMaNGOS project
  gives permission to link the code of its release of MaNGOS with the
  OpenSSL project's "OpenSSL" library (or with modified versions of it
  that use the same license as the "OpenSSL" library), and distribute
  the linked executables.  You must obey the GNU General Public License
  in all respects for all of the code used other than "OpenSSL".  If you
  modify this file, you may extend this exception to your version of the
  file, but you are not obligated to do so.  If you do not wish to do
  so, delete this exception statement from your version.
