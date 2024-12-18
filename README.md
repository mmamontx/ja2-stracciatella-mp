# JA2-Stracciatella Multiplayer

This fork of JA2 Stracciatella is an attempt to introduce full scale multiplayer to Jagged Alliance 2. It is based on JA2 Stracciatella project and leverages JA2 1.13 experience. However, here the approach is different from 1.13: rather than introducing a separate multiplayer mode - rewritten and limited to individual skirmishes - this particular implementation directly alters singleplayer campaign (code) to make it support cooperative play.

The implementation approach is simple: start the regular game from the beggining and introduce multiplayer functionality where it is naturally expected. Fix crashes caused by changes and move forward step by step.

Here is the vision: basically JA2 multiplayer should be the same thing as the singleplayer, but enabling other players to observe everything from their PCs and use their own controls. It is just as if the players would play hotseat (like some of us did it as kids), but with 2+ PCs instead of a single one. Considering this the changes are not expected to be too complex and mostly should consist of minor alterations since the original code is not designed for multiplayer. However, there is one thing that does introduce complications: somehow all the game status should be synchronized between the players. JA2 1.13 addresses this by a number of callbacks that manually reflect parameters of individual objects to other players when various game events happen. As the game session moves forward inability to track down everything causes discrepancies between what players observe and, eventually, crashes. Moreover, this approach can't be reasonably used to enable the campaign. This particular fork attempts to use RakNet object replication mechanism to address this problem and handle it in a more natural fashion.

Manifesto:

- Introduce as little custom code as possible, stick to the original JA2 and JA2 Stracciatella code. Leverage the original design and implement on top of its ideas.
- Assimilate with coding styles that surround introduced changes. Don't try to unify - it is outside of the scope.

Roadmap (Milestones):

1. Multiplayer on the strategic screen. So that players can see squad members and chat messages of each other. (Basic functionality done, need to introduce minor fixes.)
2. Multiplayer on the tactical screen:
    1. In the realtime mode. So that players can observe actions of each other and interact. (Object replication concept works, need little debugging and introduce some multiplayer specifics.)
    2. In the battle/turn-based mode.
3. Moving to another sector:
    1. From the tactical screen.
    2. From the strategic screen.
4. Saving and loading.
5. Full demo - from the helicopter in Omerta to Drassen capture.
6. All the rest.

Backlog:

- Regular priority:
    - Make client actions be processed on the server (i.e. so that the clients can trigger actions and observe their effect via replication mechanism).
    - Ensure that every merc action is properly reflected on another side.
    - Let only server start game - grey out time compression for clients. Automatically broadcast game start from server. Add 'ready' buttons (like it is done in 1.13)?
    - Probably it makes sense to consider placing mercs of every player to a separate squad.
    - Replicate all SOLDIERTYPE pointers (see in the bottom).
- Low priority:
    - Root-cause how come the animation is not loaded (see in the bottom).
    - Clear the written message in the box after it's sent to the chat (investigate the issue).
    - Fix the initial game connection screen (place the new buttons more friendly and add captions). Grey out game preferences when client checkbox is marked.
    - Remove intro movies when the game is started (for debugging purposes).
    - Sync with JA2 Stracciatella master.

Things worth mentioning and "dark knowledge":

- For some reason the client doesn't have animations in place when the tactical screen is loading. By default it causes failure so there is a workaround introduced that skips the failure and loads the missing animation instead.
- The original GROUP struct in JA2Types.h has to be renamed to GROUP_JA2 since it caused name conflicts with some standard library included by RakNet and, hence, won't compile. It's not very elegant solution since it caused dramatic amount of misleading changes to occur, but at least it is straightforward and simple. For some reason I didn't manage to do it another way (spent too much time for this).
- Pointers inside SOLDIERTYPE are not replicated. If they are used on the client side it would cause null pointer dereferences (crashes).
- There is a custom HireRandomMercs() function that is automatically called in the game beggining to hire mercs (accelerates debugging so one doesn't have to always visit AIM). This code is only executed for server, the client receives replicated mercs.
- Helicopter drop-off animation is disabled (accelerates debugging).
- For now the enemies have been disabled - until I start implementing the battle/turn-based mode.

Contact: mmamontx@gmail.com
