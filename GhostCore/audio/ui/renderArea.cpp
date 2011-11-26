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

	//����
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

	// �U���̕`��
	//
	// �I�t�Z�b�g�͉����̃������ł��Ɏg������
	//
	Controller *control = Controller::GetInstance();
	StringRecorder *recorder = control->GetStringRecorder();
	if(recorder->isRecording()){
		//�����O�o�b�t�@
		//double* buf = recorder->buffer->buffer();
		//�����O�o�b�t�@�T�C�Y
		//int buflen = recorder->bufsize();
		//��͌��ʎ擾
		QVector<TimeDomainProfile*> tdp = recorder->TimeDomainAnalyze(0, 0);
		double step1 = (double)(painter.viewport().right() - painter.viewport().left())/(double)tdp.size();
		double step2 = step1 / (double)tdp[0]->analyzeFrameLength;

		for(int i=0;i<tdp.size();i++){

			//���l�̕`��i�ΐF�j			
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

			//�ő�p���[�̕`��i�ԐF�j
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
	//�X�g�����O���R�[�_�[�e�X�g
	Controller *control = Controller::GetInstance();
	StringRecorder *recorder = control->GetStringRecorder();
	if(recorder->isRecording()){
		//std::cout << "���� :" << recorder->getAverage() << ", ";
		//std::cout << "�ő� :" << recorder->getMax() << ", ";
		//std::cout << "F0   :" << recorder->getFzeroAvg() << std::endl;
		update();
	}
}