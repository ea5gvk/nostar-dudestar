/*
	Copyright (C) 2019 Doug McLain

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

#include "ysfenc.h"

#include "YSFConvolution.h"
#include "CRCenc.h"
#include "Golay24128.h"
#include "vocoder_tables.h"

#include <iostream>
#include <cstring>

#define DEBUG

const int vd2DVSIInterleave[49] = {
		0, 3, 6,  9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 41, 43, 45, 47,
		1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 42, 44, 46, 48,
		2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38
};

const int vd2DVSIDEInterleave[49] = {
		0, 18, 36,  1, 19, 37, 2, 20, 38, 3, 21, 39, 4, 22, 40, 5, 23, 41,
		6, 24, 42, 7, 25, 43, 8, 26, 44, 9, 27, 45, 10, 28, 46, 11, 29, 47,
		12, 30, 48, 13, 31, 14, 32, 15, 33, 16, 34, 17, 35
};

const char ysf_radioid[] = {'H', '5', '0', '0', '0'};

YSFEncoder::YSFEncoder()
{
	ysf_cnt = 0;
}

YSFEncoder::~YSFEncoder()
{
}

void YSFEncoder::set_callsign(const char *cs)
{
	::memcpy(gateway, "          ", 10);
	::memcpy(callsign, "          ", 10);
	::memcpy(callsign_full, "          ", 10);
	::memcpy(gateway, cs, ::strlen(cs));
	::memcpy(callsign, cs, ::strlen(cs));
	::memcpy(callsign_full, cs, ::strlen(cs));
}

unsigned char * YSFEncoder::get_frame(unsigned char *ambe)
{
	if(!ysf_cnt){
		encode_header();
	}
	else{
		m_ambe = ambe;
		encode_dv2();
	}
	++ysf_cnt;
	return m_ysfFrame;
}

unsigned char * YSFEncoder::get_eot()
{
	encode_header(1);
	ysf_cnt = 0;
	return m_ysfFrame;
}

void YSFEncoder::encode_header(bool eot)
{
	unsigned char *p_frame = m_ysfFrame;
	if(m_fcs){
		::memset(p_frame + 120U, 0, 10U);
		::memcpy(p_frame + 121U, m_fcsname.c_str(), 8);
	}
	else{
		::memcpy(p_frame + 0U, "YSFD", 4U);
		::memcpy(p_frame + 4U, callsign, YSF_CALLSIGN_LENGTH);
		::memcpy(p_frame + 14U, callsign, YSF_CALLSIGN_LENGTH);
		::memcpy(p_frame + 24U, "ALL       ", YSF_CALLSIGN_LENGTH);

		if(eot){
			p_frame[34U] = ((ysf_cnt & 0x7f) << 1U) | 1U;
		}
		else{
			p_frame[34U] = 0U;
		}
		p_frame = m_ysfFrame + 35U;
	}
	::memcpy(p_frame, YSF_SYNC_BYTES, 5);
	
	fich.setFI(eot ? YSF_FI_TERMINATOR : YSF_FI_HEADER);
	fich.setCS(2U);
	fich.setCM(0U);
	fich.setBN(0U);
	fich.setBT(0U);
	fich.setFN(0U);
	fich.setFT(6U);
	fich.setDev(0U);
	fich.setMR(0U);
	fich.setVoIP(false);
	fich.setDT(YSF_DT_VD_MODE2);
	fich.setSQL(false);
	fich.setSQ(0U);
	fich.encode(p_frame);

	unsigned char csd1[20U], csd2[20U];
	memset(csd1, '*', YSF_CALLSIGN_LENGTH);
	//memset(csd1, '*', YSF_CALLSIGN_LENGTH/2);
	//memcpy(csd1 + YSF_CALLSIGN_LENGTH/2, ysf_radioid, YSF_CALLSIGN_LENGTH/2);
	memcpy(csd1 + YSF_CALLSIGN_LENGTH, callsign, YSF_CALLSIGN_LENGTH);
	memcpy(csd2, callsign, YSF_CALLSIGN_LENGTH);
	memcpy(csd2 + YSF_CALLSIGN_LENGTH, callsign, YSF_CALLSIGN_LENGTH);
	//memset(csd2, ' ', YSF_CALLSIGN_LENGTH + YSF_CALLSIGN_LENGTH);

	writeDataFRModeData1(csd1, p_frame);
	writeDataFRModeData2(csd2, p_frame);
}

void YSFEncoder::encode_dv2()
{
	unsigned char *p_frame = m_ysfFrame;
	if(m_fcs){
		::memset(p_frame + 120U, 0, 10U);
		::memcpy(p_frame + 121U, m_fcsname.c_str(), 8);
	}
	else{
		::memcpy(m_ysfFrame + 0U, "YSFD", 4U);
		::memcpy(m_ysfFrame + 4U, callsign, YSF_CALLSIGN_LENGTH);
		::memcpy(m_ysfFrame + 14U, callsign, YSF_CALLSIGN_LENGTH);
		::memcpy(m_ysfFrame + 24U, "ALL       ", YSF_CALLSIGN_LENGTH);
		m_ysfFrame[34U] = (ysf_cnt & 0x7f) << 1;
		p_frame = m_ysfFrame + 35U;
	}
	::memcpy(p_frame, YSF_SYNC_BYTES, 5);
	unsigned int fn = (ysf_cnt - 1U) % 7U;

	fich.setFI(YSF_FI_COMMUNICATIONS);
	fich.setCS(2U);
	fich.setCM(0U);
	fich.setBN(0U);
	fich.setBT(0U);
	fich.setFN(fn);
	fich.setFT(6U);
	fich.setDev(0U);
	fich.setMR(0U);
	fich.setVoIP(false);
	fich.setDT(YSF_DT_VD_MODE2);
	fich.setSQL(false);
	fich.setSQ(0U);
	fich.encode(p_frame);

	const uint8_t ft70d1[10] = {0x01, 0x22, 0x61, 0x5f, 0x2b, 0x03, 0x11, 0x00, 0x00, 0x00};
	//const uint8_t dt1_temp[] = {0x31, 0x22, 0x62, 0x5F, 0x29, 0x00, 0x00, 0x00, 0x00, 0x00};
	const uint8_t dt2_temp[10] = {0x00, 0x00, 0x00, 0x00, 0x6C, 0x20, 0x1C, 0x20, 0x03, 0x08};

	switch (fn) {
	case 0:
		//memset(dch, '*', YSF_CALLSIGN_LENGTH/2);
		//memcpy(dch + YSF_CALLSIGN_LENGTH/2, ysf_radioid, YSF_CALLSIGN_LENGTH/2);
		//writeVDMode2Data(m_ysfFrame + 35U, dch);	//Dest
		writeVDMode2Data(p_frame, (const unsigned char*)"**********");
		break;
	case 1:
		writeVDMode2Data(p_frame, (const unsigned char*)callsign_full);		//Src
		break;
	case 2:
		writeVDMode2Data(p_frame, (const unsigned char*)callsign);				//D/L
		break;
	case 3:
		writeVDMode2Data(p_frame, (const unsigned char*)callsign);				//U/L
		break;
	case 4:
		writeVDMode2Data(p_frame, (const unsigned char*)"          ");			//Rem1/2
		break;
	case 5:
		writeVDMode2Data(p_frame, (const unsigned char*)"          ");			//Rem3/4
		//memset(dch, ' ', YSF_CALLSIGN_LENGTH/2);
		//memcpy(dch + YSF_CALLSIGN_LENGTH/2, ysf_radioid, YSF_CALLSIGN_LENGTH/2);
		//writeVDMode2Data(frame, dch);	// Rem3/4
		break;
	case 6:
		writeVDMode2Data(p_frame, ft70d1);
		break;
	case 7:
		writeVDMode2Data(p_frame, dt2_temp);
		break;
	default:
		writeVDMode2Data(p_frame, (const unsigned char*)"          ");
	}
}

void YSFEncoder::writeDataFRModeData1(const unsigned char* dt, unsigned char* data)
{
	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	unsigned char output[25U];
	for (unsigned int i = 0U; i < 20U; i++)
		output[i] = dt[i] ^ WHITENING_DATA[i];

	CCRC::addCCITT162(output, 22U);
	output[22U] = 0x00U;

	unsigned char convolved[45U];

	CYSFConvolution conv;
	conv.encode(output, convolved, 180U);

	unsigned char bytes[45U];
	unsigned int j = 0U;
	for (unsigned int i = 0U; i < 180U; i++) {
		unsigned int n = INTERLEAVE_TABLE_9_20[i];

		bool s0 = READ_BIT(convolved, j) != 0U;
		j++;

		bool s1 = READ_BIT(convolved, j) != 0U;
		j++;

		WRITE_BIT(bytes, n, s0);

		n++;
		WRITE_BIT(bytes, n, s1);
	}

	unsigned char* p1 = data;
	unsigned char* p2 = bytes;
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p1, p2, 9U);
		p1 += 18U; p2 += 9U;
	}
}

void YSFEncoder::writeDataFRModeData2(const unsigned char* dt, unsigned char* data)
{
	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;

	unsigned char output[25U];
	for (unsigned int i = 0U; i < 20U; i++)
		output[i] = dt[i] ^ WHITENING_DATA[i];

	CCRC::addCCITT162(output, 22U);
	output[22U] = 0x00U;

	unsigned char convolved[45U];

	CYSFConvolution conv;
	conv.encode(output, convolved, 180U);

	unsigned char bytes[45U];
	unsigned int j = 0U;
	for (unsigned int i = 0U; i < 180U; i++) {
		unsigned int n = INTERLEAVE_TABLE_9_20[i];

		bool s0 = READ_BIT(convolved, j) != 0U;
		j++;

		bool s1 = READ_BIT(convolved, j) != 0U;
		j++;

		WRITE_BIT(bytes, n, s0);

		n++;
		WRITE_BIT(bytes, n, s1);
	}

	unsigned char* p1 = data + 9U;
	unsigned char* p2 = bytes;
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p1, p2, 9U);
		p1 += 18U; p2 += 9U;
	}
}

void YSFEncoder::ysf_scramble(uint8_t *buf, const int len)
{	// buffer is (de)scrambled in place
	static const uint8_t scramble_code[180] = {
	1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1,
	0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1,
	1, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1,
	0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0,
	1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1,
	1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1,
	0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0,
	1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0,
	0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0
	};

	for (int i=0; i<len; i++) {
		buf[i] = buf[i] ^ scramble_code[i];
	}
}

void YSFEncoder::generate_vch_vd2(const uint8_t *a)
{
	uint8_t buf[104];
	uint8_t result[104];
	//unsigned char a[56];
	uint8_t vch[13];
	memset(vch, 0, 13);
/*
	for(int i = 0; i < 7; ++i){
		for(int j = 0; j < 8; ++j){
			a[(8*i)+j] = (1 & (input[i] >> (7-j)));
			//a[((8*i)+j)+1] = (1 & (data[5-i] >> j));
		}
	}
*/
	for (int i=0; i<27; i++) {
		buf[0+i*3] = a[i];
		buf[1+i*3] = a[i];
		buf[2+i*3] = a[i];
	}
	memcpy(buf+81, a+27, 22);
	buf[103] = 0;
	ysf_scramble(buf, 104);

	//uint8_t bit_result[104];
	int x=4;
	int y=26;
	for (int i=0; i<x; i++) {
		for (int j=0; j<y; j++) {
			result[i+j*x] = buf[j+i*y];
		}
	}
	for(int i = 0; i < 13; ++i){
		for(int j = 0; j < 8; ++j){
			//ambe_bytes[i] |= (ambe_frame[((8-i)*8)+(7-j)] << (7-j));
			vch[i] |= (result[(i*8)+j] << (7-j));
		}
	}
	::memcpy(m_vch, vch, 13);
}

