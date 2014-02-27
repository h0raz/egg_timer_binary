egg_timer_binary
===================


![My image](http://abload.de/img/img_20140227_1413529qj0u.jpg)

This is/will be a egg timer with a battery powered binary display

It shows either the minutes in 6 bits and the seconds or if time is greater than one hour the hour and the minutes and the seconds ticking

If the timer reaches zero it will beep until the rotary encoder is pressed

While the timer is not active you can change the timer by turning the encoder. Start/Pause, Reset is a Push on the encoder

If the seconds are not zero(eg you paused it) and you change the time the seconds will be set to 0

I set the OSCCAL so the drift is minimal and I don't need any crystal



As you can asume this project is still in development

The firmware is functional and finished for now but the layout needs some tweaking and I'm currently searching for a case which has the right size


> **Todo/Bugs:**
> 
> - Power down mode after a timeout
> - generalize Pin/Port settings
> - translate comments etc.
> - sound is a bit aweful
> - PCB desing
> - find case

