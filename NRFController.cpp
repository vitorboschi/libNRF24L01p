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

#include "NRFController.h"
#include <iostream>

bool NRFController::readRegister(uint8_t regNumber, uint8_t regBuffer[], int size) {
    uint8_t tx[size+1];
    uint8_t rx[size+1];
    
    for (int i=0;i<size+1;i++) {
        tx[i] = 0;
        rx[i] = 0;
    }
    tx[0] = NRF_R_REGISTER | regNumber;

    if (m_device->transact(tx, rx, size+1)) {
        for (int i=0;i<size;i++) {
            regBuffer[i] = rx[i+1];
        }
        return true;
    }
    return false;
}

bool NRFController::writeRegister(uint8_t regNumber, const uint8_t regValue[], int size) {
    uint8_t tx[size+1];
    uint8_t rx[size+1];

    for (int i=0;i<size+1;i++) {
        tx[i] = 0;
        rx[i] = 0;
    }
    tx[0] = NRF_W_REGISTER | regNumber;

    for (int i=0;i<size;i++) {
        tx[i+1] = regValue[i];
    }
 
    if (m_device->transact(tx, rx, size+1)) {
       return true;
    }

    return false;
}

NRFController::NRFController(const char* dev) {
    m_device = new HWAbstraction(dev);
    if (m_device->openDevice() != 0) {
        std::cout << "Can't open device" << std::endl;
    }
}

NRFController::~NRFController() {
    if (m_device) {
        m_device->closeDevice();
        delete(m_device);
    }
}

bool NRFController::setPacketSize(uint8_t packetSize, uint8_t pipe) {
    uint8_t regRxPw;

    //the chip supports up to 6 pipes
    if (pipe > 5) {
        return false;
    }
    readRegister(NRF_REG_RX_PW_P0 + pipe, &regRxPw, 1);
    
    packetSize &= 0x1F; //use only 5 LSb for address
    regRxPw |= packetSize;

    writeRegister(NRF_REG_RX_PW_P0 + pipe, &packetSize, 1);
    return true;
}

bool NRFController::setSetCRC(int crcBytes) {
    uint8_t regConfig;

    readRegister(NRF_REG_CONFIG, &regConfig);
    switch (crcBytes) {
    case 0:
        regConfig = regConfig & (~0x08); //disable crc
        break;
    case 1:
        regConfig |= 0x08; //enable crc
        regConfig = regConfig & (~0x04); //crc size = 1 byte
        break;
    case 2:
        regConfig |= 0x08; //enable crc
        regConfig |= 0x04; //crc size = 2 bytes
        break;

    default:
        return false;
    }

    writeRegister(NRF_REG_CONFIG, &regConfig);
    return true;
}

bool NRFController::setDataRate(NRFDataRate rate) {
    uint8_t regRfSetup;

    readRegister(NRF_REG_RF_SETUP, &regRfSetup);
    switch (rate) {
        case NRF1Mbps:
            regRfSetup = regRfSetup & (~0x08);
            break;
        case NRF2Mbps:
            regRfSetup |= 0x08;
            break;
    }

    writeRegister(NRF_REG_RF_SETUP, &regRfSetup);
    return true;
}

bool setRetries(int retries) {
    return false;
}

bool setAutoAck(bool autoAck) {
    return false;
}

bool NRFController::setAddressWidth(int width) {
    uint8_t regSetupAW;

    readRegister(NRF_REG_SETUP_AW, &regSetupAW);
    regSetupAW = regSetupAW & (~0x03);

    switch (width) {
        case 3:
            regSetupAW |= 0x01;
            break;
        case 4:
            regSetupAW |= 0x02;
            break;
        case 5:
            regSetupAW |= 0x03;
            break;
        default:
            return false;
    }

    writeRegister(NRF_REG_SETUP_AW, &regSetupAW);
    return true;
}

uint8_t NRFController::addressWidth() {
    uint8_t regSetupAW;

    readRegister(NRF_REG_SETUP_AW, &regSetupAW);
    switch (regSetupAW & 0x03) {
        case 1:
            return 3;
            break;
        case 2:
            return 4;
            break;
        case 3:
            return 5;
            break;
        default:
            return 0;
    }
}

bool NRFController::setRxAddress(const uint8_t* address, uint8_t n, uint8_t pipe) {
    if (!setAddressWidth(n)) {
        return false;
    }
    
    writeRegister(NRF_REG_RX_ADDR_P0 + pipe, address, n);
    return true;
}

bool NRFController::setChannel(int channel) {
    if (channel < 0 || channel > NRF_MAX_CHANNEL) {
        return false;
    }

    return true;
}

int NRFController::readData(int size, char* buffer) {
    return 0;
}

int NRFController::writeData(int size, const char* buffer) {
    return 0;
}

bool NRFController::sendPkg(const char* data) {
    return false;
}

int NRFController::bytesAvailable() {
    return 0;
}

