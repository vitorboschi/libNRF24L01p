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

#ifndef HW_ABSTRACTION_H
#define HW_ABSTRACTION_H

#include <string>
#include <stdint.h>

class HWAbstraction {
    public:
    HWAbstraction(const char* spiDevice);
    ~HWAbstraction();

    int openDevice();
    void closeDevice();
    bool setCE();
    bool clearCE();
    bool transact(const uint8_t* tx, uint8_t* rx, int n);

    private:
    bool setupIO();
    int m_fd;
    uint16_t m_delay;
    std::string m_spiDevice;
    void *m_gpioMap;
    volatile unsigned int *m_gpio;
};

#endif
