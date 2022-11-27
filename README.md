# TestShooterUnreal
 #Test for some changes in a shooter game made in Unreal Engine 4

 #Added Gameplay Abilities to Shooter Character:

 - Jetpack:
 Double Jump to use jetpack. 
 This ability can be used until the jetpack bar is empty.
 The jetpack bar will charge when jetpack is not in use and the player is on the ground.
 Infos about the Jetpack will be showed in the HUD.

 - Teleport:
 Teleport 10 meters forward by using T key. 
 If teleport is not possible it will not be executed. 
 Teleport can be used every 5 seconds.

 - Time Rewind:
 Press E to Rewind time. 
 Similar to Jetpack, infos about the time rewind will be on screen. 
 Player will be hidden to the others while executing time rewind
 Time Rewind can be used every 5 seconds.

Sounds and particle effects have been added.
All of these functionalities will work Offline and Online with the ClientPrediction technique of Unreal.

A double implementation of the networking part has been made:
1. By using RPCs (commented)
2. By unpacking the moves sent by client (more efficient and works better with the CharacterMovementComponent)
