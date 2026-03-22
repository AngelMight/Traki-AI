

	METAL DETECTOR
	
		every second send 10 pulses, 5kHZ square through TX coil
		record through RX coil as audio
		send audio to app
		
		measure the 3rd harmonic (15kHz) phase shift of every response
		categorize as -90, 0, +90 or +180
		display magnitude and group
		
		send all the pulses to server and receive response
		display response


	hardware
		raspberry pi pico 2 w (RP2350&CYW43)
		battery
		amplifier
		coils

	software
		juce framework application
		
	server
		receive 1 minute 2 cahnnels 48kHz 16bit audio live
		send answer


	BUILD

		Install MSYS2 on your windows
		open ucrt64 terminal (default)
		(maybe install git, make, cmake and so on with pacman)
		cd hardware && ./build.sh
		
		Install Visual Studio 2022 Comunity edition
		open x64 Native Tools Command Prompt
		goto software && build.bat

