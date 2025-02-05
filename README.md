# JA2-Stracciatella Multiplayer

This project is a fork of JA2 Stracciatella introducing multiplayer to Jagged Alliance 2. It is based on JA2 Stracciatella project and leverages JA2 1.13 multiplayer experience. However, here the approach is different from 1.13 MP: rather than introducing a separate multiplayer game mode - rewritten and limited to individual skirmishes - this particular implementation directly alters singleplayer campaign (code) to make it support cooperative play.

## Vision

Basically, JA2 Stracciatella multiplayer is the same thing as the singleplayer, but enabling other players to observe everything from their PCs and use their own controls. It is just as if the players would play hotseat (like some of us did it as kids when we took turns to tell "our" merc what to do), but with 2+ PCs instead of a single one. Considering this the changes are not expected to be too complex and mostly would consist of minor alterations caused by the fact that the original code is not designed for multiplayer. At the same time it is rather "immersive", which makes it relatively easy to be adapted for handling MP logic. The cornerstone of this is that somehow all the game status has to be synchronized between the players. JA2 1.13 handles this rather plainly by a number of callbacks that manually reflect parameters of individual objects to other players when various game events happen. Since the code is big, as the game session moves forward, the inability to track down everything causes discrepancies between what players observe and, eventually, crashes. Moreover, this approach would probably require too much "manual" effort to enable the campaign. Here, RakNet object replication mechanism is used to address this problem and handle it in a more natural fashion. The implementation approach is straightforward: start the regular game from the beginning and introduce multiplayer functionality where it is naturally expected. Fix crashes caused by changes; if necessary, add supporting code and structures; and move forward step by step.

## Manifesto

- Introduce as little custom code as possible.
- Stick to the original JA2 (JA2 Stracciatella) code. Leverage the original design and implement on top of its ideas.
- All the new weapons and extra features of 1.13 and other modifications on top of the vanilla JA2 (JA2 Stracciatella vanilla) are outside of the scope of this project.
- The axioms of style from the original CONTRIBUTING.md of JA2S mostly remain valid (except for the rule about dead code - some of the original lines are commented out and remain for reference and convenient roll back). Assimilate with coding styles that surround introduced changes. Don't try to unify or refactor unless it's really necessary - it is outside of the scope as well.

## Roadmap (Milestones) - from the player perspective

1. Multiplayer on the strategic screen. So that players can see squad members and chat messages of each other. **Basic functionality - for game beginning before the helicopter drop in Omerta - is done, need to introduce "cosmetic" fixes and do some extra testing.**
2. Multiplayer on the tactical screen:
    1. In the real-time mode. So that players can observe actions of each other and interact. **The object replication concept works well. Now offloading all the possible client (merc) actions to the server side so it can then be broadcasted to everyone - mostly done.**
    2. In the battle/turn-based mode.
3. Moving to another sector:
    1. From the tactical screen.
    2. From the strategic screen.
4. Saving and loading.
5. Alpha demo - from the helicopter in Omerta to Drassen capture.
6. All the rest.

## Backlog

- Regular priority:
    - Implement the remaining RPC actions from the client:
        - Opening doors.
        - Picking up items (after ground items are replicated - see below).
        - Interacting with chests and taking/placing items there.
        - Healing.
        - Throwing grenades and knifes.
        - Placing charges.
        - Talking (so that the effects of talking propagate to the server).
        - Propagate stealth mode.
        - Cutting fences.
        - Etc.
    - Replicate ground items.
    - Propagate time (including time compressions) from the server to clients.
- Low priority:
    - Make IsValidTalkableNPC() support RPCs.
    - Implement respective item pointers for multiple clients: replace gpItemPointerRPC.
    - Moving and equipping items in the inventory between different mercs.
    - Multi-selection actions.
    - When climbing to and back from roofs change the level only for the player that initiated it.
    - For fun: It seems that originally developers considered enabling jumps over the windows and left the corresponding code in place. Try to extend climbing with this ability (remove it or make it optional in the release).
    - Testing: manual and automated.
    - Fix the bug when client mercs do not arrive in time and become inaccessible.
    - Fix the sporadic client crashes when it connects to the lobby.
    - Replicate all SOLDIERTYPE pointers that it makes sense to. (Or wait until it will be resolved by itself.)
    - Assign every player to a separate squad (first - automatically give a single squad, then - introduce a control so that the server can do it manually and assign multiple squads to a single player). Only the server should be able to assign mercs to the squads. Prevent selecting and controlling mercs from squads of other players.
    - Reset the time compression button when a new player connects to the lobby.
    - Root-cause: how come the animation is not loaded for the client (see in the bottom).
    - Clear the written message in the box after it's sent to the chat (investigate and fix the issue).
    - Fix player name appearance in the chat on client side.
    - Make messages from client be broadcasted and received from the server before they appear in the chat on client side.
    - Fix the initial game connection screen (place the new buttons more friendly and add captions). Grey out game preferences when the client mode checkbox is marked.
    - Verify successful connection before changing gConnected on client side.
    - Handle server disconnection on client side.
    - Fix "burst fatality" crash.
    - Investigate (and fix if needed) AnimCache memset() crash.
    - Fix columns width and add borders for MP buttons on the strategic screen. Adapt for all resolutions (including the widescreens).
    - Add player names and the respective ready statuses to the columns.
    - Verify repetitive starts and connections/disconnections within a single run.
    - Fix gPlayers struct on disconnections.

Things worth mentioning and "dark knowledge":

- The modification requires images of buttons from 1.13 MP. They are located under the following path: Data/Interface/MPGOLDPIECEBUTTONS.sti
- For some reason the client doesn't have animations in place when the tactical screen is loading. By default it causes failure, so there is a workaround introduced that skips the failure and loads the missing animation instead.

Changes compared to the vanilla game (from the player perspective):

- Starting screen is the strategic map (lobby) instead of the laptop.
- The intro videos at the beginning of the game are disabled.
- Helicopter drop-off animation is disabled (see the FIXME in Merc_Entering.cc) - it accelerates debugging, and it's overall questionable whether for multiplayer it makes sense to keep it enabled.
- The additional table on the strategic map (re-consider later).

For debugging purposes:

- There is a custom function - HireRandomMercs() - that is automatically called in the beginning (accelerates debugging so one doesn't have to always visit AIM). This code is only executed for the server, the client receives replicated mercs.
- The enemies are disabled (see gEnemyEnabled var) - until implementation of the battle/turn-based mode is started.

Contact: mmamontx@gmail.com
