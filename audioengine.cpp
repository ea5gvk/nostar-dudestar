#include "audioengine.h"
#include <QDebug>
//AudioEngine::AudioEngine(QObject *parent) : QObject(parent)
AudioEngine::AudioEngine(QString in, QString out) :
	m_outputdevice(out),
	m_inputdevice(in)
{

}

AudioEngine::~AudioEngine()
{
	//m_indev->disconnect();
	//m_in->stop();
	//m_outdev->disconnect();
	//m_out->stop();
	//delete m_in;
	//delete m_out;
}

QStringList AudioEngine::discover_audio_devices(uint8_t d)
{
	QStringList list;
	QAudio::Mode m = (d) ? QAudio::AudioOutput :  QAudio::AudioInput;
	QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(m);

	for (QList<QAudioDeviceInfo>::ConstIterator it = devices.constBegin(); it != devices.constEnd(); ++it ) {
		//fprintf(stderr, "Playback device name = %s\n", (*it).deviceName().toStdString().c_str());fflush(stderr);
		list.append((*it).deviceName());
	}
	return list;
}

void AudioEngine::init()
{
	QAudioFormat format;
	QAudioFormat tempformat;
	format.setSampleRate(8000);
	format.setChannelCount(1);
	format.setSampleSize(16);
	format.setCodec("audio/pcm");
	format.setByteOrder(QAudioFormat::LittleEndian);
	format.setSampleType(QAudioFormat::SignedInt);

	QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioOutput);

	if(devices.size() == 0){
		fprintf(stderr, "No audio playback hardware found\n");fflush(stderr);
	}
	else{
		QAudioDeviceInfo info(QAudioDeviceInfo::defaultOutputDevice());
		for (QList<QAudioDeviceInfo>::ConstIterator it = devices.constBegin(); it != devices.constEnd(); ++it ) {
			//qDebug() << "Device name = " << (*it).deviceName();
			if((*it).deviceName() == m_outputdevice){
				info = *it;
			}
		}
		if (!info.isFormatSupported(format)) {
			qWarning() << "Raw audio format not supported by backend, trying nearest format.";
			tempformat = info.nearestFormat(format);
			qWarning() << "Format now set to " << format.sampleRate() << ":" << format.sampleSize();
		}
		else{
			tempformat = format;
		}
		//fprintf(stderr, "Using playback device %s\n", info.deviceName().toStdString().c_str());fflush(stderr);

		m_out = new QAudioOutput(info, tempformat, this);
		m_out->setBufferSize(6400);
		m_outdev = m_out->start();
	}

	devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

	if(devices.size() == 0){
		fprintf(stderr, "No audio recording hardware found\n");fflush(stderr);
	}
	else{
		QAudioDeviceInfo info(QAudioDeviceInfo::defaultInputDevice());
		for (QList<QAudioDeviceInfo>::ConstIterator it = devices.constBegin(); it != devices.constEnd(); ++it ) {
			//qDebug() << "Device name = " << (*it).deviceName();
			if((*it).deviceName() == m_inputdevice){
				info = *it;
			}
		}
		if (!info.isFormatSupported(format)) {
			qWarning() << "Raw audio format not supported by backend, trying nearest format.";
			tempformat = info.nearestFormat(format);
			qWarning() << "Format now set to " << format.sampleRate() << ":" << format.sampleSize();
		}
		else{
			tempformat = format;
		}
		//fprintf(stderr, "Using recording device %s\n", info.deviceName().toStdString().c_str());fflush(stderr);
		m_in = new QAudioInput(info, format, this);
	}
}

void AudioEngine::set_output_volume(qreal v)
{
	qDebug() << "set_output_volume() v == " << v;
	m_out->setVolume(v);
}

void AudioEngine::set_input_volume(qreal v)
{
	qDebug() << "set_input_volume() v == " << v;
	m_in->setVolume(v);
}

void AudioEngine::start_capture()
{
	m_audioinq.clear();
	m_indev = m_in->start();
	connect(m_indev, SIGNAL(readyRead()), SLOT(input_data_received()));
}

void AudioEngine::stop_capture()
{
	m_indev->disconnect();
	m_in->stop();
}

void AudioEngine::input_data_received()
{
	QByteArray data;
	qint64 len = m_in->bytesReady();

	if (len > 0){
		data.resize(len);
		m_indev->read(data.data(), len);
/*
		fprintf(stderr, "AUDIOIN: ");
		for(int i = 0; i < len; ++i){
			fprintf(stderr, "%02x ", (unsigned char)data.data()[i]);
		}
		fprintf(stderr, "\n");
		fflush(stderr);
*/
		for(int i = 0; i < len; i+=2){
			m_audioinq.enqueue(((data.data()[i+1] << 8) & 0xff00) | (data.data()[i] & 0xff));
		}
	}
}

void AudioEngine::write(int16_t *pcm, size_t s)
{
	m_outdev->write((const char *) pcm, sizeof(int16_t) * s);
}

uint16_t AudioEngine::read(int16_t *pcm, int s)
{
	if(m_audioinq.size() >= s){
		for(int i = 0; i < s; ++i){
			pcm[i] = m_audioinq.dequeue();
		}
		return 1;
	}
	else{
		//fprintf(stderr, "audio frame not avail size == %d\n", m_audioinq.size());
		return 0;
	}
}
