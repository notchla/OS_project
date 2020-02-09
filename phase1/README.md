# Phase 0
This is the first part of the project.
The purpose is to write a program that takes the user input from the terminal *term0* and output it in a file, using a printer.

Follow a brief documentation of the functions implemented:
- **tx_status** returns the content of the trans_status register, witch contains an integer representing the status of the "medium".
- **rx_status** same as *tx_status* but for the recv_status register.
- **getchar** returns a character from the term0 input buffer. Waits if the receiver is busy, and errors out if the device isn't ready.
- **term_putchar** takes a character as parameter and puts it in the term0 input buffer. Similarly to *getchar*, checks on the readiness of the medium are performed.
- **print_putchar** similar to *term_putchar*, but instead it writes in the printer data and commands the printer to print the character.
- **term_puts** and **print_puts** perform the same task: they call the functions *term_putchar* and *print_putchar* iteratively over a string.
- **get_line** read one character at a time (**until a max of 256 characters**) and puts it in a buffer (*TERM_BUF*). When the input is over (ended either by sending a '\n' line break or by inputting 256 character), the buffer is returned.
- the code in the code **main** simply asks for input by calling *get_line*, sends it to the *print_puts* function and blocks.