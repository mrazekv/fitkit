/*******************************************************************************
   main.c: FITKit 2.0 codec control
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

#include <fitkitlib.h>
#include <codec/audio_codec.h>

/*******************************************************************************
 * mute headphone
 ******************************************************************************/
void hphone_mute(void) {
  term_send_str_crlf("codec setup: headphone left and right");
  codec_SPI_write(HPHONE_OUT_VOL_L, LZC_on | LHV_GAIN_MIN);
  codec_SPI_write(HPHONE_OUT_VOL_R, RZC_on | RHV_GAIN_MIN);
}

/*******************************************************************************
 * restore headphone settings
 ******************************************************************************/
void hphone_restore(void) {
  term_send_str_crlf("codec setup: headphone left and right");
  codec_SPI_write(HPHONE_OUT_VOL_L, LZC_on | LHV_GAIN_Z);
  codec_SPI_write(HPHONE_OUT_VOL_R, RZC_on | RHV_GAIN_Z);
}

/*******************************************************************************
 * digital uadio settings
 ******************************************************************************/
void codec_dsp_mode(void) {
  term_send_str_crlf("codec setup: reset");
  codec_SPI_write(RST_REG, 0);
  term_send_str_crlf("codec setup: analog path control");
  codec_SPI_write(ALOG_PATH_CTRL, ST_disable | DAC_on | BYP_off | INSEL_line | MICM_on | MICB_off);
  term_send_str_crlf("codec setup: digital interface format");
  // LRP_on must be set, MSB must be available after LRCIN/CLRCOUT falling edge
  codec_SPI_write(DTAL_IFACE_FMAT, MS_master |  LRSWAP_off | LRP_on | IWL_16bit | FOR_DSP);
  term_send_str_crlf("codec setup: sample rate format");
  codec_SPI_write(SRATE_CTRL, CLKOUT_full | CLKIN_full | USB_ADC48k_DAC48k | CLKMOD_USB);
  term_send_str_crlf("codec setup: digital interface activation");
  codec_SPI_write(DTAL_IFACE_ACT, ACT_on);
  term_send_str_crlf("codec setup: line-in left and right");
  codec_SPI_write(LINE_IN_VOL_L, LRS_off | LIM_off | (LIV_GAIN_Z));
  codec_SPI_write(LINE_IN_VOL_R, RLS_off | LIM_off | (LIV_GAIN_Z));
  term_send_str_crlf("codec setup: headphone left and right");
  codec_SPI_write(HPHONE_OUT_VOL_L, LZC_on | (LHV_GAIN_Z - 0));
  codec_SPI_write(HPHONE_OUT_VOL_R, RZC_on | (RHV_GAIN_Z - 0));
  term_send_str_crlf("codec setup: digital path control");
  codec_SPI_write(DTAL_PATH_CTRL, DACM_off | DEEMP_disabled | ADCHP_on);
  term_send_str_crlf("codec setup: power control");
  codec_SPI_write(PWR_DWN_CTRL, CODEC_ON | CODEC_CLK_on | CODEC_OSC_on |
                                CODEC_OUT_on | CODEC_DAC_on | CODEC_ADC_on | CODEC_MIC_off | CODEC_LINE_on);
}

/*******************************************************************************
 * analog bypass settings
 ******************************************************************************/
void codec_internal_bypass(void) {
  term_send_str_crlf("codec setup: reset");
  codec_SPI_write(RST_REG, 0);
  term_send_str_crlf("codec setup: analog path control");
  codec_SPI_write(ALOG_PATH_CTRL, ST_minus0dB | DAC_off | BYP_on | INSEL_line | MICM_on | MICB_off);
  term_send_str_crlf("codec setup: power control");
  codec_SPI_write(PWR_DWN_CTRL, CODEC_ON | CODEC_CLK_on | CODEC_OSC_on |
                                CODEC_OUT_on | CODEC_DAC_off | CODEC_ADC_off | CODEC_MIC_off | CODEC_LINE_on);
}

/*******************************************************************************
 * Vypis uzivatelske napovedy (funkce se vola pri vykonavani prikazu "help")
 * systemoveho helpu
*******************************************************************************/
void print_user_help(void)
{
  term_send_str_crlf(" MUTE       ... mute headphone output");
  term_send_str_crlf(" RESTORE    ... restore headphone volume");
  term_send_str_crlf(" DSP_ON     ... setup digital interface to DSP mode etc.");
  term_send_str_crlf(" BYPASS_DSP ... disable digital interface, enable bypass");
}


/*******************************************************************************
 * Dekodovani a vykonani uzivatelskych prikazu
*******************************************************************************/
unsigned char decode_user_cmd(char *cmd_ucase, char *cmd)
{
  if (strcmp4(cmd_ucase, "MUTE"))
    hphone_mute();
  else if (strcmp7(cmd_ucase, "RESTORE"))
    hphone_restore();
  else if (strcmp6(cmd_ucase, "DSP_ON"))
    codec_dsp_mode();
  else if (strcmp10(cmd_ucase, "BYPASS_DSP"))
    codec_internal_bypass();
  else
    return CMD_UNKNOWN;

  return USER_COMMAND;
}

/*******************************************************************************
 * Inicializace periferii/komponent po naprogramovani FPGA
*******************************************************************************/
void fpga_initialized()
{
  codec_dsp_mode();
}

/*******************************************************************************
 * Hlavni funkce
*******************************************************************************/
int main(void)
{
  initialize_hardware();
  WDG_stop();                               // zastav watchdog


  set_led_d5(1);

  while (1) {
    terminal_idle();                       // obsluha terminalu
  }

  return 0;
}
