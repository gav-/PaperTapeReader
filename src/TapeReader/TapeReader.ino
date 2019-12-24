/**
 * 1" Punched Paper Tape Reader developed on Arduino UNO with 1.6.5-r5 IDE.
 * 
 * @author Gavin Stewart <gavin@stewart.com.au>
 * 20151003
 */

#define BUILDSERIAL "20151004"

/**
 * Configurable constants.
 */
const uint8_t feedInterrupt = 1;              // Pin 3 on Uno.
const uint8_t dataPins[] = { 4, 5,  6,  7,    // High nybble of Atmega 328 PORT-D (Uno PIND).
                             8, 9, 10, 11 };  // Low nybble of Atmega 328 PORT-B (Uno PINB).
HardwareSerial& useSerial = Serial;           // Serial port we are using, e.g. Serial on Uno, Serial1 on Mega.
const uint32_t serialSpeed = 115200;          // 115200 appears good enough to avoid input data overflow.

// How we read a byte from input registers (Atmega PORTs).
// Note that low value is a hole (1), and high value is no hole (0), so we complement/invert the result (~).
#define READBYTE() ~(((PIND & 0xf0) >> 4) | ((PINB & 0x0f) << 4))  

/**
 * Non-configurable constants and globals.
 */
const uint8_t dataPinsSize = sizeof(dataPins) / sizeof(dataPins[0]);  // Length of dataPins[] array.

volatile uint8_t data;                        // Byte as read by feedSenseISR().
volatile bool dataAvailable;                  // Set by feedSenseISR() when new data available, must be cleared when read.
volatile bool dataOverflow;                   // Set by feedSenseISR() if dataAvailable is still set on next interrupt.

enum State {
  STATE_IDLE,
  STATE_DUMP,
  STATE_INTERRUPT,
};

uint8_t state;              // Current state.

enum Dump {
  DUMP_NONE,
  DUMP_HEX,
  DUMP_TAPE,
};

uint8_t dump;               // Current dump type.
uint8_t dumpColumns;        // Number of columns we have dumped to terminal so far (for wrapping).
uint32_t dumpAddress;       // Line number for hex dumps. 32bit - would like to see 4GB tape!

uint8_t dataFIFO[3];        // FIFO queue to combine two sensor columns at different data ingestion times.
bool dataSkipNulls;         // Set at start of dump, then unset on first non-null byte received.

/**
 * Banner and help menu.
 */
void printHelp () {
  useSerial.print(F(
    "\n"
    "+----------------------------------------+\n"
    "|      1\" Punched Tape Reader            |\n"
    "|          Build: " BUILDSERIAL "               |\n"
    "|                                        |\n"
    "|  Copyright (C) 2015                    |\n"
    "|  Gavin Stewart <gavin@stewart.com.au>  |\n"
    "+----------------------------------------+\n"
    "\n"
  ));

  useSerial.print(F(
    " ?  - This help.\n"
    " h  - Hexdump format. Use \"xxd -r\" to convert to binary.\n"
    " s  - Show current holes under head (debug).\n"
    " t  - Tape-like output with hex, decimal, octal, ASCII.\n"
    " <any key> - Interrupt the current dump.\n"
    "\n"
  ));
  
  printPrompt();
}

/**
 * Input prompt.
 */
void printPrompt () {
  useSerial.print(F("> "));
}

/**
 * Arduino required setup function.
 */
void setup () {
  // Use a fast enough speed, too slow increases chance of input data overflow.
  useSerial.begin(serialSpeed);

  // Initialise data input pins. We have external pull-up resistors, so they
  // are not enabled here.
  for (uint8_t i = 0; i < dataPinsSize; i++) {
    pinMode(dataPins[i], INPUT);
  }
  
  printHelp();

  // Initial state.
  state = STATE_IDLE;
  dump = DUMP_NONE;
}

/** 
 * Feed hole sense Interrupt Service Routine. 
 * 
 * Called each time the feed hole sensor triggers a falling edge (a hole appears).
 * The eight data sensor values are read into a byte.
 */
void feedSenseISR () {
  // Detect data overflow condition.
  if (dataAvailable) {
    dataOverflow = true;
  }

  // Read a byte and set the notification flag.
  data = READBYTE();
  dataAvailable = true;
}

