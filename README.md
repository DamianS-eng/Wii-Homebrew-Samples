# Wii-Homebrew-Samples
A collection of working templates for Wii Homebrew. Most can be extracted and loaded as is.

## About
The purpose of this repository is to hold source files and compiled executables for Wii hardware and emulators. A known working homebrew app was reverse engineered from its source files to successfully recompile and redesign a new app from the structure of the former.

At the time of writing, only three projects compiled successfully with less errors as more projects are completed. 

## Project02-WorkingWiiAppDerivedFromPowerCheck
While not fully optimized, 
Project02 is the broken remnants of the published homebrew app in question, with little purpose aside from proof that the app can compile. 
## Project04-testTwoWiimotePointers
Project04 is a somewhat successful integration of multiple Wiimote pointers on screen while failing to properly utilize GRRLIB's background library.
## Project05-backgroundAndPointer
Project05 has the best compatibility with graphics handling and IR controller input without displaying warnings. Project05 even allows custom images to be compiled as background objects by properly fixing the Makefile.
