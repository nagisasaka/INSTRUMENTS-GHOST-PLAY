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

#include <QPainter>
#include <QPointF>

#include "bezier.hpp"
#include "actuator/controller.hpp"

#include <iostream>

#define MAX(a,b) (((a)>(b))?(a):(b))

//�_�T�C�Y
const int Bezier::pointSize = 5;

Bezier::Bezier(ControlPoint startPoint, ControlPoint endPoint, double x_max, double y_max)
{
	std::cout << "�x�W�F�Ȑ���������" << std::endl;

	//�n�_�ƏI�_��ݒ�
	ControlPoints.append(startPoint);
	ControlPoints.append(endPoint);

	//���W�n��ݒ�
	system = new Cartesian(startPoint.p, x_max, y_max);

	//�ړ��p�̏���������
	selected = 0;
	movetype = 0;

	//�t���O��������
	flagDrawAxis = true;
	flagDrawOperationGraph = false;
}

void Bezier::Convert()
{
	double whole_msec = duration;
	std::cout << "�S��: " << duration << "[msec]" << std::endl;
	OperationGraph.clear();	

	//�� �r������
	//��Ȃ̉��t���Ԃ�10���i600�b�j�Ƃ��āC�^�]�O���t�������̏ꍇ��1000����1�ŋ�؂�ƁG
	//600,000�~���b��1,000��600�~���b�i0.6�b�j
	//���l��1������1�ŋ�؂�ƁG
	//600,000�~���b��1,0000��60�~���b�i0.06�b�j
	//���l��10������1�ŋ�؂�ƁG
	//600,000�~���b��100,000��6�~���b�i0.006�b�j
	//���l��100������1�ŋ�؂�ƁG
	//600,000�~���b��1,000,000��0.6�~���b�i0.0006�b�j
	//���ۂɂ͉^�]�O���t�͋Ȑ��ł���C�����̏ꍇ�������̒����͒����̂ŁC���̐���l���𑜓x�͍r���Ȃ�D
	
	//�� �ۑ�
	//�^�]�O���t�̗��U������܂��ł����Ă��C�T���v�����O�_�Ɠ_�̊Ԃ́C�����ŕ⊮���邱�Ƃ��ł���D
	//�����⊮�͐ڑ������̗��U�I���炩���𑹂Ȃ��̂ŁC�ǂ̒��x�T���v�����O���r�����邱�Ƃ��ł��邩�́C
	//���ۂɉ^�]���Ă݂Ȃ��Ɣ��f�ł��Ȃ��D

	//�� ����
	//�Ƃ肠�����^�]�O���t�̋Ȑ��ɉ�����10������1�i1.0e-5�j�ŋ敪���Ă݂�
	qreal step  = 1.0e-5;
	std::cout << "���U��:" << step << std::endl;	

	//�x�����H
	//QVector<double> key;
	//QVector<double> val;

	int elem = 1.0/step + 2;
	double *key = (double*)malloc(elem*sizeof(double));
	double *val = (double*)malloc(elem*sizeof(double));

	QPointF origin = GetOrigin();
	double y_max = GetYMaxValue();
	double x_max = GetXMaxValue();
	qreal pos   = 0.0;
	int counter = 0;
	std::cout << "�p�X�敪�����J�n" << std::endl;	
	QPointF coffset;
	while(pos < 1.0){
		//�p�X��̓_�𓾂�
		QPointF p = strokedLine.pointAtPercent(pos);
		//�I�t�Z�b�g��߂�
		QPointF c = p - origin;
		//Y���̒l��[0,100]�ɕϊ��C�ۑ�
		double v = c.y()*100. / y_max;
		//val.append(v);
		val[counter] = v;
		//X���̒l��[0,whole_msec]�ɕϊ�
		double t = (c.x()*whole_msec) / x_max;
		//key.append(t);
		key[counter] = t;
		//�p�X��̃X�e�b�v��i�߂�
		pos += step;
		counter++;
		if(counter % 10000 == 0){
			std::cout << "�p�X�敪����: " << counter << std::endl;
		}
	}

	//�� ���ԕ����ɏĂ������������⊮
	//���쐬�����x�W�F�Ȑ����C�������W�n��ŉ�͓I�ɉ�����΂���ɉz�������Ƃ͂Ȃ��ł����c
	//�p�X�ɉ����ăp�X���敪����`�œ���ꂽ�f�[�^�񂩂�C�����ɉ����ċ敪�����f�[�^��ւ̕ϊ�
	//10msec���Ƃɒl�����肵�Ă����D�S�̒�����whole_msec�ŗ^�����Ă���D
	int front = 1; // ��͍ψʒu
	int time =  0; // ���ݎ���
	//int size = key.size();
	int size = elem;
	bool found = false;
	int i=0;
	int j=0;
	std::cout << whole_msec << "�~���b��" << Controller::GetCycle() << "�~���b�Ԋu�ŗ��U�ϊ��J�n..." << std::endl;	
	for(i=0;i<(double)whole_msec/(double)Controller::GetCycle();++i){
		time = i*(double)Controller::GetCycle();
		found = false;
		for(j=front;j<size;++j){
			//if(key[j-1] < time && time <= key[j]){
			if(time <= key[j]){
				//�Ƃ肠���������⊮����
				OperationGraph.insert(i, (double)val[j]); // �[�_�Ƀx�^�t������
				found = true;
				front = MAX(0,j-2);//���Ԃ͐i�ނ݂̂Ȃ̂ŁC�����Ԃ̈�O�܂ł���͍ς݂Ƃ���D
				break;
			}
		}
		if(!found){
			std::cout << "[ BEZIER ] �ϊ��G���[: i=" << i << ", j=" << j << std::endl;
			throw "[ BEZIER ] �ϊ��G���[";
		}
	}
	//�������Ō�ɗ��Ƃ�
	OperationGraph.insert(i, 0.);
	std::cout << "�f�[�^�ϊ��I��" << std::endl;
	flagDrawOperationGraph = true;

	free(key);
	free(val);
}

