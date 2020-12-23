#ifndef HANDLEMDNS_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define HANDLEMDNS_H
#include <ESP8266mDNS.h>

bool setStationHostname(const char* p_pcHostname);
void hostProbeResult(String p_pcDomainName, bool p_bProbeResult);

void startmDNS();
void mDNSnotifyAPChange();

String findFirstFreeHostname();
void hostProbeKnownHostnames(String p_pcDomainName, bool p_bProbeResult);

#endif