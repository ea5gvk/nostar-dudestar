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

#ifndef CODEC_H
#define CODEC_H

#include <QObject>
#include <QtNetwork>
#ifdef USE_FLITE
#include <flite/flite.h>
#endif
#include "audioengine.h"
#ifdef AMBEHW_SUPPORTED
#include "serialambe.h"
#endif
#include "mbedec.h"
#include "mbeenc.h"

class Codec : public QObject
{
	Q_OBJECT
public:
	Codec(QString callsign, char module, QString hostname, QString host, int port, QString vocoder, QString audioin, QString audioout);
	~Codec();
	bool get_hwrx() { return m_hwrx; }
	bool get_hwtx() { return m_hwtx; }
	void set_hostname(std::string);
	void set_callsign(std::string);
	void set_input_src(uint8_t s, QString t) { m_ttsid = s; m_ttstext = t; }
	struct MODEINFO {
		qint64 ts;
		int status;
		int stream_state;
		QString callsign;
		QString gw;
		QString gw2;
		QString src;
		QString dst;
		QString usertxt;
		QString netmsg;
		uint32_t gwid;
		uint32_t srcid;
		uint32_t dstid;
		QString host;
		int port;
		bool path;
		char type;
		uint16_t frame_number;
		uint8_t frame_total;
		int count;
		uint32_t streamid;
		bool mode;
	} m_modeinfo;
	enum{
		DISCONNECTED,
		CONNECTING,
		DMR_AUTH,
		DMR_CONF,
		DMR_OPTS,
		CONNECTED_RW,
		CONNECTED_RO
	};
	enum{
		STREAM_NEW,
		STREAMING,
		STREAM_END,
		STREAM_LOST,
		STREAM_IDLE,
		TRANSMITTING,
		STREAM_UNKNOWN
	};
signals:
	void update(Codec::MODEINFO);
	void update_output_level(unsigned short);
protected slots:
	virtual void send_disconnect(){}
	void send_connect();
	void input_src_changed(int id, QString t) { m_ttsid = id; m_ttstext = t; }
	void start_tx();
	void stop_tx();
	void deleteLater();
	void in_audio_vol_changed(qreal);
	void out_audio_vol_changed(qreal);
protected:
	QUdpSocket *m_udp = nullptr;
	QHostAddress m_address;
	char m_module;
	QString m_hostname;
	bool m_tx;
	uint16_t m_txcnt;
	uint16_t m_ttscnt;
	uint8_t m_ttsid;
	QString m_ttstext;
#ifdef USE_FLITE
	cst_voice *voice_slt;
	cst_voice *voice_kal;
	cst_voice *voice_awb;
	cst_wave *tts_audio;
#endif
	QTimer *m_ping_timer;
	QTimer *m_txtimer;
	QTimer *m_rxtimer;
	AudioEngine *m_audio;
	QString m_audioin;
	QString m_audioout;
	uint32_t m_rxwatchdog;
	uint8_t m_rxtimerint;
	uint8_t m_txtimerint;
	QQueue<uint8_t> m_rxcodecq;
	QQueue<uint8_t> m_txcodecq;
	MBEDecoder *m_mbedec;
	MBEEncoder *m_mbeenc;
	QString m_vocoder;
#ifdef AMBEHW_SUPPORTED
	SerialAMBE *m_ambedev;
#endif
	bool m_hwrx;
	bool m_hwtx;
};

#endif // CODEC_H
