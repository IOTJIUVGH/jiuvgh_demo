#pragma once

/* open audio device*/
void audio_init(void);

/* start playing */
int audio_dac_start(void);
/* stop playing */
int audio_dac_stop(void);
/* set play channel. default is mono mode
 * channel=1: mono mode.
 * channel=2: stereo mode.
 */
int audio_dac_channel_set(int channel);

/* set audio samplerate.
 * valid samplerate: 11025, 22050, 44100, 12000, 24000, 48000, 8000, 16000 and 32000.
 */
int audio_dac_samplerate_set(int samplerate);

/* set volume. from 0~100*/
int audio_dac_volume_set(int vol);

/* write PCM data to ring buffer */
int audio_dac_write(uint8_t *buf, int len);

void audio_dac_mute(void);
void audio_dac_unmute(void);

/* start recording */
int audio_adc_start(void);

/* stop recording */
int audio_adc_stop(void);

/* set recorder samplerate.
 * valid samplerate: 11025, 22050, 44100, 12000, 24000, 48000, 8000, 16000 and 32000.
 */
int audio_adc_samplerate_set(int samplerate);

/* set recorder channel mode. this function must be called before audio_adc_start
 * channel=1: mono mode.
 * channel=2: stereo mode.
 */
int audio_adc_channel_set(int channel);
/*
  * 
  * @param  gain: digital volume gain
  *          This parameter can be one of the following values:
  *            @arg 0x00: -17.625dB
  *            @arg ...
  *            @arg 0x2f: 0dB
  *            @arg 0x30: 0.375dB
  *            @arg ...
  *            @arg 0x7f: 30dB
  * @param  amic_bst: AMIC boost gain
  *          This parameter can be one of the following values:
  *            @arg 0: 0dB
  *            @arg 1: 20dB
  *            @arg 2: 30dB
  *            @arg 3: 40dB
*/
int audio_adc_gain_set(uint32_t gain, int amic_bst);

/* read recording PCM data to buf. */
int audio_adc_read(uint8_t *buf, int maxlen);

