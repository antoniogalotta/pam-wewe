#ifndef WEWE_NET_H
#define WEWE_NET_H

// Returns the MAC address of the default gateway as a string.
// The caller is responsible for freeing the returned string.
char* get_default_gateway_mac();

#endif // WEWE_NET_H
