/*******************************************************************************
   audio_codec.h: FITKit 2.0 codec control functions
   Copyright (C) 2009 Brno University of Technology,
                      Faculty of Information Technology
   Author(s): Karel Slany <slany AT fit.vutbr.cz>

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


*******************************************************************************/

#ifndef _AUDIO_CODEC_H_
#define _AUDIO_CODEC_H_

#if defined MSP_16X
  //#error "Codec not present on this version of FITKit"
#elif defined MSP_261X
#endif

#define ADDRMASK  0x7F
#define DATAMASK  0x1FF
#define DATABITS  9
#define codec_join_addr_data(addr, data) ((((unsigned int)(addr) & ADDRMASK) << DATABITS) | ((unsigned int)(data) & DATAMASK))

/* 16 bits are send via the SPI interface to the device
 *   MSB                                                                        LSB
 * | 15 | 14 | 13 | 12 | 11 | 10 |  9 |  8 |  7 |  6 |  5 |  4 |  3 |  2 |  1 |  0 |
 * |          7bit register address        |            9bit control data          |
 */

/* codec control register SPI addresses (7 bits) */
#define LINE_IN_VOL_L     0x00    /* left line input channel volume control */
#define LINE_IN_VOL_R     0x01    /* right line input channel volume control */
#define HPHONE_OUT_VOL_L  0x02    /* left channel headphone volume control */
#define HPHONE_OUT_VOL_R  0x03    /* right channel headphone volume control */
#define ALOG_PATH_CTRL    0x04    /* analog audio path control */
#define DTAL_PATH_CTRL    0x05    /* digital audio path control */
#define PWR_DWN_CTRL      0x06    /* power down control */
#define DTAL_IFACE_FMAT   0x07    /* digital audio interface format */
#define SRATE_CTRL        0x08    /* sample rate control */
#define DTAL_IFACE_ACT    0x09    /* digital interface activation */
#define RST_REG           0x0F    /* reset register */

/* left line input channel volume control (9 bits)
 * |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * | LRS | LIM |  X  |  X  |             LIV             |
 * default
 *    0     1     0     0     1     0     1     1     1
 */
#define LRS               0x100   /* left/right line simultaneous volume/mute update; 1 - enabled */
  #define LRS_on          LRS
  #define LRS_off         0x000
#define LIM               0x080   /* left line input mute; 1 - muted */
  #define LIM_on          LIM
  #define LIM_off         0x000
//#define LRES              0x060   /* left control reserved bits */
#define LIV               0x01F   /* left line input volume control (1.5dB step); 10111 - default */
  #define LIV_GAIN_Z        0x017 /* 10111 = 0dB */
  #define LIV_GAIN_MAX      LIV   /* 11111 = +12dB */
  #define LIV_GAIN_MIN      0x000 /* 00000 = -34.5dB */

/* right line input channel volume control (9 bits)
 * |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * | RLS | RIM |  X  |  X  |             RIV             |
 * default
 *    0     1     0     0     1     0     1     1     1
 */
#define RLS               0x100   /* right/left line simultaneous volume/mute update; 1 - enabled */
  #define RLS_on          RLS
  #define RLS_off         0x000
#define RIM               0x080   /* right line input mute; 1 - muted */
  #define RIM_on          RIM
  #define RIM_off         0x000
//#define RRES              0x060   /* right control reserved bits */
#define RIV               0x01F   /* right line input volume control (1.5dB step); 10111 - default */
  #define RIV_GAIN_Z      0x017   /* 10111 = 0dB */
  #define RIV_GAIN_MAX    RIV     /* 11111 = +12dB */
  #define RIV_GAIN_MIN    0x000   /* 00000 = -34.5dB */

/* left channel headphone volume control (9 bits)
 * |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * | LRS | LZC |                   LHV                   |
 * default
 *    0     1     1     1     1     1     0     0     1
 */
//#define LRS               0x100   /* left/right headphone channel simultaneous volume/mute update; 1 - enabled */
#define LZC               0x080   /* left-channel zero-cross detect */
  #define LZC_on          LZC
  #define LZC_off         0x000
#define LHV               0x07F   /* left headphone volume control (79 steps); 1111001 - default */
  #define LHV_GAIN_Z      0x079   /* 1111001 = 0dB */
  #define LHV_GAIN_MAX    LHV     /* 1111111 = +6dB */
  #define LHV_GAIN_MIN    0x030   /* 0110000 = -73dB; any below this value doe nothing - still muted */

/* right channel headphone volume control
 * |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * | RLS | RZC |                   RHV                   |
 * default
 *    0     1     1     1     1     1     0     0     1
 */
//#define RLS               0x100   /* right/left headphone channel simultaneous volume/mute update; 1 - enabled */
#define RZC               0x080   /* right-channel zero-cross detect */
  #define RZC_on          RZC
  #define RZC_off         0x000
