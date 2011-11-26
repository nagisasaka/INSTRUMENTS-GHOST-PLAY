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

#include "renderArea3.hpp"
#include "../../actuator/controller.hpp"
#include "../stringRecorder.hpp"
#include "../timeDomainProfile.hpp"

#include "../audioBuffer.hpp"

#include <QPalette>
#include <QPainter>

RenderArea3::RenderArea3(QWidget *parent)
	:	QWidget(parent)
{
	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);
}

void RenderArea3::paintEvent(QPaintEvent *)
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

		//��͌��ʎ擾
		QVector<FrequencyDomainProfile*> fdp = recorder->FrequencyDomainAnalyze(1024, 0);//�I�[����i�ŐV�j��1024*20�T���v���i��0.2�b���j
		int windowSize = fdp[0]->windowSize;
		double step1 = (double)(painter.viewport().right() - painter.viewport().left())/(double)(windowSize/2.);

		//�`��p
		double *alog = (double*)calloc(fdp[0]->windowSize/2., sizeof(double));

		for(int i=0;i<windowSize/2.;i++){
			alog[i] = fdp[fdp.size()-1]->dft[i];
		}
				
		for(int i=0;i<fdp.size();i++){
			delete fdp[i];
			fdp[i] = 0;
		}
				
		//ALOG�l�̕`��						
		int dc_cut = 0;
		double tekitou = 1.0;
		painter.setPen(Qt::blue);
		for(int j=dc_cut;j<windowSize/2.;j++){
			painter.drawLine(
				QPoint(
					painter.viewport().left() + j*step1,
					painter.viewport().bottom()
				),
				QPoint(
					painter.viewport().left() + j*step1,
					painter.viewport().bottom() - alog[j]*tekitou*painter.viewport().bottom()
				)
			);
		}

		delete [] alog;

	}
	//std::cout << "repaint" << std::endl;
}

void RenderArea3::Update()
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