void YSFEncoder::writeVDMode2Data(unsigned char* data, const unsigned char* dt)
{
	data += YSF_SYNC_LENGTH_BYTES + YSF_FICH_LENGTH_BYTES;
	
	unsigned char dt_tmp[13];
	::memcpy(dt_tmp, dt, YSF_CALLSIGN_LENGTH);

	for (unsigned int i = 0U; i < 10U; i++)
		dt_tmp[i] ^= WHITENING_DATA[i];

	CCRC::addCCITT162(dt_tmp, 12U);
	dt_tmp[12U] = 0x00U;

	unsigned char convolved[25U];
	CYSFConvolution conv;
	conv.start();
	conv.encode(dt_tmp, convolved, 100U);

	unsigned char bytes[25U];
	unsigned int j = 0U;
	for (unsigned int i = 0U; i < 100U; i++) {
		unsigned int n = INTERLEAVE_TABLE_5_20[i];

		bool s0 = READ_BIT(convolved, j) != 0U;
		j++;

		bool s1 = READ_BIT(convolved, j) != 0U;
		j++;

		WRITE_BIT(bytes, n, s0);

		n++;
		WRITE_BIT(bytes, n, s1);
	}

	unsigned char* p1 = data;
	unsigned char* p2 = bytes;
#ifdef DEBUG
	fprintf(stderr, "AMBE: ");
	for(int i = 0; i < 45; ++i){
		fprintf(stderr, "%02x ", m_ambe[i]);
	}
	fprintf(stderr, "\n");
	fflush(stderr);
#endif
	for (unsigned int i = 0U; i < 5U; i++) {
		::memcpy(p1, p2, 5U);
		if(use_hw){
			char ambe_bits[56];
			unsigned char di_bits[56];
			unsigned char *d = &m_ambe[9*i];
			for(int ii = 0; ii < 7; ++ii){
				for(int j = 0; j < 8; j++){
					ambe_bits[j+(8*ii)] = (1 & (d[ii] >> (7 - j)));
					//ambe_bits[j+(8*ii)] = (1 & (d[ii] >> j));
				}
			}
			for(int ii = 0; ii < 49; ++ii){
				di_bits[ii] = ambe_bits[vd2DVSIInterleave[ii]];
			}
			generate_vch_vd2(di_bits);
		}
		else{
			unsigned char a[56];
			unsigned char *d = &m_ambe[9*i];
			for(int ii = 0; ii < 7; ++ii){
				for(int j = 0; j < 8; ++j){
					a[(8*ii)+j] = (1 & (d[ii] >> (7-j)));
					//a[((8*i)+j)+1] = (1 & (data[5-i] >> j));
				}
			}
			generate_vch_vd2(a);
		}
		::memcpy(p1+5, m_vch, 13);
		p1 += 18U; p2 += 5U;
	}
}
