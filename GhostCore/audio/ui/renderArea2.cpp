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

#include "renderArea2.hpp"
#include "../../actuator/controller.hpp"
#include "../stringRecorder.hpp"
#include "../timeDomainProfile.hpp"

#include "../audioBuffer.hpp"

#include <QPalette>
#include <QPainter>

RenderArea2::RenderArea2(QWidget *parent)
	:	QWidget(parent)
{
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
	fzero = 0;
}

void RenderArea2::paintEvent(QPaintEvent *)
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

		//解析結果取得
		QVector<FrequencyDomainProfile*> fdp = recorder->FrequencyDomainAnalyze(1024, 0);//終端から（最新）の1024*20サンプル（約0.2秒分）
		Sleep(1);
		int windowSize = fdp[0]->windowSize;
		double step1 = (double)(painter.viewport().right() - painter.viewport().left())/(double)(windowSize/2.);

		//描画用		
		double *cps = (double*)calloc(fdp[0]->windowSize/2., sizeof(double));

		for(int i=0;i<windowSize/2.;i++){
			cps[i] = fdp[fdp.size()-1]->cps[i];
		}

		//FZEROの取得
		fzero = fdp.at(fdp.size()-1)->F0;
		//std::cout << fzero << std::endl;

		Sleep(1);
		for(int i=0;i<fdp.size();i++){
			delete fdp[i];
			fdp[i] = 0;
		}

				
		//CPS値の描画						
		int dc_cut = 10;
		double tekitou = 4.0;
		painter.setPen(Qt::blue);
		for(int j=dc_cut;j<windowSize/2.;j++){
			painter.drawLine(
				QPoint(
					painter.viewport().left() + j*step1,
					painter.viewport().bottom()
				),
				QPoint(
					painter.viewport().left() + j*step1,
					painter.viewport().bottom() - cps[j]*tekitou*painter.viewport().bottom()
				)
			);
		}

		delete [] cps;

	}
	//std::cout << "repaint" << std::endl;
}

void RenderArea2::Update()
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