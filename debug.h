#define DEBUG
#ifdef DEBUG
#define DPRINTLN(x) Serial.println(x)
#define DPRINTBINLN(x) Serial.println(x, BIN)
#define DPRINTLNF(x) Serial.println(F(x))
#define DPRINT(x) Serial.print(x)
#define DPRINTBIN(x) Serial.print(x, BIN)
#define DPRINTF(x) Serial.print(F(x))
#else
#define DPRINTLN(x)
#define DPRINTBINLN(x)
#define DPRINTLNF(x)
#define DPRINT(x)
#define DPRINTBIN(x)
#define DPRINTF(x)
#endif
