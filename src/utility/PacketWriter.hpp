/*!
 *  @author		chris-zen
 *	@date		19/12/16
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

BEGIN_APPLEMIDI_NAMESPACE

template<class UdpClass>
class PacketWriter {
private:
  UdpClass& udp;
  size_t totalWritten;
  size_t expected;

public:
  PacketWriter(UdpClass& udp) : udp(udp){
    totalWritten = expected = 0;
  }

  bool allWritten() {
    return totalWritten == expected;
  }

  size_t writePadding(size_t size) {
    size_t written = 0;
    for (int i = 0; i < size; i++) {
      written += this->udp.write((uint8_t)0);
    }
    totalWritten += written;
    expected += size;
    return written;
  }

  size_t write(uint8_t data) {
    size_t written = this->udp.write(data);
    totalWritten += written;
    expected += 1;
    return written;
  }

  size_t write(uint16_t data) {
    size_t written = this->udp.write((data >> 8) & 0xff) +
                      this->udp.write(data & 0xff);
    totalWritten += written;
    expected += 2;
    return written;
  }

  size_t write(uint32_t data) {
    size_t written = this->udp.write((data >> 24) & 0xff) +
                      this->udp.write((data >> 16) & 0xff) +
                      this->udp.write((data >> 8) & 0xff) +
                      this->udp.write(data & 0xff);
    totalWritten += written;
    expected += 4;
    return written;
  }

  size_t write(uint64_t data) {
    size_t written = this->udp.write((data >> 56) & 0xff) +
                      this->udp.write((data >> 48) & 0xff) +
                      this->udp.write((data >> 40) & 0xff) +
                      this->udp.write((data >> 32) & 0xff) +
                      this->udp.write((data >> 24) & 0xff) +
                      this->udp.write((data >> 16) & 0xff) +
                      this->udp.write((data >> 8) & 0xff) +
                      this->udp.write(data & 0xff);
    totalWritten += written;
    expected += 8;
    return written;
  }
};

END_APPLEMIDI_NAMESPACE
