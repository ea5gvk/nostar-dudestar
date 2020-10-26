#ifndef M17CODEC_H
#define M17CODEC_H
#include <string>
#include "codec2/codec2.h"

class M17Codec
{
public:
	M17Codec();
	static void encode_callsign(uint8_t *);
	static void decode_callsign(uint8_t *);
	void set_hostname(std::string);
	void set_callsign(std::string);
	void decode_audio(int16_t *, uint8_t *);
	void encode_c2(int16_t *, uint8_t *);
	void set_mode(bool m){ m_c2->codec2_set_mode(m);}
	bool get_mode(){ return m_c2->codec2_get_mode(); }
	CCodec2 *m_c2;
};

#endif // M17CODEC_H
