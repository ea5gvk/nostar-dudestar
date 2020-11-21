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

#ifndef DUDESTAR_H
#define DUDESTAR_H

#include <QMainWindow>
#include <QSerialPort>
#include <QtNetwork>
#include <QAudioOutput>
#include <QAudioInput>
#include <QButtonGroup>
#include <QTimer>
#include <QLabel>
#ifdef USE_FLITE
#include <flite/flite.h>
#endif
#include "httpmanager.h"
#include "mbedec.h"
#include "mbeenc.h"
#include "ysfcodec.h"
#include "dmrcodec.h"
#include "p25codec.h"
#include "nxdncodec.h"
#include "m17codec.h"


namespace Ui {
class DudeStar;
}

class DudeStar : public QMainWindow
{
    Q_OBJECT

public:
    explicit DudeStar(QWidget *parent = nullptr);
    ~DudeStar();

signals:
	void input_source_changed(int, QString);
	void dmr_tgid_changed(unsigned int);
	void rate_changed(int);
private:
    void init_gui();
    void transmit();
	void transmitREF();
	void transmitDCS();
	void transmitXRF();
    void calcPFCS(char *d);
    Ui::DudeStar *ui;
	QSerialPort *serial = nullptr;
    QUdpSocket *udp = nullptr;
	QButtonGroup *m17rates;

	enum{
		DISCONNECTED,
		CONNECTING,
		DMR_AUTH,
		DMR_CONF,
		DMR_OPTS,
		CONNECTED_RW,
		CONNECTED_RO
	} connect_status;

	uint16_t usb_pid;
	uint16_t usb_vid;
	QLabel *status_txt;
	QString host;
	QString hostname;
	QString hosts_filename;
	bool m_update_host_files;
	int port;
	QHostAddress address;
    QString config_path;
    QString callsign;
	//QString serial;
	QString dmr_password;
	QString saved_refhost;
	QString saved_dcshost;
	QString saved_xrfhost;
	QString saved_ysfhost;
	QString saved_dmrhost;
	QString saved_p25host;
	QString saved_nxdnhost;
	QString saved_m17host;
    char module;
	uint32_t dmrid;
	uint32_t dmr_srcid;
	uint32_t dmr_destid;
	uint32_t dmrcc;
	uint32_t dmrslot;
	uint8_t dmrcalltype;
	QString protocol;
	uint64_t ping_cnt;
	QThread *m_modethread;
    MBEDecoder *mbe;
	MBEEncoder *mbeenc;
	YSFCodec *m_ysf;
	DMRCodec *m_dmr;
	P25Codec *m_p25;
	NXDNCodec *m_nxdn;
	M17Codec *m_m17;
    QAudioOutput *audio;
	QAudioInput *audioin;
	QBuffer audio_buffer;
	int audiotx_cnt;
	QFile audiofile;
    QIODevice *audiodev;
	QIODevice *audioindev;
    QByteArray user_data;
    QTimer *audiotimer;
    QTimer *txtimer;
	QTimer *ysftimer;
	QTimer *ping_timer;
	QTimer *dmr_header_timer;
	bool muted;
	bool input_muted;
	bool tx;
	bool hwtx;
	bool hwrx;
	bool enable_swtx;
	bool hw_ambe_present;
	QQueue<unsigned char> hw_ambe_audio;
	QQueue<unsigned char> audioinq;
	QQueue<int16_t> s16_le_audioinq;
    QQueue<unsigned char> audioq;
	QQueue<unsigned char> ambeq;
	QQueue<unsigned char> swambeq;
	QQueue<unsigned char> ysfq;
	QMap<uint32_t, QString> m_dmrids;
	QMap<uint16_t, QString> nxdnids;
    const unsigned char header[5] = {0x80,0x44,0x53,0x56,0x54}; //DVSI packet header
#ifdef USE_FLITE
	cst_voice *voice_slt;
	cst_voice *voice_kal;
	cst_voice *voice_awb;
	cst_voice *voice_rms;
	cst_wave *tts_audio;
#endif
	bool text2speech;
	QButtonGroup *tts_voices;
private slots:
    void about();
    void process_connect();
	void process_serial();
	void process_mode_change(const QString &m);
	void process_host_change(const QString &);
	void swrx_state_changed(int);
	void swtx_state_changed(int);
	void tts_changed(int);
	void tts_text_changed(QString);
	void tgid_text_changed(QString);
	void discover_vocoders();
	void connect_to_serial(QString);
    void readyRead();
	void readyReadREF();
	void readyReadXRF();
	void readyReadDCS();
	void update_ysf_data();
	void update_dmr_data();
	void update_p25_data();
	void update_nxdn_data();
	void update_m17_data();
	void m17_rate_changed(int);
	void disconnect_from_host();
    void handleStateChanged(QAudio::State);
    void hostname_lookup(QHostInfo);
    void process_audio();
	void process_mute_button();
	void process_volume_changed(int);
	void process_input_volume_changed(int);
	void process_input_mute_button();
	void audioin_data_ready();
	void process_ping();
    void tx_timer();
	void process_ref_hosts();
	void process_dcs_hosts();
	void process_xrf_hosts();
	void process_ysf_hosts();
	void process_fcs_rooms();
	void process_dmr_hosts();
	void process_p25_hosts();
	void process_nxdn_hosts();
	void process_m17_hosts();
	void check_host_files();
	void update_host_files();
	void process_dmr_ids();
	void update_dmr_ids();
	void process_nxdn_ids();
	void update_nxdn_ids();
    void process_settings();
	void download_file(QString);
	void file_downloaded(QString);
	void tx_dmr_header();
	//void download_dmrid_list();
	void AppendVoiceLCToBuffer(QByteArray& buffer, uint32_t uiSrcId, uint32_t uiDstId) const;
};

#endif // DUDESTAR_H