void Bezier::DrawOperationGraph(QPainter *p)
{
   p->save();
   QPen pen(QColor(0, 0, 255),1,Qt::SolidLine);
   pen.setWidth(3);
   p->setPen(pen);
   p->setBrush(Qt::NoBrush);
   QPointF origin = GetOrigin();

   QMapIterator<int,double> i(OperationGraph);
   while(i.hasNext()){
	   i.next();
	   QPointF s = origin+QPointF(i.key()*(system->GetXMaxValue()/(duration/10.)), 
								  i.value()*system->GetYMaxValue()/100.);
	   if(i.hasNext()){
		   i.next();
		   QPointF e = origin+QPointF(i.key()*(system->GetXMaxValue()/(duration/10.)), 
									  i.value()*system->GetYMaxValue()/100.);
		   p->drawLine(s, e);
		   //�j���ɂ��ĊԊu�����₷������
		   //i.previous(); // �����ɂ���
	   }
	}
	p->restore();
}

void Bezier::Scale(double val)
{
	//�x�W�F�Ȑ��g��k��
	QPointF backVector = ControlPoints[0].p - (ControlPoints[0].p*val);
	for(int i=0;i<ControlPoints.size();i++){
		ControlPoints[i].p = ControlPoints[i].p * val;
		ControlPoints[i].pp = ControlPoints[i].pp * val;
	}
	//���_�߂�
	Move(backVector);
	//���_�Đݒ�
	system->SetOrigin(ControlPoints[0].p);
	system->Scale(val);
}


int Bezier::AddControlPoint(const QPointF &pos)
{
	ControlPoint cp;
	cp.p  = pos;
	cp.pp = pos + QPointF(50,0);
	int index = 0;
	for(int i=0;i<ControlPoints.size()-1;i++){
		if(ControlPoints[i].p.x() < pos.x() && pos.x() <= ControlPoints[i+1].p.x()){
			index = i+1;
			std::cout << "�R���g���[���|�C���g��ǉ�: " << index << std::endl;			
			ControlPoints.insert(index,cp);
			break;
		}
	}
	return index;
}

void Bezier::Move(const QPointF &translate)
{
	//���_�Đݒ�
	system->SetOrigin(system->GetOrigin() + translate);	
	//�x�W�F�Ȑ��ړ�
	for(int i=0;i<ControlPoints.size();++i){
		ControlPoints[i].p = ControlPoints[i].p + translate;
		ControlPoints[i].pp= ControlPoints[i].pp+ translate;
	}
}

void Bezier::Drag(const QPointF &translate)
{
    if (selected) {
		if(movetype == 1){
			//�n���h��
	        (*selected).pp += translate;
		}else if(movetype == 2){
			//�Z���^�[
	        (*selected).p  += translate;
	        (*selected).pp += translate;
		}else if(movetype == 3){
			//�t�n���h��
			(*selected).pp -= translate;
		}
    } 
}

void Bezier::UnSelect()
{
	selected = 0;
	movetype = 0;
}

void Bezier::Resize()
{
	system->SetOrigin(ControlPoints[0].p);
}

void Bezier::Delete(const QPointF &pos)
{
	//�I������
	int id = Select(pos); // �i�p�X����E�N���b�N���Ă��ǉ�����Ă��܂����c�j
	if(movetype == 2){
		//�R���g���[���|�C���g���I������Ă���ꍇ�̂�
		ControlPoints.remove(id);
	}
}

