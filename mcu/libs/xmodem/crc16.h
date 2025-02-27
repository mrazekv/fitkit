/*******************************************************************************
   crc16.h: 16b CRC
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology

   LICENSE TERMS

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:
   1. Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the
      distribution.
   3. All advertising materials mentioning features or use of this software
      or firmware must display the following acknowledgement:

        This product includes software developed by the University of
        Technology, Faculty of Information Technology, Brno and its
        contributors.

   4. Neither the name of the Company nor the names of its contributors
      may be used to endorse or promote products derived from this
      software without specific prior written permission.

   This software or firmware is provided ``as is'', and any express or implied
   warranties, including, but not limited to, the implied warranties of
   merchantability and fitness for a particular purpose are disclaimed.
   In no event shall the company or contributors be liable for any
   direct, indirect, incidental, special, exemplary, or consequential
   damages (including, but not limited to, procurement of substitute
   goods or services; loss of use, data, or profits; or business
   interruption) however caused and on any theory of liability, whether
   in contract, strict liability, or tort (including negligence or
   otherwise) arising in any way out of the use of this software, even
   if advised of the possibility of such damage.

   $Id$

   Popis:
     Definice fce pro vypocet 16b CRC, pouziteho potokolem XModem.
*******************************************************************************/


#ifndef _CRC16_H_
#define _CRC16_H_

/**
  \brief  V�po�et hodnoty CRC16.
  \param  buf - ukazatel na po��tek pole zdrojov�ch dat pro v�po�et
  \param  len - po�et znak�, pou�it�ch pro v�po�et
  \return Hodnota vypo�ten�ho CRC
 **/
unsigned short crc16_ccitt(const unsigned char *buf, int len);

#endif /* _CRC16_H_ */

