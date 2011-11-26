/*
 * Ghost Play - Autonomous Violin-Player Imitation Device
 * 
 * Copyright (C) 2009-2012  Masato Fujino <fujino@fairydevices.jp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "renderArea.hpp"
#include "../../actuator/controller.hpp"
#include "../stringRecorder.hpp"
#include "../timeDomainProfile.hpp"

#include "../audioBuffer.hpp"

#include <QPalette>
#include <QPainter>

RenderArea::RenderArea(QWidget *parent)
	:	QWidget(parent)
{
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
}

void RenderArea::paintEvent(QPaintEvent *)
{
	QPainter painter(this);

	//横軸
	painter.setPen(Qt::black);
	painter.drawLine(		
		QPoint(
			painter.viewport().left(), 
			painter.viewport().bottom()/2.
		),		
		QPoint(
			painter.viewport().right(),
			painter.viewport().bottom()/2.
		)
	);

	// 振幅の描画
	//
	// オフセットは横軸のメモリ打ちに使うだけ
	//
	Controller *control = Controller::GetInstance();
	StringRecorder *recorder = control->GetStringRecorder();
	if(recorder->isRecording()){
		//リングバッファ
		//double* buf = recorder->buffer->buffer();
		//リングバッファサイズ
		//int buflen = recorder->bufsize();
		//解析結果取得
		QVector<TimeDomainProfile*> tdp = recorder->TimeDomainAnalyze(0, 0);
		double step1 = (double)(painter.viewport().right() - painter.viewport().left())/(double)tdp.size();
		double step2 = step1 / (double)tdp[0]->analyzeFrameLength;

		for(int i=0;i<tdp.size();i++){

			//生値の描画（緑色）			
			painter.setPen(Qt::green);
			for(int j=0;j<tdp[i]->analyzeFrameLength;j+=16){
				painter.drawLine(
					QPoint(
						painter.viewport().left() + i*step1 + j*step2,
						painter.viewport().bottom()/2.
					),
						QPoint(
						painter.viewport().left() + i*step1 + j*step2,
						painter.viewport().bottom()/2. - (double)(tdp[i]->subwindow[j])*(double)(painter.viewport().bottom())/2.0
					)
				);
			}

			//最大パワーの描画（赤色）
			painter.setPen(Qt::red);
			painter.drawLine(
				QPoint(
					painter.viewport().left() + i*step1,
					painter.viewport().bottom()/2.
				),
				QPoint(
					painter.viewport().left() + i*step1,
					painter.viewport().bottom()/2. - tdp[i]->maxAmplitude*painter.viewport().bottom()/2.
				)
			);


		}
	}
	//std::cout << "repaint" << std::endl;
}

void RenderArea::Update()
{
	//ストリングレコーダーテスト
	Controller *control = Controller::GetInstance();
	StringRecorder *recorder = control->GetStringRecorder();
	if(recorder->isRecording()){
		//std::cout << "平均 :" << recorder->getAverage() << ", ";
		//std::cout << "最大 :" << recorder->getMax() << ", ";
		//std::cout << "F0   :" << recorder->getFzeroAvg() << std::endl;
		update();
	}
}