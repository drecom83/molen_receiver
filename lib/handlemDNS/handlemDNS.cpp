#include "handlemDNS.h"

String firstFreeHostname = "";

char* pcHostDomain = 0;             // negotiated host domain
bool bHostDomainConfirmed = false;  // flags the confirmation of the host domain

void mDNSnotifyAPChange() {
    MDNS.notifyAPChange();
}

void startmDNS() {
  MDNS.setHostProbeResultCallback(hostProbeResult);
  if ((!MDNSResponder::indexDomain(pcHostDomain, 0, "model")) ||
      (!MDNS.begin(pcHostDomain))) {
        Serial.println("Error setting up MDNS Responder!");
        while(1) {
          delay(1000);
        }
  }
  Serial.println("MDNS responder started");
}

bool setStationHostname(const char* p_pcHostname)
{
    if (p_pcHostname) {
        WiFi.hostname(p_pcHostname);
        Serial.printf("SetStationHostname: Station hostname is set to '%s'\n", p_pcHostname);
        firstFreeHostname = p_pcHostname;
        return true;
    }
    return false;
}

String findFirstFreeHostname() {
    MDNS.notifyAPChange();
    return firstFreeHostname;
}

void hostProbeResult(String p_pcDomainName, bool p_bProbeResult)
{
    Serial.printf("MDNSHostProbeResultCallback: Host domain '%s.local' is %s\n", p_pcDomainName.c_str(), (p_bProbeResult ? "free" : "already USED!"));

    if (true == p_bProbeResult) {
        setStationHostname(pcHostDomain);

        if (!bHostDomainConfirmed) {
            bHostDomainConfirmed = true;
        }
    }
    else {
        // Change hostname, use '-' as divider between base name and index
        if (MDNSResponder::indexDomain(pcHostDomain, "-", 0)) {
            MDNS.setHostname(pcHostDomain);
            Serial.printf("MDNSProbeResultCallback: After failing to update hostname. Set hostname '%s'\n", p_pcDomainName.c_str());
        }
        else {
            Serial.println("MDNSProbeResultCallback: Faild to update hostname!");
        }
    }
}