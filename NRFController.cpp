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

/**
* @brief Read a register from the NRF24L01+ module
*
* @param regNumber which register to read. Should be one of NRF_REG_* defines
* @param regBuffer[] buffer to hold register data. Must be able to hold the number of bytes specified by last parameter
* @param size size of register, in bytes
*
* @return true for success, false otherwise
*/
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

/**
* @brief Write a register into the NRF24L01+ module
*
* @param regNumber which register to write. Should be one of NRF_REG_* defines
* @param regBuffer[] buffer holding register data to be written. Must be able to hold the number of bytes specified by last parameter
* @param size size of register, in bytes
*
* @return true for success, false otherwise
*/
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

/**
* @brief instantiate a controller for the NRF24L01+ module
*
* @param dev Linux SPI device to use for communication
* @todo move device opening to another method
*/
NRFController::NRFController(const char* dev) {
    m_device = new HWAbstraction(dev);
    if (m_device->openDevice() != 0) {
        std::cout << "Can't open device" << std::endl;
    }
}

/**
* @brief releases all resources used by the controller
*/
NRFController::~NRFController() {
    if (m_device) {
        m_device->closeDevice();
        delete(m_device);
    }
}

/**
* @brief Configure payload size to be used in transmissions
*
* @param packetSize payload size, in bytes. Valid sizes are from 0 (disable pipe) to 32
* @param pipe which pipe to configure the payload size
*
* @return 
*/
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

/**
* @brief Configure CRC mode to be used
* unless you need to squeeze the maximum possible data rate, use 2 bytes for
* CRC. This will help avoid spurious and invalid packages
*
* @param crcBytes how many bytes to use in CRC. Valid values are 0, 1 and 2
*
* @return true for success, false otherwise
*/
bool NRFController::setCRC(int crcBytes) {
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

/**
* @brief Configure data rate to be used
* the slower the data rate, the longer range you achieve
*
* @param rate which data rate to use
*
* @return true for success, false otherwise
*/
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

/**
* @brief Configure how many times the module tries to send a package not ACK'ed
*
* @param retries how many retries. Valid values are from 0 to 15
*
* @return true for success, false otherwise
*/
bool NRFController::setRetries(int retries) {
    uint8_t regSetupRetR;
    
    //validate input
    if (retries < 0 || retries > 15) {
        return false;
    }

    //get current register value
    readRegister(NRF_REG_SETUP_RETR, &regSetupRetR);

    //clear ARC field (bits 3:0)
    regSetupRetR = regSetupRetR & (~0x0F);
    //set retries count
    regSetupRetR |= retries;

    //update register
    writeRegister(NRF_REG_SETUP_RETR, &regSetupRetR);
    return true;
}

/**
* @brief Enable or disable auto ack feature
*
* @param autoAck true to enable, false to disable
*
* @return true for success, false otherwise
*/
bool setAutoAck(bool autoAck) {
    return false;
}

/**
* @brief Configure the number of bytes to be used in the address field
*
* @param width how many bytes to use. Valid values currently are 3, 4 ou 5
*
* @return true for success, false otherwise
*/
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

/**
* @brief retrieve the number of bytes used for device address
*
* @return the number of bytes used for address
*/
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

/**
* @brief Set the address used for a RX pipe
* Address width must be set to n bytes before setting the actual address
*
* @param address array of bytes containing the address
* @param n size of the array address
* @param pipe which pipe to set the address. Pipes 0 and 1 will use the complete address, but other pipes will only set LSB. Other bytes will be mirrored from pipe 1 address
*
* @return true for success, false otherwise
*/
bool NRFController::setRxAddress(const uint8_t* address, uint8_t n, uint8_t pipe) {
    if (!setAddressWidth(n)) {
        return false;
    }
    
    writeRegister(NRF_REG_RX_ADDR_P0 + pipe, address, n);
    return true;
}

/**
* @brief Configure the RF channel to be used by NRF24L01+ module
*
* @param channel channel number, from 0 to 127. Check which address you're allowed to use in your country
*
* @return true for successm false otherwise
*/
bool NRFController::setChannel(int channel) {
    if (channel < 0 || channel > NRF_MAX_CHANNEL) {
        return false;
    }

    return true;
}

/**
* @brief read data from internal buffer.
* Always check and retrieve data from RF module, if it's available. Note: this
* method will not block in case data is not available. It'll just return 0
*
* @param size how many bytes to read
* @param buffer pre-allocated buffer where data will be written
*
* @return how many bytes were effectively read
*/
int NRFController::readData(int size, char* buffer) {
    return 0;
}

/**
* @brief write data to internal buffer.
* data will be split in packages and sent.
*
* @param size how many bytes to be written
* @param buffer buffer containing data to be written
*
* @return how many bytes were effectively written
*/
int NRFController::writeData(int size, const char* buffer) {
    return 0;
}

/**
* @brief low level method to dispatch a single package
*
* @param data buffer containing data. It must have the size specified in setPackageSize()
*
* @return true for success, false otherwise
*/
bool NRFController::sendPkg(const char* data) {
    return false;
}

/**
* @brief retrieve the number of bytes available for reading
* this method takes in account the size of the internal buffer, plus the module's buffer
*
* @return bytes available for reading
*/
int NRFController::bytesAvailable() {
    return 0;
}