/**
 * Data dump initialisation routine.
 * 
 * @param uint8_t dumpType
 *  See Dump enum.
 */
void dumpInit (uint8_t dumpType) {
  useSerial.println(F("+ Dump started."));
  state = STATE_DUMP;
  dump = dumpType;
  dumpColumns = 0;
  dumpAddress = 0;
  dataAvailable = false;
  dataOverflow = false;
  dataFIFO[0] = 0;
  dataFIFO[1] = 0;
  dataFIFO[2] = 0;
  dataSkipNulls = true;
  attachInterrupt(feedInterrupt, feedSenseISR, FALLING);
}

/**
 * Data dump cleanup routine.
 */
void dumpStop () {
  detachInterrupt(feedInterrupt);
  state = STATE_IDLE;
  dump = DUMP_NONE;
  dataAvailable = false;
  dataOverflow = false;
  useSerial.println(F("\n+ Dump stopped."));
  printPrompt();
}

/**
 * Dump the provided val to serial port in the configured dump format.
 * 
 * @param uint8_t val
 *  Value to dump to serial port.
 */
void dumpData (uint8_t val) {
  char buff[26];      // Note this maximum output buffer size including terminating null!
  uint8_t pos = 0;    // Cursor position in buffer.
  const char hex[] = "0123456789abcdef";  // Hex translation string.
  #define MAXDUMPCOLUMNS 16               // Number of output columns
  
  static char asciiBuff[MAXDUMPCOLUMNS + 3 + 2];    // Persistent buffer for hexdump ascii field,
                                                    // space, 2x'|' borders, newline and null.
  static char asciiBuffPos;                         // Persistent position in hexdump ascii buffer.

  //Serial.print("Address:");
  //Serial.println(dumpAddress, HEX);
  
  switch (dump) {
         
    case DUMP_HEX:
      // Check for start of new line.
      if (!(dumpAddress % MAXDUMPCOLUMNS)) {
        // Build up 32-bit address field in hex.
        buff[pos++] = hex[(dumpAddress >> 28) & 0xf];
        buff[pos++] = hex[(dumpAddress >> 24) & 0xf];
        buff[pos++] = hex[(dumpAddress >> 20) & 0xf];
        buff[pos++] = hex[(dumpAddress >> 16) & 0xf];
        buff[pos++] = hex[(dumpAddress >> 12) & 0xf];
        buff[pos++] = hex[(dumpAddress >>  8) & 0xf];
        buff[pos++] = hex[(dumpAddress >>  4) & 0xf];
        buff[pos++] = hex[(dumpAddress >>  0) & 0xf];
        buff[pos++] = ' ';
        buff[pos++] = ' ';

        // Reset ascii buffer.
        asciiBuffPos = 0;        
        asciiBuff[asciiBuffPos++] = ' ';             
        asciiBuff[asciiBuffPos++] = '|';
      }

      // Dump byte in hex.
      buff[pos++] = hex[val >> 4];
      buff[pos++] = hex[val & 0x0f];
      buff[pos++] = ' ';
      dumpColumns++;

      // Store ASCII.
      asciiBuff[asciiBuffPos++] = (val >= 33 && val <= 126) ? val : '.';
      
      break;
      
    case DUMP_TAPE:    
      // Tape edge. (1)
      buff[pos++] = '|';
      
      // First three holes (LSb first) (4)
      for (uint8_t i = 0; i < 3; i++) {
        if (val & (1 << i)) {
          buff[pos++] =  'o';
        } else {
          buff[pos++] = ' ';
        }
      }
      
      // Feed hole (5)
      buff[pos++] = '.';
      
      // Last five holes (MSb last) (10)
      for (uint8_t i = 3; i < 8; i++) {
        if (val & (1 << i)) {
          buff[pos++] =  'o';
        } else {
          buff[pos++] = ' ';
        }
      }

      // Tape edge. (11)
      buff[pos++] = '|';

      // Hex. (14)
      buff[pos++] = ' ';
      buff[pos++] = hex[val >> 4];
      buff[pos++] = hex[val & 0x0f];

      // Decimal. (18)
      buff[pos++] = ' ';
      buff[pos++] = (val / 100) + '0';
      buff[pos++] = (val % 100 / 10) + '0';
      buff[pos++] = (val % 10) + '0';

      // Octal. (22)
      buff[pos++] = ' ';
      buff[pos++] = ((val >> 6) & 7) + '0';
      buff[pos++] = ((val >> 3) & 7) + '0';
      buff[pos++] = ((val >> 0) & 7) + '0';
      
      // ASCII. (24)
      buff[pos++] = ' ';
      buff[pos++] = (val >= 33 && val <= 126) ? val : '.';

      // Newline. (25)
      buff[pos++] = '\n';
      
      break;
  }

  // String terminator at last position.
  buff[pos] = '\0';

  // Output dump data buffer.
  useSerial.print(buff);

  // After MAXDUMPCOLUMNS data columns, output hexdump ascii field with a newline.
  if (dumpColumns >= MAXDUMPCOLUMNS) {
    asciiBuff[asciiBuffPos++] = '|';
    asciiBuff[asciiBuffPos++] = '\n';
    asciiBuff[asciiBuffPos] = '\0';   // ascii field string terminator.
    useSerial.print(asciiBuff);
    dumpColumns = 0;      // Reset, will be starting on a new line.
  }

  // Increment dump address.
  dumpAddress++;
}

