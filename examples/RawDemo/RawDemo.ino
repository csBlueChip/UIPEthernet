#include <UIPEthernet.h>
#include <utility/logging.h>
#include <utility/uip.h>
#include <utility/uip_arp.h>

//------------------------------------------------------------------------------ ---------------------------------------
#define APPNAME  "RawDemo"

#define VER_MAJ  1
#define VER_MIN  0
#define VER_SUB  0

// Stringify the version number -> VERS
#define STR(s)   #s
#define XSTR(s)  STR(s)
#define VERS     "v" XSTR(VER_MAJ) "." XSTR(VER_MIN) "." XSTR(VER_SUB)

//------------------------------------------------------------------------------ ---------------------------------------
// You many need to change the value of ARDPIN2 {ANODE, CATHODE} to reflect whether
//     your ARDuino PIN is connected TO the ANODE or CATHODE of the LED [RAP: Red, Anode, Postive]
//
#define ANODE    0
#define CATHODE  1
#define ARDPIN2  CATHODE  // <--- Choose ANODE or CATHODE

#if   (ARDPIN2==ANODE)
#   define LED_ON   HIGH
#   define LED_OFF  LOW

#elif (ARDPIN2==CATHODE)
#   define LED_ON   LOW
#   define LED_OFF  HIGH

#else
#   error Choose ANODE or CATHODE
#endif

#undef ARDPIN2
#undef CATHODE
#undef ANODE

