# JA2-Stracciatella Continued (mmamontx fork)

This fork is an attempt to introduce full scale multiplayer to Jagged Alliance 2. It is based on JA2 Stracciatella project and leverages JA2 1.13 experience. However, here the approach is different from 1.13: rather than introducing a separate multiplayer mode - rewritten and limited to individual skirmishes - this particular implementation directly alters singleplayer campaign (code) to make it support cooperative play.

The implementation approach is simple: start the regular game from the beggining and introduce multiplayer functionality where it is naturally expected. Fix crashes caused by changes and move forward step by step. Here is the vision: basically JA2 multiplayer should be the same thing as the singleplayer, but enabling other players to observe everything from their PCs and use their own controls. It is just as if the players would play hotseat (like some of us did it as kids), but with 2+ PCs instead of a single one. Considering this the changes are not expected to be too complex and mostly should consist of minor alterations since the original code is not designed for multiplayer. However, there is one thing that does introduce complications: somehow all the game status should be synchronized between the players. JA2 1.13 addresses this by a number of callbacks that manually reflect parameters of individual objects to other players when various game events happen. As the game session moves forward inability to track down everything causes discrepancies between what players observe and also crashes. This particular fork attempts to use RakNet object replication mechanism to address this problem and handle it in a more natural fashion.

Roadmap (Milestones):

- Multiplayer on the strategy screen. So that players can see squad members and chat messages of each other. (Basic functionality done, need to introduce minor fixes.)
- Multiplayer on the tactical screen: in the realtime mode. So that players can observe actions of each other and interact. (WIP: introducing object replication mechanism.)
- Multiplayer on the tactical screen: in the battle/turn-based mode.
- Moving to another sector.
- Full demo - from the helicopter in Omerta to Drassen capture.
- All the rest.

Backlog:

- Make object replication work (fix crashes).
- Get updates from JA2 Stracciatella master.
- Clear the written message in the box after it's sent to the chat (investigate the issue).
- For debugging purposes: make N mercs be automatically hired (stable and without cloned/repetitive mercs).
- Fix the initial game connection screen (place the new buttons more friendly and add captions).

Contact: mmamontx@gmail.com

[Below goes original README.md content from JA2 Stracciatella repo.]

[![GitHub CI](https://img.shields.io/github/actions/workflow/status/ja2-stracciatella/ja2-stracciatella/github-ci.yml?branch=master&label=GitHub%20CI&logo=github)](https://github.com/ja2-stracciatella/ja2-stracciatella/actions?query=workflow%3A%22GitHub+CI%22)
[![AppVeyor](https://img.shields.io/appveyor/ci/ja2-stracciatella/ja2-stracciatella/master.svg?style=flat-square&logo=appveyor&label=AppVeyor)](https://ci.appveyor.com/project/ja2-stracciatella/ja2-stracciatella)
[![Coverity Scan](https://img.shields.io/coverity/scan/8431.svg?style=flat-square&label=Coverity%20Scan)](https://scan.coverity.com/projects/ja2-stracciatella-ja2-stracciatella)
[![Current Release](https://img.shields.io/github/downloads/ja2-stracciatella/ja2-stracciatella/v0.21.0/total)](https://github.com/ja2-stracciatella/ja2-stracciatella/releases/tag/v0.21.0)

This is continuation of venerable JA2-Stracciatella project.

The goal of the project is to make Jagged Alliance 2 available on a wide range of platforms, improve its stability, fix bugs and provide a stable platform for mod development.  At the moment the goal is mostly to fix bugs.

[Official Homepage: http://ja2-stracciatella.github.io](http://ja2-stracciatella.github.io)

To get in touch with the developers you can use the [#ja2-stracciatella Discord channel on The Bear's Pit](https://discord.com/invite/GqrVZUM).

## How to start the game

1. Install original Jagged Alliance 2 game on your computer.  Data files from the original game will be used by JA2-Stracciatella

2. [Download JA2-Stracciatella](http://ja2-stracciatella.github.io/download/) or [compile](COMPILATION.md) it from the cloned git repository.

### With the optional launcher

3. Start the launcher and use it to configure the game. It will automatically create the configuration file.

4. Set “JA2 Data Directory” to point to the directory where the original game was installed during step 1. You can manually enter the directory or use the “...” button to browse your computer.

5. If you haven't installed the English version of the original game, you have to select the correct “Game Version” i.e. localization. Note that the game supports two different Russian localizations: RUSSIAN for the “BUKA Agonia Vlasty” release and RUSSIAN_GOLD for the “Gold” release.

### Without the optional launcher

3. Start the game the first time.  It will create the configuration file `%USERPROFILE%\Documents\JA2\ja2.json` on Windows or `~/.ja2/ja2.json` on Unix-like systems.

4. Edit the configuration file and set parameter game_dir to point to the directory where the original game was installed during step 1.  For example, `D:\games\ja2\` (on Windows) or `/home/user/games/ja2-installed` (on Linux).

5. If you installed not the English version of the original game, but one of the localized varieties (e.g. French or Russian), you need to start `ja2.exe` with parameter telling which version of the game you are using.  For example: `ja2.exe -resversion FRENCH`

Supported localizations are DUTCH, ENGLISH, FRENCH, GERMAN, ITALIAN, POLISH, RUSSIAN, RUSSIAN_GOLD. Use RUSSIAN for the “BUKA Agonia Vlasty” release and RUSSIAN_GOLD for the “Gold” release.

If you downloaded a precompiled version of JA2-Stracciatella, the archive may contain a set of bat files for all supported localizations.

Run `ja2.exe -help` for list of available options.

## Development

The project is developed on [github](https://github.com/ja2-stracciatella/ja2-stracciatella).  Pull requests with bug fixes are very welcome.

### How to contribute

The best way to contribute is to make a pull request with a bug fix.  Please see list of open issues [here](https://github.com/ja2-stracciatella/ja2-stracciatella/issues).

The second best way is to file a bug report if you encounter a bug.

### How to make a pull request

1. Fork the project
2. Create a branch (git checkout -b my_feature_branch)
3. Commit your changes (git commit -am "description of your changes")
4. Push to the branch (git push origin my_feature_branch)
5. Create a pull request from your branch into master

Please don't reformat the code for the sake of it, because it will make the merge process harder.  Instead use the following settings in your editor:

- display tab as 8 spaces
- indent with tabs

If you add new code, please don't add spaces after opening or before closing parentheses.

## History of the Project

The original project was run by Tron since 2006.  He did an amazing job of
cleaning up the JA2 sources and making them portable.  The work was massive too -
there are over *7000 commits* in the original svn repository
svn://tron.homeunix.org/ja2/trunk.  Unfortunately, the work on the project
ceased in 2010.  The [original project homepage](http://tron.homeunix.org/ja2)
is no longer available.  Some history can be found in [JA2-Stracciatella
Q&A](http://thepit.ja-galaxy-forum.com/index.php?t=msg&th=13222), or the
[Wayback Machine](https://web.archive.org/web/20140204204243/http://tron.homeunix.org/ja2)

## License

Unless specified explicitly in the commit message, all changes since `commit 8287b98`
are released to the public domain.  All libraries in `dependencies/lib-*`
have their own licenses.

It is not known under which license Tron released his changes.  All we know,
the source codes were publicly available in his svn repository.

The original Jagged Alliance source code was released by Strategy First Inc. in
2004 under the Source Code License Agreement ("SFI-SCLA").  You can find the
license in file *SFI Source Code license agreement.txt*.
