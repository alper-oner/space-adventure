# Space Adventure
3D space simulation developed with OpenGL in C++ for BIL 421 Computer Graphics.


# Environment Setup
- Project must be compiled on x86 mode.
# Controls
- Move the agent by pressing left/right keys.
- Quit the simulation by pressing Q.
- Pause the simulation by pressing P
    - if paused, continue the game.
    - if continued, pause the game.
- Increase ship’s speed by pressing D.
- Decrease ship’s speed by pressing A.
- Increase station’s rotational speed by pressing J.
- Decrease station’s rotational speed by pressing K.
- Switch First Person View by pressing C.
- Switch Third Person View by pressing T.
- Switch World View by pressing W.
- Switch Station View by pressing S.
- Turn on/off autopilot mode by pressing F.
- Turn on/off Auto Attack mode by pressing G.
- Turn on/off Auto Target Choose mode by pressing H.
- Show score by pressing M.
- You can select the target ship by pressing [0,1,2,3,4,5,6,7]
- Left Mouse Button
    - if paused, continue the game.
    - if continued, pause the game.
- Right Mouse Button
    - if continued, pause the game.
    - if paused, advance game by 1 frame.
# Features

NPC Ships:

- NPC ships are spawned in random color. Their movements are also random.(Turns left with 5% probability and turns right with 5% probability)
- If you hit them, your score will increase and they will spawn at base planet again.

Auto Pilot Mode:

- When the autopilot mode is activated, the spaceship will automatically navigate towards the station. If you get lost in space, the autopilot will be life saver :)

Auto Attack Mode:

- When the auto attack mode is activated, the main ship(our ship) try to hit other npc ships automatically.

Auto Target Choose Mode:

- When the auto target choose mode is activated, the main ship select the appropriate target ship automatically.

The Space:

- The ground was drawn as a large square on the x-y plane and rendered as an alternating checkerboard pattern of smaller squares.

Planets:

- Planets revolve around themselves.
- If the main ship hit them, your score will set 0.

# Example
![a](https://user-images.githubusercontent.com/36198409/81699149-a0e71180-946f-11ea-9f3b-565d20bf7c77.PNG)
![b](https://user-images.githubusercontent.com/36198409/81699170-a6dcf280-946f-11ea-8a31-e3e72c42c77f.PNG)
![c](https://user-images.githubusercontent.com/36198409/81699175-a7758900-946f-11ea-914e-d872275ffe5b.PNG)

