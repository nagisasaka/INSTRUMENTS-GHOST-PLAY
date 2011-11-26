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

#include <iostream>
#include <math.h>

#include <QPainter>
#include <QPaintEvent>
#include <QPainterPathStroker>
#include <QPainterPath>
#include <QBrush>
#include <QFileDialog>
#include <QPen>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QTimer>

#include "BezierWidget.hpp"
#include "../graph/Bezier.hpp"
#include "../graph/cartesian.hpp"

//�Y�[���P�ʔ{��
const double BezierWidget::zoomAmp = 1.2;

#define MAX(a,b) (((a)>(b))?(a):(b))

BezierWidget::BezierWidget(QWidget *_parent) : parent(_parent)
{
	//// �^�C��������

	tileSize = 40.;
	makeTile();

	//// �x�W�F�Ȑ�������

	//�n�_��ݒ�
	QPointF origin = QPointF(0, this->height()/2.);
	//�n�_�ƏI�_
	Bezier::ControlPoint start = { origin, origin+QPointF(100,100) };
	Bezier::ControlPoint end   = { origin+QPointF(600,0), origin+QPointF(600,0)-QPointF(100,100) };
	//�x�W�F�Ȑ��̍\�z
	bezier = new Bezier(start,end,600,200);

	//// ���̑�

	setMouseTracking(true);
	prev_vs = 0; //��ʃX�N���[���p
	prev_hs = 0; //��ʃX�N���[���p
}

void BezierWidget::drawBackground(QPainter *painter, const QRectF &fullRect)
{
   painter->save();
   painter->setRenderHint(QPainter::Antialiasing);
   painter->drawTiledPixmap(fullRect, m_tile, bezier->GetOrigin());
   painter->restore();
}

void BezierWidget::verticalScroll(int val)
{
	double diff =  (prev_vs-val)*10;
	bezier->Move(QPointF(0,diff));
	prev_vs = val;
	update();
}

void BezierWidget::horizontalScroll(int val)
{
	double diff = (prev_hs-val)*10;
	bezier->Move(QPointF(diff,0));
	prev_hs = val;
	update();
}

void BezierWidget::zoomin(){ zoomCommon(zoomAmp); }
void BezierWidget::zoomout(){ zoomCommon(1.0/zoomAmp);}
void BezierWidget::zoomCommon(double amp)
{
	double scrollBarVMax = 0;
	double scrollBarHMax = 0;
	double scrollBarVCurrent = 0;
	double scrollBarHCurrent = 0;

	//�Y�[������
	tileSize = tileSize * amp;
	makeTile();
	bezier->Scale(amp);

	//�͂ݏo�����������c��̃X�N���[���Ō��؂��Ηǂ��̂�
	double hamidashi_h  = bezier->GetXMaxValue() - (width()-40) ;
	if(hamidashi_h <=0){
		scrollBarHMax = 0;
	}else{
		scrollBarHMax = hamidashi_h/10.;		
	}

	QPointF origin = bezier->GetOrigin();
	//��}�[�W���`�F�b�N
	double v_up = MAX(0, (bezier->GetYMaxValue() - origin.y())/10.);
	//���}�[�W���`�F�b�N
	double v_down = MAX(0, ( bezier->GetYMaxValue() + origin.y() - (height()-20) )/10. );
	//�c�X�N���[���o�[����
	scrollBarVMax = v_up + v_down;
	//�X�N���[���o�[�J�����g�ʒu
	scrollBarVCurrent = v_up;
	//�������L�����Z������
	prev_vs = v_up;
	
	//�J�����g�ʒu�͕ύX����K�v���Ȃ��H
	emit AdjustScrollBar(scrollBarVMax, scrollBarHMax, scrollBarVCurrent, scrollBarHCurrent);
	update();
}

void BezierWidget::paintEvent(QPaintEvent *e)
{
	//�y�C���^�[��o�^�C�`��������w��
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());	
	//�w�i�`��
    drawBackground(&p, rect());
	//�x�W�F�Ȑ��`��
	bezier->Draw(p);
}

void BezierWidget::mousePressEvent(QMouseEvent *e)
{	
	if(e->button() == Qt::LeftButton){
		//�����Ɠo�^
		mouseStart = e->pos();
		bezier->Select(mouseStart);
	}else if(e->button() == Qt::RightButton){
		//�폜�i�������p�X��̏ꍇ�̓R���g���[���|�C���g���ǉ������d�l�c�j
		bezier->Delete(e->pos());
	}
	update();
}

void BezierWidget::mouseMoveEvent(QMouseEvent *e)
{
    QPointF diff = e->pos() - mouseStart;
	bezier->Drag(diff);
	mouseStart = e->pos();
	update();
}

void BezierWidget::mouseReleaseEvent(QMouseEvent *)
{
	bezier->UnSelect();
}

void BezierWidget::resizeEvent(QResizeEvent *e)
{
	bezier->Resize();
	makeTile();
	zoomCommon(1.0);
}

void BezierWidget::makeTile()
{
	//100x100�w�i�^�C��   
	m_tile = QPixmap(tileSize, tileSize);
	m_tile.fill(Qt::white);

	//�����^�C�����쐬
    QPainter pt(&m_tile);
    QColor color(220, 220, 220);
    pt.fillRect(0, 0, tileSize/2., tileSize/2., color);
    pt.fillRect(tileSize/2., tileSize/2., tileSize/2., tileSize/2., color);
    pt.end();
}

//�ϊ��Ɖ^�]
void BezierWidget::setDuration(double v)
{
	bezier->SetDuration(v*1000.);//msec
}

void BezierWidget::convert()
{
	bezier->Convert();
}

QMap<int,double> BezierWidget::GetOperationGraph()
{
	return bezier->GetOperationGraph();
}
