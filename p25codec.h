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

#ifndef P25CODEC_H
#define P25CODEC_H

#include <QObject>
#include <QtNetwork>
#include "audioengine.h"
#include "mbedec.h"
#include "mbeenc.h"
#ifdef USE_FLITE
#include <flite/flite.h>
#endif

class P25Codec : public QObject
{
	Q_OBJECT
public:
	P25Codec(QString callsign, int hostname, QString host, int port);
	~P25Codec();
	unsigned char * get_frame(unsigned char *ambe);
	QString get_callsign() { return m_callsign; }
	uint32_t get_src() { return m_srcid; }
	uint32_t get_dst() { return m_dstid; }
	QString get_host() { return m_host; }
	int get_port() { return m_port; }
	int get_fn() { return m_fn; }
	int get_cnt() { return m_cnt; }
public slots:
	void start_tx();
	void stop_tx();
signals:
	void update();
private:
	int m_p25cnt;
	bool m_tx;
	uint16_t m_txcnt;
	uint8_t m_ttsid;
	QString m_ttstext;
#ifdef USE_FLITE
	cst_voice *voice_slt;
	cst_voice *voice_kal;
	cst_voice *voice_awb;
	cst_voice *voice_rms;
	cst_wave *tts_audio;
#endif
	unsigned char imbe[11U];
	enum{
		DISCONNECTED,
		CONNECTING,
		DMR_AUTH,
		DMR_CONF,
		DMR_OPTS,
		CONNECTED_RW,
		CONNECTED_RO
	} m_status;
	QUdpSocket *m_udp = nullptr;
	QHostAddress m_address;
	QString m_callsign;
	int m_hostname;
	QString m_host;
	int m_port;
	uint32_t m_srcid;
	uint32_t m_dstid;
	uint16_t m_fn;
	uint32_t m_cnt;
	MBEDecoder *m_mbedec;
	MBEEncoder *m_mbeenc;

	QQueue<unsigned char> m_codecq;
	QTimer *m_ping_timer;
	QTimer *m_txtimer;
	AudioEngine *m_audio;

private slots:
	void deleteLater();
	void process_udp();
	void send_ping();
	void send_connect();
	void send_disconnect();

	void transmit();
	void hostname_lookup(QHostInfo i);
	void input_src_changed(int id, QString t) { m_ttsid = id; m_ttstext = t; }
};

#endif // P25CODEC_H
