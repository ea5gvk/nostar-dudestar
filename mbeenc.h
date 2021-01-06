/*
	Copyright (C) 2019-2021 Doug McLain

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef INCLUDED_AMBE_ENCODER_H
#define INCLUDED_AMBE_ENCODER_H

#include <stdint.h>
#include "ambe.h"
#include "imbe_vocoder/imbe_vocoder.h"

class MBEEncoder {
public:
	void encode(int16_t samples[], uint8_t codeword[]);
	MBEEncoder(void);
	void set_49bit_mode(void);
	void set_dmr_mode(void);
	void set_88bit_mode(void);
	void set_dstar_mode(void);
	void set_gain_adjust(const float gain_adjust) {d_gain_adjust = gain_adjust;}//vocoder.set_gain_adjust(gain_adjust);}
	void set_alt_dstar_interleave(const bool v) { d_alt_dstar_interleave = v; }
private:
	imbe_vocoder vocoder;
	mbe_parms cur_mp;
	mbe_parms prev_mp;
	bool d_49bit_mode;
	bool d_dmr_mode;
	bool d_88bit_mode;
	bool d_dstar_mode;
	float d_gain_adjust;
	bool d_alt_dstar_interleave;
	void encode_dstar(uint8_t result[72], const int b[9], bool alt_dstar_interleave);
	void encode_dmr(const unsigned char* in, unsigned char* out);
	void encode_vcw(uint8_t vf[], const int* b);
	void interleave_vcw(uint8_t _vf[], int _c0, int _c1, int _c2, int _c3);
};

#endif /* INCLUDED_AMBE_ENCODER_H */
