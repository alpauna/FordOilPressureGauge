# FordOilPressureGauge
Add actual oil pressure to Ford and allows for dash light to still work via ATTiny uCPU

## Hardware
Uses a board that I developed and all is in schematics folder.

## Software
Arduino based ATTiny85 for original revision.
New revision is ATTiny1624 and it will add buttons so that some items can be adjusted

##Blog
Check out the progress of this in my [alshowto.com](https://alshowto.com) page. Also, you can check out the tag related to the truck that 
inspired this project: [99 f150](https://alshowto.com/tag/99-f150/).

Finally, if just insterested in this project check out the blog on it. [Add an Actual Oil Pressure Gauge](https://alshowto.com/99-f150-add-an-actual-oil-pressure-gauge/)

## Latest development
![20230107_211525](https://user-images.githubusercontent.com/36033796/211179979-201e2979-fc04-431e-b56c-179791744b9d.jpg)
##Testing
Testing the drive capability of the n-mosfet. However for the truck it is simply switching the dash oil circuit to ground so it should be very low current.
![20230108_014446](https://user-images.githubusercontent.com/36033796/211186164-42c93347-14c4-46af-bcac-c4bd4fad60b5.jpg)
##Dash circuit

![image](https://user-images.githubusercontent.com/36033796/211186272-ced22f4c-7b11-4806-a659-a844881b2000.png)
As seen above, I think this circuit is grounded out to turn the oil light on so the code will drive pin 17 high to turn on the MOSFET and this will then send that dash circuit Pin 6 on I/O connector to ground. The code is set to do this if the pressure drops below 10 PSI. I think the original circuit did this at 7 psi. ![currentOilPressureDriverSch](https://user-images.githubusercontent.com/36033796/211186425-7ea2f5f4-173e-420e-b75d-0e3fe2603a36.png)

I will provide more specifics once I fully test this on my trucks. This may be exactly backward and it should be grounded when the light is off and basically open to have the light on. Running on my truck will verify this fact. That will happen sometime next month with my prototype board running the ATTiny85.

![20230103_222853](https://user-images.githubusercontent.com/36033796/211186651-a41a7be0-ad24-49fa-be31-55d16135e91a.jpg)

