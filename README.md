# Lunar Lander

Requirement 1: Player Falls With Gravity 
The player should fall according to an acceleration of gravity that you choose.   

Traditional Lunar Lander games use a small gravity value to simulate outer space, but you may pick any value that fits your theme.

Requirement 2: Moving With Acceleration 
Moving left or right should change the player's acceleration, not velocity. 

When the player releases the movement key, the ship should drift briefly before coming to a stop.

Do not directly update m_movement (or your equivalent variable) 

You should only modify the values in m_acceleration.

Requirement 3: Mission Failed / Mission Accomplished 
If the player touches any forbidden area of the map, display a "Mission Failed" message and end the game. 

If the player successfully lands on a winning platform, display a "Mission Accomplished" message and end the game. 

Include at least one moving platform that the player can interact with to either win or lose.

Requirement 4: Fuel Mechanic 
Implement a fuel system.

Each time the player presses a key to change acceleration, fuel should be consumed.

When the fuel runs out, acceleration keys should stop working.

Include a UI element that displays the remaining fuel.