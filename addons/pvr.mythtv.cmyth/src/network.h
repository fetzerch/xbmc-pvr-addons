
#ifndef NETWORK_H
#define	NETWORK_H

class Network {
public:
  // Return true if the magic packet was send
   static bool WakeOnLan(const char *mac);
};

#endif	/* NETWORK_H */