//------------------------------------------------------------------------------ ---------------------------------------
// Preprocessor assertions
//
#define CASSERT(predicate, file)  _impl_CASSERT_LINE(predicate,__LINE__,file)
#define _impl_PASTE(a,b)          a##b
#define _impl_CASSERT_LINE(predicate, line, file) \
    typedef char _impl_PASTE(assertion_failed_##file##_,line)[2*!!(predicate)-1];
    
//------------------------------------------------------------------------------ ---------------------------------------
// I hate typing: SerialUSB.println(F("message"));
// I prefer       FSAYLN("message");
// ...This also means, you can move the outpout device by editing STDOUT
//
#define STDOUT      SerialUSB

#define SAY(...)    STDOUT.print(__VA_ARGS__)
#define SAYLN(...)  STDOUT.println(__VA_ARGS__)

#define FSAY(s)     STDOUT.print(F(s))
#define FSAYLN(s)   STDOUT.println(F(s))

//------------------------------------------------------------------------------ ---------------------------------------
// Get size of a typedef'ed struct'ure member
//
#define SIZEOFM(type,member)  (sizeof(((type*)0)->member))

//------------------------------------------------------------------------------ ---------------------------------------
// Because this demo sends a pure Ethernet packed (not TCP nor UDP (etc.)), we will never actually use the IP Address
// ...but we have to have *something* or UIPEthernet will DHCP
// These details would be better stored in EEPROM with some kind of Serial Control routine to configure it
// ...but this is just a demo of sending raw packets
//
IPAddress      myIP     = {192, 168, 0, 188};    // Default address
unsigned char  myMAC[6] = {0x02, 0xAA, 0xAA, 0xAA, 0xBC, 0x01};

//------------------------------------------------------------------------------ ---------------------------------------
// The overhead of an ethernet frame is 18 bytes
// If your data block is < (64-18)=46 bytes, it will be padded with 0x00's
// Your data block MUST be <= 1500 bytes
// The Ethernet overhead will take this to 1518 
// Most networks have an MTU of 1500, 
// ...So I suggest your data block is < (1500-18)=1482
// ...And even THAT may cause "fragmentation" on some networks
// This code has NOT been tested for tolerance to Fragmentation!
//
typedef
  struct myData {
    uint32_t  tag;  // Magic number for a datum
    uint16_t  len;  // Size of datum
    struct    val {
      // Your datum
      uint32_t  i;
      char      c;
      char      pad[22-5];  // My test required a 22 byte "value"
      //...
    } __attribute__ ((packed))  val ;
  } __attribute__ ((packed))
myData_t;

// How many copies of your data you want in a single Ethernet frame
#define DATA_CNT (2)  

// This is the frame which will be sent
// It contains an ethernet frame header and your data
// The CRC is (seemingly) offloaded to the ENC28J60
typedef
  struct ethii {
    struct uip_eth_hdr  ethhdr;
    myData_t            data[DATA_CNT];
    uint32_t            tagEOL;
  } __attribute__ ((packed)) // Ensure no gaps are inserted between variables
ethii_t;

// Compile-time Fragmentation check
#define MTU  1500
CASSERT(sizeof(ethii_t) <= MTU, RawDemo);

// Pick an unused Frame "type"
// https://www.cisco.com/c/en/us/td/docs/ios/12_2/ibm/vol1/command/reference/fibm_r1/br1fethc.pdf
#define ETHTYPE_XXX  (0xBEEF)

// Tag names
#define MAGIC_BE(s)   ( (uint32_t)((s[0] << 24) | (s[1] << 16) | (s[2] << 8) | s[3]) )
#define BEEF_TAG_1    MAGIC_BE("TEsT")
#define BEEF_TAG_EOL  MAGIC_BE("EOL!")

// A pointer to the UIP Frame Buffer
#define UIPBUF  ((ethii_t*)&uip_buf[0])

//+============================================================================= =======================================
void  serial_setup (void)
{
  // Attempt to talk to the outside world  
  STDOUT.begin(115200);
  
  // Wait for a com port to go live; give up after 10 seconds
  for (unsigned int ms = millis() + (10 * 1000);  !STDOUT && (ms > millis());  delay(300))
    digitalWrite(LED_BUILTIN, (digitalRead(LED_BUILTIN) == LED_ON) ? LED_OFF : LED_ON);  // Blink LED while waiting
  digitalWrite(LED_BUILTIN, (STDOUT) ? LED_ON : LED_OFF);  // Denote on:success / off:timed out
  
  // If we found a monitor - say hello
  if (STDOUT)  FSAYLN("# " APPNAME " " VERS) ;
}

//+============================================================================= =======================================
void  uip_setup (void) 
{
  FSAY("# Initialise Eth Controller");
  Ethernet.begin(myMAC, myIP);
  FSAYLN(" - OK");
  
  FSAY("# Build Packet : ");

  // The Ethernet Frame header
  memset(UIPBUF->ethhdr.dest.addr, 0xff, 6);             // Dst MAC : Broadcast
  memcpy(UIPBUF->ethhdr.src.addr, uip_ethaddr.addr, 6);  // Src MAC : local
  UIPBUF->ethhdr.type = HTONS(ETHTYPE_XXX);              // Type    : ???

  // Your data
  for (int i = 0;  i < DATA_CNT;  i++) {
    UIPBUF->data[i].tag   = htonl(BEEF_TAG_1);
    UIPBUF->data[i].len   = HTONS(SIZEOFM(myData_t, val));
    UIPBUF->data[i].val.i = 0xBC;
    UIPBUF->data[i].val.c = '9';
  }
  UIPBUF->tagEOL = htonl(BEEF_TAG_EOL);

  // Transmission size
  uip_len = sizeof(ethii_t);

  // Tell user
  SAY(uip_len, DEC);
  if (uip_len < 64) {
    FSAY(" -> 64");
  }
  FSAYLN(" bytes");
}

//+============================================================================= =======================================
void  setup (void)
{
  pinMode(LED_BUILTIN, OUTPUT);
  serial_setup();
  uip_setup();
}

//+============================================================================= =======================================
// This floods the network with the raw packet made in uip_setup()
// and reports how many frames & data blocks were sent every second
//
void  loop (void)
{
  unsigned int ms;
  unsigned int cnt;
  
  for (cnt = 0,  ms = millis() + 1000;  millis() < ms;  cnt++) {
    Ethernet.network_send();
  }

  SAY(cnt);  FSAY(" * ");  SAY(DATA_CNT);  FSAY(" = ");  SAYLN(cnt * DATA_CNT);
}