#define RHV               0x07F   /* right headphone volume control (79 steps); 1111001 - default */
  #define RHV_GAIN_Z      0x079   /* 1111001 = 0dB */
  #define RHV_GAIN_MAX    LHV     /* 1111111 = +6dB */
  #define RHV_GAIN_MIN    0x030   /* 0110000 = -73dB; any below this value doe nothing - still muted */

/* analog audio path control (9 bits)
 * |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * |       STA       | STE | DAC | BYP |INSEL|MICM |MICB |
 * default
 *    0     0     0     0     0     1     0     1     0
 */
#define STA2              0x100
#define STA1              0x080
#define STA0              0x040
#define STE               0x020   /* STE STA2 STA1 STA0 | added sidetone */
  #define ST_minus0dB     0x120   /*  1   1    X    X   |       0dB      */
  #define ST_minus6dB     0x020   /*  1   0    0    0   |      -6dB      */
  #define ST_minus9dB     0x060   /*  1   0    0    1   |      -9dB      */
  #define ST_minus12dB    0x0A0   /*  1   0    1    0   |     -12dB      */
  #define ST_minus18dB    0x0E0   /*  1   0    1    1   |     -18dB      */
  #define ST_disable      0x000   /*  0   X    X    X   |    disabled    */
#define DAC               0x010   /* DAC selected; 1 = DAC selected */
  #define DAC_on          DAC
  #define DAC_off         0x000
#define BYP               0x008   /* bypass, 1 = enabled */
  #define BYP_on          BYP
  #define BYP_off         0x000
#define INSEL             0x004   /* input select for DAC; 0 = line, 1 = microphone */
  #define INSEL_line      0x000
  #define INSEL_mic       INSEL
#define MICM              0x002   /* microphone mute; 1 = muted */
  #define MICM_on         MICM
  #define MICM_off        0x000
#define MICB              0x001   /* microphone boost; 0 = 0dB, 1 = +20dB */
  #define MICB_on         MICB
  #define MICB_off        0x000

/* digital audio path control (9 bits)
 * |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * |  X  |  X  |  X  |  X  |  X  |DACM |   DEEMP   |ADCHP|
 * default
 *    0     0     0     0     0     1     0     0     0
 */
//#define DAPRES            0x1F0   /* reserved bits */
#define DACM              0x008   /* DAC soft mute; 1 = enabled */
  #define DACM_on         DACM
  #define DACM_off        0x000
#define DEEMP             0x006   /* de-emphasis control */
  #define DEEMP_disabled  0x000   /*    00 - disabled    */
  #define DEEMP_32kHz     0x002   /*    01 - 32kHz       */
  #define DEEMP_44k1Hz    0x004   /*    10 - 44.1kHz     */
  #define DEEMP_48kHz     DEEMP   /*    11 - 48kHz       */
#define ADCHP             0x001   /* ADC high-pass filter; 0 - enabled; 1 - disabled */
  #define ADCHP_on        0x000
  #define ADCHP_off       ADCHP

/* power down control (9 bits)
 * |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * |  X  | OFF | CLK | OSC | OUT | DAC | ADC | MIC |LINE |
 * default
 *    0     0     0     0     0     0     1     1     1
 */
//#define PDCRES            0x100   /* reserved bits */
#define CODEC_OFF         0x080   /* device power; 0 = on, 1 = off */
  #define CODEC_ON        0x000
#define CODEC_CLK         0x040   /* clock; 0 = on; 1 = off */
  #define CODEC_CLK_on    0x000
  #define CODEC_CLK_off   CODEC_CLK
#define CODEC_OSC         0x020   /* oscillator; 0 = on, 1 = off */
  #define CODEC_OSC_on    0x000
  #define CODEC_OSC_off   CODEC_OSC
#define CODEC_OUT         0x010   /* outputs; 0 = on, 1 = off */
  #define CODEC_OUT_on    0x000
  #define CODEC_OUT_off   CODEC_OUT
#define CODEC_DAC         0x008   /* DAC; 0 = on, 1 = off */
  #define CODEC_DAC_on    0x000
  #define CODEC_DAC_off   CODEC_DAC
#define CODEC_ADC         0x004   /* ADC; 0 = on; 1 = off */
  #define CODEC_ADC_on    0x000
  #define CODEC_ADC_off   CODEC_ADC
#define CODEC_MIC         0x002   /* microphone input; 0 = off, 1 = on */
  #define CODEC_MIC_on    0x000
  #define CODEC_MIC_off   CODEC_MIC
#define CODEC_LINE        0x001   /* line input; 0 = on, 1 = off */
  #define CODEC_LINE_on   0x000
  #define CODEC_LINE_off  CODEC_LINE

