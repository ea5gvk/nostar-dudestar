#ifndef HTTPMANAGER_H
#define HTTPMANAGER_H

#include <QObject>
#include <QtNetwork>

class HttpManager : public QObject
{
	Q_OBJECT
public:
	explicit HttpManager(QString);
	//void start_request(QString file);

signals:
	void file_downloaded(QString);

private:
	QString m_filename;
	QString m_config_path;
	QNetworkAccessManager *m_qnam;

private slots:
	void process();
	void doRequest();
	void http_finished(QNetworkReply *reply);
};

#endif // HTTPMANAGER_H
