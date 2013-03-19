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
#include <sys/mman.h>

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)
// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(m_gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(m_gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(m_gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(m_gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(m_gpio+10) // clears bits which are 1 ignores bits which are 0

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
    
    if (!setupIO()) {
        return -2;
    }

    
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
    GPIO_SET = 1 << 25;
    return true;
}


/**
* @brief Put CE pin in logic state 0
*
* @return true for success, false otherwise
*/
bool HWAbstraction::clearCE() {
    GPIO_CLR = 1 << 25;
    return true;
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

bool HWAbstraction::setupIO() {
    int memFd;
    /* open /dev/mem */
    if ((memFd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        return false;
    }

    /* mmap GPIO */
    m_gpioMap = mmap(
            NULL,             //Any adddress in our space will do
            BLOCK_SIZE,       //Map length
            PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
            MAP_SHARED,       //Shared with other processes
            memFd,           //File to map
            GPIO_BASE         //Offset to GPIO peripheral
            );

    close(memFd); //No need to keep mem_fd open after mmap

    if (m_gpioMap == MAP_FAILED) {
        printf("mmap error %p\n", m_gpioMap);//errno also set!
        return false;
    }

    // Always use volatile pointer!
    m_gpio = (volatile unsigned *)m_gpioMap;
    INP_GPIO(25);
    OUT_GPIO(25);

    return true;
} // setup_io