int Bezier::Select(const QPointF &pos)
{
	// (1) �_���N���b�N�������H
	selected = 0;
	movetype = 0;
	int size = ControlPoints.size();
	for(int i=0;i<size;++i){
		//�n���h���̕Е�
		if(markContains(ControlPoints[i].pp, pos)){
			selected = &(ControlPoints[i]);
			movetype = 1; // �n���h���̓o�^���ipp�j�𓮂���
			return i;
		}
		//�x�W�F�̗��[�ł͂Ȃ��ꍇ�̂�
		if(!(i==0 || i == size -1)){
			//�R���g���[���|�C���g�i�p�X��̓_�̂��Ɓj
			if(markContains(ControlPoints[i].p, pos)){
				selected = &(ControlPoints[i]);
				movetype = 2;// �n���h�����ƃR���g���[���|�C���g�S�̂𓮂���
				return i;
			}
			//�n���h���̋t������[
			if(markContains(mirror(ControlPoints[i].p, ControlPoints[i].pp), pos)){
				selected = &(ControlPoints[i]);
				movetype = 3; // �n���h���̃~���[���imirror(pp)�j�݂̂𓮂���
				return i;
			}
		}				
	}

	// (2) �p�X����N���b�N�������H		
	if(strokedBand.contains(pos)){
		//�R���g���[���|�C���g��ǉ����Ēǉ������R���g���[���|�C���g��ID��Ԃ�
		return AddControlPoint(pos);
	}

	// (3) ��L�̂ǂ��ł��Ȃ��_
	return 0;
}

void Bezier::Draw(QPainter& p)
{
	//����`��
	if(flagDrawAxis){
		DrawAxis(&p);
	}

	//�^�]�O���t��`��
	if(flagDrawOperationGraph){
		DrawOperationGraph(&p);
	}

	//�n�_��o�^
	QPainterPath path(ControlPoints[0].p);

	drawPoint(&p, ControlPoints[0].p);
	drawPoint(&p, ControlPoints[0].pp);
	drawLine(&p, ControlPoints[0].p, ControlPoints[0].pp);

	//���ԃR���g���[���|�C���g�̐�
	int size = ControlPoints.size();
	for(int i=1;i<ControlPoints.size();i++){ // �P�X�^�[�g		

		//�J�����g�R���g���[��
		QPointF center = ControlPoints[i].p;
		QPointF r_handle = ControlPoints[i].pp;
		QPointF i_handle = mirror(ControlPoints[i].p, ControlPoints[i].pp);

		//�p�X�̕`��
		QPointF c_con;
		if(i==size-1){
			c_con = r_handle;
		}else{
			c_con = i_handle;		
		}
		path.cubicTo(ControlPoints[i-1].pp, c_con, center);

		//�J�����g�R���g���[���̕`��
		drawPoint(&p, center);
		drawPoint(&p, r_handle);
		drawLine(&p, center, r_handle);
		if(i != size - 1){
			//�Ώ̎���`�悵�Ȃ�
			drawPoint(&p, i_handle);
			drawLine(&p, center, i_handle);
		}
	}
	p.strokePath(path, QPen(Qt::red, 2));

	//�p�X�̕ۑ�
	QPainterPathStroker stroker;
	//������
	stroker.setWidth(10);
	strokedBand = stroker.createStroke(path);	
	//���Ȃ�
	stroker.setWidth(0.00001);
	strokedLine = stroker.createStroke(path);
}

void Bezier::DrawAxis(QPainter *painter)
{
   painter->save();
   QPen pen(QColor(0, 0, 0));
   pen.setWidth(1);
   painter->setPen(pen);
   painter->setBrush(Qt::NoBrush);
   //X��
   painter->drawLine(GetOrigin()-QPointF(20,0), system->GetXMax());
   //Y��
   painter->drawLine(GetOrigin(), system->GetYMin());
   painter->drawLine(GetOrigin(), system->GetYMax());
   //Y������K�C�h
   pen.setColor(QColor(0,255,0));
   painter->setPen(pen);
   painter->drawLine(system->GetYMin()-QPointF(20,0), system->GetXMax() + (system->GetYMin() - GetOrigin()));
   painter->drawLine(system->GetYMax()-QPointF(20,0), system->GetXMax() + (system->GetYMax() - GetOrigin()));
   //���x��
   pen.setColor(QColor(0,0,0));
   painter->setPen(pen);
   //���_���x��
   painter->drawText(GetOrigin()+QPointF(-15,+15),QObject::tr("O"));
   painter->restore();
}

void Bezier::drawPoint(QPainter *painter, const QPointF &pos)
{
    painter->save();
	//�������F�w��
    painter->setPen(QColor(50, 100, 120, 200));	
    painter->setBrush(QColor(200, 200, 210, 120));
	//�֊s��`��
    painter->drawEllipse(QRectF(pos.x()-pointSize, pos.y()-pointSize, pointSize*2, pointSize*2));	
    painter->restore();
}

void Bezier::drawLine(QPainter *painter, const QPointF &start, const QPointF &end)
{
    painter->save();
	//�������F�C�j��
    QPen pen(QColor(255, 0, 255, 127), 1, Qt::DashLine);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(start, end);
    painter->restore();
}
