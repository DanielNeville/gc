Grind Control Device
====================

<b>Disclaimer: use this design, or any of the parts at your own risk.</b>

This device attemps to prevent night-time bruxism. It works by detecting muscle activation on the temporalis 
muscle and will emit an audio signal to wake the user up if bruxism is detected.

Features:
* Has an audio jack to plug-in headphones. These headphones are use to play an audio tone that will wake up the user during sleep if grinding is detected.
* An [EMG circuit](http://www.advancertechnologies.com/) is used along with custom-made electrodes that are placed on the temporalis muscle.
* A micro-SD card is used to record bruxism patterns for later analysis.
* A custom-made case was designed which can be 3d-printed.

The design is heavily inspired by the commercially available [Grind Care](http://grindcare.com/) device (which appears to no longer be available), except an audio signal is used instead of electrical impulses.

Photos
======

The device is worn around the neck, held by a strap:
![Device](https://raw.github.com/lucwastiaux/gc/master/photos/version1/20130601-P6010001.jpg)

Three electrodes are stuck on the temporalis muscle, the red one on the middle of the muscle, the blue one at the end, and the black one at a neutral area of the skin. Earphones are used for the audio signal:
![Electrodes](https://raw.github.com/lucwastiaux/gc/master/photos/version1/20130601-P6010010.jpg)

The device with strap, earphone, and electrode cable. The case is a custom designed enclosure, designed in Sketchup and printed using Selective Laser Sintering:
![Device 2](https://raw.github.com/lucwastiaux/gc/master/photos/version1/20130601-P6010011.jpg)

Inside of the device, with two 9V batteries:
![Inside](https://raw.github.com/lucwastiaux/gc/master/photos/version1/20130601-P6010012.jpg)

The printed circuit board. You can see the Arduino Micro microcontroller, micro-sd card reader, muscle sensor, switches and 3.5mm headphone jack.
![PCB](https://raw.github.com/lucwastiaux/gc/master/photos/version1/20130601-P6010019.jpg)

The printed circuit board, designed in Eagle:
![PCB](https://raw.github.com/lucwastiaux/gc/master/photos/version1/eagle_board.png)

Schematics in Eagle:
![PCB](https://raw.github.com/lucwastiaux/gc/master/photos/version1/eagle_schematic.png)

History and development
=======================

Symptoms and Diagnosis
----------------------
In October 2010, I started experiencing an unexplained permanent dizziness. It would come and go and be more severe 
during certain weeks. By March 2011, the symptoms had become more severe and included permanent headaches and facial
muscle pain. I started making doctor's appointments, saw three neurologists, one ENT doctor, took blood samples, had an
MRI brain scan but no diagnosis yet. Since the beginning I had been researching an explanation for those symptoms on the
web. Many leads did not pan out but I read that night-time bruxism (unconscious grinding of teeth at night-time) could
be a possible explanations. Several things pointed in that direction:
* I had been told before that I grind my teeth at night.
* My symptoms started one month after the end of an orthodontic treatment. After my orthodontic appliance was removed, I had to wear an Invisalign appliance, which is a transparent plastic mold designed to keep your teeth in place.
* Very quickly after starting to wear the Invisalign appliance, it started to show wear marks at the points of contact of my teeth.

I made an appointment with a New York dentist who made a special mouth guard designed to fit on the front teeth only.
This had a hugely positive impact on my symptoms and two months after wearing that, I believed the problem was gone.

However I occasionally still felt dizziness. In October of 2011, I had ocasional dizziness and semi-permanent headaches again.
This time, I experienced pain when chewing which clearly pointed to an issue with the jaw and especially jaw muscles.
In march 2012, I went to see a dentist in Hong Kong who made another type of mouthguard, and eventually put tooth fillings on my back teeth
to attempt to remediate a malocclusion problem. This had a beneficial impact.

In September 2012, I purchased the Grind Care device which detects night-time bruxism and attemps to condition you out of 
it by triggering a small electric shock. It's a very well made very compact device which you wear at night around your neck.
At electrode is stuck on your face near the temporal muscle. The electrode serves both to monitor muscle activity and
send a shock if muscle contraction is detected.

The grindcare device was the thing that helped me the most up to that point. It would give me some metrics of how many times
I grinded my teeth at night. It started out at 140/night, then progressively moved to 30/night. My symptoms improved radically.
However by December 2012 the number of grinds/night started increasing, as if my body had somehow adjusted and no longer cared
about the electric shock.

The idea started forming in my head that the device should not just send an electric shock, but actually wake me up if 
bruxism is detected.

Development of Grind Control Device
-----------------------------------

In December 2012, I purchased a book on electronics to refresh my knowledge on the topic and learn some of the necessary
techniques such as soldering.





