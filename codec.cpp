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
#include "codec.h"

#ifdef USE_FLITE
extern "C" {
extern cst_voice * register_cmu_us_slt(const char *);
extern cst_voice * register_cmu_us_kal16(const char *);
extern cst_voice * register_cmu_us_awb(const char *);
}
#endif

Codec::Codec(QString callsign, char module, QString hostname, QString host, int port, QString vocoder,QString audioin, QString audioout) :
	m_module(module),
	m_hostname(hostname),
	m_tx(false),
	m_ttsid(0),
	m_audioin(audioin),
	m_audioout(audioout),
	m_rxwatchdog(0),
#ifdef Q_OS_WIN
	m_rxtimerint(19),
#else
	m_rxtimerint(20),
#endif
	m_txtimerint(19),
	m_vocoder(vocoder),
	m_ambedev(nullptr),
	m_hwrx(false),
	m_hwtx(false)
{
	m_modeinfo.callsign = callsign;
	m_modeinfo.gwid = 0;
	m_modeinfo.srcid = 0;
	m_modeinfo.dstid = 0;
	m_modeinfo.host = host;
	m_modeinfo.port = port;
	m_modeinfo.count = 0;
	m_modeinfo.frame_number = 0;
	m_modeinfo.frame_total = 0;
	m_modeinfo.streamid = 0;
	m_modeinfo.stream_state = STREAM_IDLE;
#ifdef USE_FLITE
	flite_init();
	voice_slt = register_cmu_us_slt(nullptr);
	voice_kal = register_cmu_us_kal16(nullptr);
	voice_awb = register_cmu_us_awb(nullptr);
#endif
}

Codec::~Codec()
{
}

void Codec::in_audio_vol_changed(qreal v){
	m_audio->set_input_volume(v);
}

void Codec::out_audio_vol_changed(qreal v){
	m_audio->set_output_volume(v);
}

void Codec::send_connect()
{
	m_modeinfo.status = CONNECTING;
	QHostInfo::lookupHost(m_modeinfo.host, this, SLOT(hostname_lookup(QHostInfo)));
}

void Codec::start_tx()
{
	//std::cerr << "Pressed TX buffersize == " << audioin->bufferSize() << std::endl;
	qDebug() << "start_tx() " << m_ttsid << " " << m_ttstext;
	if(m_hwtx){
		m_ambedev->clear_queue();
	}
	m_txcodecq.clear();
	m_tx = true;
	m_txcnt = 0;
	m_ttscnt = 0;
	m_rxtimer->stop();
	m_modeinfo.streamid = 0;
#ifdef USE_FLITE

	if(m_ttsid == 1){
		tts_audio = flite_text_to_wave(m_ttstext.toStdString().c_str(), voice_kal);
	}
	else if(m_ttsid == 2){
		tts_audio = flite_text_to_wave(m_ttstext.toStdString().c_str(), voice_awb);
	}
	else if(m_ttsid == 3){
		tts_audio = flite_text_to_wave(m_ttstext.toStdString().c_str(), voice_slt);
	}
#endif
	if(!m_txtimer->isActive()){
		if(m_ttsid == 0){
			m_audio->set_input_buffer_size(640);
			m_audio->start_capture();
			//audioin->start(&audio_buffer);
		}
		m_txtimer->start(m_txtimerint);
	}
}

void Codec::stop_tx()
{
	m_tx = false;
}

void Codec::deleteLater()
{
	if(m_modeinfo.status == CONNECTED_RW){
		m_udp->disconnect();
		m_ping_timer->stop();
		send_disconnect();
		delete m_audio;
		if(m_hwtx){
			delete m_ambedev;
		}
	}
	m_modeinfo.count = 0;
	QObject::deleteLater();
}
