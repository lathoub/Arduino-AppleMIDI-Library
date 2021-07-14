#pragma once

class IPAddress
{
public:
    IPAddress(){};
    IPAddress(const IPAddress& from){};
    IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet){};
    IPAddress(uint32_t address) { }
    IPAddress(int address) { }
    IPAddress(const uint8_t *address) {};

    bool operator==(const IPAddress&) const { return true; }
    bool operator!=(const IPAddress&) const { return true; }
};

const IPAddress INADDR_NONE(0, 0, 0, 0);