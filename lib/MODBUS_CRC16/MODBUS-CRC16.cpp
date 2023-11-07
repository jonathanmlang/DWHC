/*
 * This file is part of the "bluetoothheater" distribution 
 * (https://gitlab.com/mrjones.id.au/bluetoothheater) 
 *
 * Copyright (C) 2018  Ray Jones <ray@mrjones.id.au>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * 
 */

#include "MODBUS-CRC16.h"

CModBusCRC16::CModBusCRC16()
{
  _CRC = 0xffff;
}

void
CModBusCRC16::reset()
{
  _CRC = 0xffff;
}

uint16_t 
CModBusCRC16::process(int len, const uint8_t* pData)
{
  // calculate a CRC-16/MODBUS checksum using the bytes of the data array
  // carries on from prior developed CRC value, or 0xffff if first pass

  while (len--)
  {
    uint8_t nTemp = *pData++ ^ _CRC;
    _CRC >>= 8;
    //_CRC ^= _CRCTable[nTemp];
    _CRC ^= pgm_read_word_near(_CRCTable + nTemp); //Improvement added over ray jones version, saves ram
  }

  return _CRC;
}

uint16_t 
CModBusCRC16::get() const
{ 
  return _CRC; 
}