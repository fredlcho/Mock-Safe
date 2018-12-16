# Mock-Safe
Mock safe using LEDS, Attiny 85, and AVR programmer
Software based debounce algorithm on mechanical switches
Timer interrupt checks for mechanical switch input every 5 ms
Safe "unlocks" only when the correct sequence of mechanical switch inputs match initial input of switches
Safe can then accept another sequence of inputs from mechanical switches to enter "locked" state again