/* digital audio interface format (9 bits)
 * |  8  |  7  |  6  |  5   |  4  |  3  |  2  |  1  |  0  |
 * |  X  |  X  | MS  |LRSWAP| LRP |    IWL    |    FOR    |
 * default
 *    0     0     0     0      0     0     0     0     1
 */
//#define DAIFRES           0x180   /* reserved bits */
#define MS                0x040   /* master/slave mode; 0 = slave; 1 = master */
  #define MS_slave        0x000
  #define MS_master       MS
#define LRSWAP            0x020   /* DAC left/right swap; 1 = enabled */
  #define LRSWAP_on       LRSWAP
  #define LRSWAP_off      0x000
#define LRP               0x010   /* left/right phase */
  #define LRP_on          LRP     /* 0 = right channel on, LRCIN high; in DSP mode - MSB is
                                   * available on 1st BCLK rising edge after LRCIN rising edge */
  #define LRP_off         0x000   /* 1 = right channel on, LRCIN low; in DSP mode - MSB is
                                   * available on 2nd BCLK rising edge after LRCIN rising edge */
#define IWL               0x00C   /* input bit length */
  #define IWL_16bit       0x000   /*    00 - 16bit    */
  #define IWL_20bit       0x004   /*    01 - 20bit    */
  #define IWL_24bit       0x008   /*    10 - 24bit    */
  #define IWL_32bit       IWL     /*    11 - 32bit    */
#define FOR               0x003   /* data format */
  #define FOR_DSP         FOR     /* 11 - DSP format, frame sync followed by two data words */
  #define FOR_I2S         0x002   /* 10 - I2S format, MSB first, left - 1 aligned */
  #define FOR_left        0x001   /* 01 - MSB first, left aligned */
  #define FOR_right       0x000   /* 00 - MSB first, right aligned */

/* sample rate control (9 bits)
 * |  8  |  7   |  6  |  5  |  4  |  3  |  2  |  1  |  0       |
 * |  X  |CLKOUT|CLKIN|          SR           |BOSR |USB/normal|
 * default
 *    0     0      0     1     0     0     0     0     0
 */
//#define SRCRES            0x100   /* reserved bits */
#define CLKOUT            0x080   /* clock output divider; 0 = MCLK, 1 = MCLK/2 */
  #define CLKOUT_full     0x000
  #define CLKOUT_half     CLKOUT
#define CLKIN             0x040   /* clock input divider; 0 = MCLK, 1 = MCLK/2 */
  #define CLKIN_full      0x000
  #define CLKIN_half      CLKIN
#define SR                0x03C   /* sampling rate control - see datasheet */
#define BOSR              0x002   /* base oversampling rate */
                                  /* for SR and BOSR settings see datasheet */

  /* USB mode */
  #define USB_ADC96k_DAC96k     0x01C
  #define USB_ADC88k2_DAC88k2   0x03E
  #define USB_ADC48k_DAC48k     0x000
  #define USB_ADC44k1_DAC44k1   0x022
  #define USB_ADC32k_DAC32k     0x018
  #define USB_ADC8k021_DAC8k021 0x02E
  #define USB_ADC8k_DAC8k       0x00C

  /* normal mode - MCLK = 12.288MHz */
  #define N12_ADC96k_DAC96k     0x01C
  #define N12_ADC48k_DAC48k     0x000
  #define N12_ADC32k_DAC32k     0x018
  #define N12_ADC8k_DAC8k       0x00C

  /* normal mode - MCLK = 11.2896MHz */
  #define N11_ADC88k2_DAC88k2   0x03C
  #define N11_ADC44k1_DAC44k1   0x020
  #define N11_ADC8k021_DAC8k021 0x02C


#define CLKMOD            0x001   /* clock mode select; 0 = normal, 1 = USB */
  #define CLKMOD_normal   0x000
  #define CLKMOD_USB      CLKMOD  /* FITkit has 12MHz clock -> USB mode */

/* digital interface activation (9 bits)
 * |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * |  X  | RES | RES |  X  |  X  |  X  |  X  |  X  | ACT |
 * default
 *    0     0     0     0     0     0     0     0     0
 */
//#define DIARES            0x1FE   /* reserved bits */
#define ACT               0x001   /* activate interface; 0 = inactive, 1 = active */
  #define ACT_on          ACT
  #define ACT_off         0x000

/* reset register (9 bits)
 * |  8  |  7  |  6  |  5  |  4  |  3  |  2  |  1  |  0  |
 * | RES | RES | RES | RES | RES | RES | RES | RES | RES |
 * default
 *    0     0     0     0     0     0     0     0     0
 */
#define RES               0x1FF   /* write 000000000 to this register triggers reset */

/*******************************************************************************
 * function sends codec configuration data via SPI
 ******************************************************************************/
void codec_SPI_write(unsigned int addr, unsigned int data);

#endif /* _AUDIO_CODEC_H_ */
