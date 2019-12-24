# PaperTapeReader
Punched paper tape reader - 3D model and source code.

This is punched paper tape reader that allows tape to be pulled through by hand, using the tapes feed holes as a clock/trigger. The tape can be pulled at any speed as long as it is not allowed to slacken.

## Notes
These are some extra notes off the top of my head:

* In the 3D model, the LED emitter hole for the paper tape feed hole is smaller than the rest. This turned out to be a mistake, and the emitter for that hole had to brightened to compensate. It is recommended to modify the "feedHoleRadius" in model/PaperTapeReader.scad to the same as "dataHoleRadius", and export a new .stl from OpenSCAD.

* Setting the correct emitter brightness is important. Ideally it is as bright as possible where the paper still satisfactorily blocks the light where no holes are punched. Test with various resistor values and your own paper tape to get it right.

* I haven't added a schematic, but the wiring should be simple, basically just power the emitters from a 5V out (and an appropriate resistor), and tie the phototransistors to pins 4,5,6,7,8,9,10,11. I used external pull-up resistors on the phototransistors, this was a waste of time as the Arduino Uno has internal pull-ups that can be enabled. The setup() function in src/TapeReader/TapeReader.ino should be changed to enable internal pull-ups.

