/*                                                                                 
    This file is part of libNRF24L01p.                                 
    Copyright 2013  Vitor Boschi da Silva <vitorboschi@gmail.com>                            
                                                                                   
    This library is free software; you can redistribute it and/or                  
    modify it under the terms of the GNU Lesser General Public                     
    License as published by the Free Software Foundation; either                   
    version 2.1 of the License, or (at your option) any later version.             
                                                                                   
    This library is distributed in the hope that it will be useful,                
    but WITHOUT ANY WARRANTY; without even the implied warranty of                 
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              
    Lesser General Public License for more details.                                
                                                                                   
    You should have received a copy of the GNU Lesser General Public               
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NRFCONTROLER_H
#define NRFCONTROLER_H

#include "HWAbstraction.h"

#define NRF_MAX_ADDRESS_SIZE 5
#define NRF_MAX_CHANNEL 127


#define NRF_R_REGISTER 0x00
#define NRF_W_REGISTER 0x20
#define NRF_R_RX_PAYLOAD 0x60
#define NRF_W_TX_PAYLOAD 0xA0
#define NRF_FLUSH_TX 0xE1
#define NRF_FLUXH_RX 0xE2
#define NRF_REUSE_TX_PL 0xE3
#define NRF_NOP 0xFF

#define NRF_DUMMY 0x00

#define NRF_REG_CONFIG 0x00
#define NRF_REG_EN_AA 0x01
#define NRF_REG_EN_RXADDR 0x02
#define NRF_REG_SETUP_AW 0x03
#define NRF_REG_SETUP_RETR 0x04
#define NRF_REG_RF_CH 0x05
#define NRF_REG_RF_SETUP 0x06
#define NRF_REG_STATUS 0x07
#define NRF_REG_OBSERVE_TX 0x08
#define NRF_REG_CD 0x09
#define NRF_REG_RX_ADDR_P0 0x0A
#define NRF_REG_RX_ADDR_P1 0x0B
#define NRF_REG_RX_ADDR_P2 0x0C
#define NRF_REG_RX_ADDR_P3 0x0D
#define NRF_REG_RX_ADDR_P4 0x0E
#define NRF_REG_RX_ADDR_P5 0x0F
#define NRF_REG_TX_ADDR 0x10
#define NRF_REG_RX_PW_P0 0x11
#define NRF_REG_RX_PW_P1 0x12
#define NRF_REG_RX_PW_P2 0x13
#define NRF_REG_RX_PW_P3 0x14
#define NRF_REG_RX_PW_P4 0x15
#define NRF_REG_RX_PW_P5 0x16
#define NRF_REG_FIFO_STATUS 0x017


class NRFController {
    public:
    enum NRFDataRate {
        NRF1Mbps,
        NRF2Mbps
    };

    enum NRFMode {
        NRFTxMode,
        NRFRxMode
    };

    NRFController(const char* dev);
    ~NRFController();

    bool setPacketSize(uint8_t numBytes, uint8_t pipe = 0);
    bool setCRC(int size);
    bool setDataRate(NRFDataRate rate);
    bool setRetries(int retries);
    bool setAutoAck(bool autoAck, uint8_t pipe = 0);
    bool setAddressWidth(int width);
    uint8_t addressWidth();
    bool setRxAddress(uint64_t address, uint8_t n, uint8_t pipe = 0);
    bool setChannel(int channel);
    int readData(int size, char* buffer);
    int writeData(int size, const char* buffer);
    bool sendPkg(const char* data);
    bool dataAvailable();
    bool setPowerUp(bool powerUp);
    bool setMode(NRFMode mode);
    private:
    bool readRegister(uint8_t regNumber, uint8_t regBuffer[], int size = 1);
    bool writeRegister(uint8_t regNumber, const uint8_t regValue[], int size = 1);

    HWAbstraction* m_device;
};

#endif
