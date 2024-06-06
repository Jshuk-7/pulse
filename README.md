# pulse
stb like thread pool library

To begin using the pulse library:
 1. Copy and paste this file into your project
 
 2. In 'one' .c or .cpp file type the following lines<br/>
    #define PULSE_IMPLEMENTATION<br/>
    #include "pulse.h"
 
 3. Then whenever you need access to the library in another
    .c/.cpp file just '#include "pulse.h"' and you're off!

Things to note:
 
 1. You can '#define PULSE_MAX_THREADS' to a whatever
    you want. It defaults to 100 threads per pool.