/**
 * Read character from serial port and handle it.
 */
void handleInput () {
  char inp = useSerial.read();

  // Any key stops a dump.
  if (state == STATE_DUMP) {
    dumpStop();  
    return;
  }

  useSerial.println(inp);       // Echo input.

  uint8_t underHead;            // For 'd' - showing representation of holes under read head.
  switch(inp) {
    case 'h':
      dumpInit(DUMP_HEX);
      break;
    case 's':
      dump = DUMP_TAPE;
      underHead = READBYTE();
      dumpData(underHead & B10101101); // Column 2 sensor data.
      dumpData(0);                     // Column with no sensors.
      dumpData(underHead & B01010010); // Column 1 sensor data.
      dump = DUMP_NONE;
      printPrompt();
      break;
    case 't':
      dumpInit(DUMP_TAPE);
      break;
    case '?':
    default:      
      printHelp();
      break;
  }
}

/**
 * Arduino required loop function.
 */
void loop () {

  // Check for overflow condition.
  if (dataOverflow) { 
    dumpStop();
    useSerial.println(F("** Error: input data overflow detected."));
  }

  // Check for new data available.
  if (dataAvailable) {
    
    // Disable interrupts, copy new data, reenable interrupts ASAP.
    noInterrupts();
    uint8_t val = data;       // Make a copy of the new data.
    dataAvailable = false;    // Unset dataAvailable flag for feedSenseISR().
    interrupts();

    // Push data through FIFO (used for combining read head sensor columns).
    dataFIFO[2] = dataFIFO[1];
    dataFIFO[1] = dataFIFO[0];
    dataFIFO[0] = val;

    // Combine sensor column data:
    //
    // Column 1 (C1) has the feed sensor (S) and bits: 1, 4, 6 
    // Column 2 (C2) has the remaining bits:           0, 2, 3, 5, 7
    //
    // C1  C2
    // ------
    //     0
    //  1
    //     2      ----------------->
    //  S           tape direction
    //     3      ----------------->
    //  4
    //     5
    //  6
    //     7
    // ------
    //
    // As tape moves from left to right, an entire byte is not available until the row of holes has
    // passed both C1 and C2 sensor columns. The bits read from the row of holes under C2 "now" must be 
    // combined with the bits read from the row of holes under C1 two "feed clocks" ago.
    //
    // Note: Tape holes are least significant bit (LSb) at top, and most significant bit (MSb) at bottom.
    //       Bitmasks used below are naturally read MSb left to LSb right.
    //
    //    (  C1 from 2 clocks ago ) | (      C2 from now      )
    val = (dataFIFO[2] & B01010010) | (dataFIFO[0] & B10101101);

    if (dataSkipNulls && val == 0) {
      // We don't want to know about nulls at the beginnig of a tape dump.
    } else {
      dataSkipNulls = false;    // From now on all nulls must be dumped.
      dumpData(val);
    }
  }

  // Check for serial port input.
  if (useSerial.available()) {
    handleInput();
  }
  
}


