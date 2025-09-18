# hw0-part1

- Creates two processes using `fork()`.
- **Parent (Producer):** generates numbers 1–5, prints them, and sends them through a pipe.
- **Child (Consumer):** receives numbers, prints them, and sends back an acknowledgment.
- A second pipe is used so that the Producer waits for the Consumer each time.
- This ensures strict alternating output.

### Run Instructions
```bash
gcc HW0P1.c -o hw0p1
./hw0p1
