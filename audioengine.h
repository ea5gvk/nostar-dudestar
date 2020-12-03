#ifndef AUDIOENGINE_H
#define AUDIOENGINE_H

#include <QObject>
#include <QAudioOutput>
#include <QAudioInput>
#include <QQueue>

#define AUDIO_OUT 1
#define AUDIO_IN  0

class AudioEngine : public QObject
{
	Q_OBJECT
public:
	//explicit AudioEngine(QObject *parent = nullptr);
	AudioEngine(QString in, QString out);
	~AudioEngine();
	static QStringList discover_audio_devices(uint8_t d);
	void init();
	void start_capture();
	void stop_capture();
	void write(int16_t *, size_t);
	void set_output_buffer_size(uint32_t b) { m_out->setBufferSize(b); }
	void set_input_buffer_size(uint32_t b) { m_in->setBufferSize(b); }
	void set_output_volume(qreal);
	void set_input_volume(qreal);
	bool frame_available() { return (m_audioinq.size() >= 320) ? true : false; }
	uint16_t read(int16_t *, int);
signals:

private:
	QString m_outputdevice;
	QString m_inputdevice;
	QAudioOutput *m_out;
	QAudioInput *m_in;
	QIODevice *m_outdev;
	QIODevice *m_indev;
	QQueue<int16_t> m_audioinq;

private slots:
	void input_data_received();
};

#endif // AUDIOENGINE_H
