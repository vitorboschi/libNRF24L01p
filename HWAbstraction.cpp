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

#include "HWAbstraction.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

HWAbstraction::HWAbstraction(const char* spiDevice) {
    m_spiDevice = spiDevice;
    m_fd = -1;
}

HWAbstraction::~HWAbstraction() {
    if (m_fd >= 0) {
        closeDevice();
    }
}

/**
* @brief prepares SPI and GPIO access (to control CE pin)
* this method must be executed succefully before calling any other method in
* this class.
*
* @return 
*/
int HWAbstraction::openDevice() {
    uint8_t mode = 0;
    uint8_t bits = 8;
    uint32_t speed = 1000000;
    int ret;

    m_fd = open(m_spiDevice.c_str(), O_RDWR);
    if (m_fd < 0) {
       return -1;
    }

    ret = ioctl(m_fd, SPI_IOC_WR_MODE, &mode);
    ret = ioctl(m_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ret = ioctl(m_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    return 0;
}

/**
* @brief Releases devices used for SPI and GPIO access. No method may be called
* after a call to this method, except openDevice()
*/
void HWAbstraction::closeDevice() {
    close(m_fd);
    m_fd = -1;
}

/**
* @brief Put CE pin in logic state 1
*
* @return true for success, false otherwise
*/
bool HWAbstraction::setCE() {

}


/**
* @brief Put CE pin in logic state 0
*
* @return true for success, false otherwise
*/
bool HWAbstraction::clearCE() {

}

/**
* @brief Send and receive bytes over SPI interface
* The method will block until de end of transaction. tx and rx buffers must
* be allocated prior to calling this method
*
* @param tx array of size n containing data to be transmitted
* @param rx array of size n that will be filled with bytes received
* @param n size of tx and rx buffers
*
* @return true for success, false otherwise
*/
bool HWAbstraction::transact(const uint8_t* tx, uint8_t* rx, int n) {
    int ret;

    struct spi_ioc_transfer tr;
    tr.tx_buf = (unsigned long)tx;
    tr.rx_buf = (unsigned long)rx;
    tr.len = n;
    tr.delay_usecs = m_delay;
    tr.speed_hz = 0;
    tr.bits_per_word = 0;

    if (m_fd < 0) {
        //device not opened
        return false;
    }

    ret = ioctl(m_fd, SPI_IOC_MESSAGE(1), &tr);

    return true;
}

