/*

===============================================================================                 
                              PINOUTS & WIRING
===============================================================================                 

Looking at the M2 from the BACK, with the SDCard/USB port on the TOP,
so you are looking IN to the FEMALE connector, from the USER-SIDE

                                            [83]
                                 SPI0_CK   SPI_CS2    +3v3  Gnd
                                       |     |           |  |
                      ,----------------^-----^-----------^--+.
    Macchina M2       | 1| 3| 5| 7| 9|11|13|15|17|19|21|23|25|
        [J5]          |--+--+--+--+--+--+--+--+--+--+--+--+--+
 "26 pin Connector"   | 2| 4| 6| 8|10|12|14|16|18|20|22|24|26|
                      `----------------^-----v-----v---------'
                                       |     |     |
                               SPI0_MISO SPI0_MOSI +5v0
                               

No, "SPI_CS2" is NOT a typing error ...Well, at least, not MY typing error!
And, YES, it IS the SPI CS pin on J5/15 ...Arduino Pin Number 83

===============================================================================                

The pins on my ENC28J60's are labelled (silkscreened)
Viewed from the TOP (with the MALE header pins pointing TOWARD you),
and with the components {driver, PHY, etc.} on the ->RIGHT->

                  ,-----.
            CLK   |  |  |   NT
                  |--+--+   
            WOL   |  |  >-- SO
                  |--+--+   
 ENC28J60    SI -->  |  <-- SCK
                  |--+--+   
             CS -->  |  |   RST
                  |--+--+
            VCC -->  |  +-- GND
                  `-----'

# These things are made out of the purest Chinesium
  ...Your pinout may well be different to mine - check it!
   
# VCC is allegedly +3V3
  ...And, indded, some of them run quite nicely on +3V3
  ...BUT I have had devices that required 5V0 to
     a) Send packets to the network, and
     b) Get very hot

# NT is probably some kind of Interrupt Line

# WOL will be a Wake-On-LAN feature
  ...So the ENC28J60 can poke the Arduino if it sees a Wakeup packet
  
# RST is the Reset pin
  Now... The M2 does NOT expose Rst on J5
  So... I'm going to ignore it for now and hope it's not a problem
        ...Which I think it might be :(
        ...Maybe we can use the Rst pin on the XBEE connector?

# CLK is ...errr, no idea - but it's sure not the SPI Clock

===============================================================================                

You need to attach your ENC28J60 Ethernet board to the M2 via SPI Bus.
The SPI bus is availabe on the "26-pin Connector" [J5]
You will also need +3V3 and Gnd to power the ENC28J60
  J5/11 = SAM/PA27 : SPI0_CK     ---->   SCK
  J5/12 = SAM/PA25 : SPI0_MISO   <----   SO
  J5/15 = SAM/PB21 : SPI_CS2     ---->   CS
  J5/16 = SAM/PA26 : SPI0_MOSI   ---->   SI
  J5/20 =          : +5V0        ---->   n/c (see above notes)
  J5/23 =          : +3V3        ---->   VCC
  J5/25 =          : Gnd         -----   GND

===============================================================================                

//------------------------------------------------------------------------------ ---------------------------------------
#define HOSTNAME_MAX  (255)
char           hostname[HOSTNAME_MAX];
IPAddress      zip    = {0, 0, 0, 0};          // 0.0.0.0 - used to reset the interface
IPAddress      local  = {172, 26, 26, 250};    // Default address
unsigned char  mac[6] = {0xAA, 0xAA, 0xAA, 0xAA, 0xBC, 0x01};

//+============================================================================= =======================================
void  eth_setIP (IPAddress ip) 
{
  int  len = sprintf(hostname, "%.*s_%02X%02X", HOSTNAME_MAX - 6, "MacChinaM2", mac[4], mac[5]);
  Ethernet.setHostname(hostname, len);
  
  Ethernet.configure(zip, zip, zip, zip); // ip, dns, gw, subnet

  led_off(LED_RED);
  led_off(LED_GRN);
  if (memcmp(&ip, &zip, sizeof(IPAddress)) == 0) {
    FSAY("# DHCP...");
    led_on(LED_BLU);
    if (!Ethernet.begin(mac)) {
      led_off(LED_BLU);
      led_on(LED_RED);
      FSAYLN(" FAIL");
      return;  // RETURN
      
    } else {
      led_off(LED_BLU);
      led_on(LED_GRN);
      FSAYLN(" OK");
    }
    
  } else {
    FSAYLN("# Set IP...");
    led_on(LED_RED2);
    Ethernet.begin(mac, ip);
    led_off(LED_RED2);
    
    led_on(LED_RED);
    led_on(LED_GRN);
  }
  
  FSAY("  Local IP    : ");  SAYLN(Ethernet.localIP());
  FSAY("  Gateway     : ");  SAYLN(Ethernet.gatewayIP());
  FSAY("  DNS Server  : ");  SAYLN(Ethernet.dnsServerIP());
  FSAY("  Netmask     : ");  SAYLN(Ethernet.subnetMask());
}

*/

