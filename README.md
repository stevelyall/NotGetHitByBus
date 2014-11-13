NotGetHitByBus
==============

Help Billy cross the street without dying. Traffic light thing on PIC32 microcontroller.

====================
Project Description
====================
@author mitch9654

Our project will emulate a traffic light system at a crosswalk. When a user pushes the button to cross the road, the traffic light system should change to yellow after a delay, then red, then back to green.

Project Criteria

1) On start, the green light should be on

a) When Button B is pressed, the lights should act as though a person is going to use the crosswalk

i) After a delay, the green light should turn off and yellow should come on

ii) After another delay, the yellow light should turn off and the red light should come on

iii) After a longer delay (of the user using the crosswalk) the red light should turn off and the green light should turn back on

b) When Button A is pressed, the system should go back to its initial phase (reset)

i) The yellow light should be off

ii) The red light should be off

iii) The green light should be on

iv) All timers should be stopped

c) When Button C is pressed, the system should go into “testing” mode

i) The green, yellow and red lights should flash on and off in unison

ii) All unrelated timers should be stopped
