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

#ifndef LEVELMETER_H
#define LEVELMETER_H

#include <QWidget>
#include <QTimer>

class LevelMeter : public QWidget
{
	Q_OBJECT
public:
	explicit LevelMeter(QWidget *parent = nullptr);
	void setLevel(qreal value);

protected:
	void paintEvent(QPaintEvent *event) override;
private slots:
	void process_falloff();
private:
	qreal m_level = 0;
	QPixmap m_pixmap;
	QTimer *m_falloff_timer;

signals:

};

#endif // LEVELMETER_H
