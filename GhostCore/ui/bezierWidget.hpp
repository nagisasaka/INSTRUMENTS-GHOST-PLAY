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

#ifndef __BEZIER_WIDGET_HPP__
#define __BEZIER_WIDGET_HPP__

#include <QtGui/QWidget>
#include <QPointF>
#include <QPixmap>
#include <QPushButton>
#include <QMap>

class Cartesian;
class Bezier;

/*!
 * \brief
 * �x�W�F�Ȑ��\���E�B�W�F�b�g
 * 
 * \see
 * bezier.hpp
 */
class BezierWidget : public QWidget
{
    Q_OBJECT

signals:
    /*!
	 * \brief
	 * �X�N���[���o�[�ݒ�˗��V�O�i��
	 * 
	 * \param v
	 * �c�X�N���[���o�[�ő�l 
	 *
	 * \param h
	 * ���X�N���[���o�[�ő�l
	 *
	 * \param vc
	 * �c�X�N���[���o�[���ݒl
	 *
	 * \param hc
	 * ���X�N���[���o�[���ݒl�i�g�p���Ă��Ȃ��GJogTab.hpp ���Q�Ɓj
	 * 
	 * \see
	 * JogTab.hpp
	 */
	void AdjustScrollBar(double v, double h, double vc, double hc);

public:

	BezierWidget(QWidget *parent = 0);
	~BezierWidget(){}
	
public slots:

	/*!
	 * \brief
	 * �g��
	 * 
	 */
	void zoomin();

	/*!
	 * \brief
	 * �k��
	 * 
	 */
	void zoomout();

	/*!
	 * \brief
	 * �g��k�����ʏ���
	 * 
	 * \param amp
	 * �{��
	 */
	void zoomCommon(double amp);

	/*!
	 * \brief
	 * �c�X�N���[��
	 * 
	 * \param amp
	 * �X���C�_�[�o�[�ʒu
	 */
	void verticalScroll(int val);

	/*!
	 * \brief
	 * ���X�N���[��
	 * 
	 * \param amp
	 * �X���C�_�[�o�[�ʒu
	 */
	void horizontalScroll(int val);

	/*!
	 * \brief
	 * �ϊ�����
	 * 
	 */
	void convert();
    
	/*!
	 * \brief
	 * �I�����Ԃ̐ݒ�
	 * 
	 */
	void setDuration(double v);

	QMap<int,double> GetOperationGraph();

  protected slots:

      void paintEvent(QPaintEvent *e);
      void mousePressEvent(QMouseEvent *e);
      void mouseMoveEvent(QMouseEvent *e);
      void mouseReleaseEvent(QMouseEvent *e);
      
  protected:

	  void drawBackground(QPainter *painter, const QRectF &rect);
      virtual void resizeEvent(QResizeEvent *);

private:
	/*!
	 * \brief
	 * �w�i�����`�^�C�������
	 * 
	 * \param width
	 * �w�i�����`�^�C���̈�ӂ̑傫��
	 * 
	 */
	void makeTile();

	//�e�E�B�W�F�b�g
	QWidget *parent;

	//�x�W�F�Ȑ�
	Bezier* bezier;

	//�w�i�^�C��
    QPixmap m_tile;

	//�R���g���[���|�C���g�h���b�O���̃}�E�X�X�^�[�g�ʒu
    QPointF mouseStart;

	//�X�N���[���̂��߂̕ϐ�
	double prev_hs;
	double prev_vs;

	//�^�C���T�C�Y
	double tileSize;

	//�Y�[���C���A�E�g�̒P�ʔ{��
	static const double zoomAmp;
};

#endif // BezierWidget_H
