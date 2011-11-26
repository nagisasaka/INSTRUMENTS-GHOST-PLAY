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

#include "sequenceGenerateTab.hpp"
#include "../ghostcore.hpp"


SequenceGenerateTab::SequenceGenerateTab(GhostCore *_parent) : parent(_parent)
{

	connect(parent->ui.sequenceGenerate, SIGNAL(pressed()), this, SLOT(Generate()));

}


SequenceGenerateTab::~SequenceGenerateTab()
{
}


void SequenceGenerateTab::Generate()
{
	double min = parent->ui.sequenceMin->text().toDouble();
	double max = parent->ui.sequenceMax->text().toDouble();

	double step = parent->ui.sequenceStep->text().toDouble();

	double length = parent->ui.sequenceLength->text().toDouble();

	int index = parent->ui.sequenceReturn->currentIndex();

	QVector<double> sequence;
	double curr = min;

	if(index == 0){

		//のこぎり（max無視）
		int half = length / 2.;
		for(int i=0;i<half;i++){			
			sequence.append(curr);
			curr += step;
		}
		for(int i=half;i>0;i--){
			sequence.append(curr);
			curr -= step;
		}

	}else if(index == 1){

		//Ｓ字（step無視）
		double alpha = max*8.0 / (length*length);

		for(int i=0;i<length/4.;i++){
			double y = alpha*i*i;
			sequence.append(curr+y);
		}

		for(int i=length/4.;i<length*3./4.;i++){
			double y = (-1.)*alpha*(i-length/2.)*(i-length/2.)+max;
			sequence.append(curr+y);
		}

		for(int i=length*3./4.;i<length;i++){
			double y = alpha*(i-length)*(i-length);
			sequence.append(curr+y);
		}

	}else if(index == 2){

		//単調（max無視）
		for(int i=0;i<length;i++){
			sequence.append(curr);
			curr += step;
		}

	}

	QString sequenceData = QString();
	for(int i=0;i<sequence.size();i++){
		sequenceData.append(QString("%1\n").arg(sequence[i]));
	}
	parent->ui.sequenceData->setText(sequenceData);
}

