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

#ifndef __BEZIER_HPP__
#define __BEZIER_HPP__

#include <QMap>

#include "cartesian.hpp"

/*!
 * \brief
 * �x�W�F�Ȑ��N���X
 * 
 * \remarks
 * ���̋Ȑ����T�|�[�g����ꍇ�́C���̏�ɒ��ۃN���X�����`���ǂ��Ǝv���܂��D
 * 
 * \see
 * bezierWidget.hpp | cartesian.hpp
 */
class Bezier
{
public:
	/*!
	 * \brief
	 * �x�W�F�Ȑ��̃R���g���[���|�C���g�\����
	 *
	 * �p�X��̓_�ƃn���h���̐�[�̓_�̃Z�b�g
	 *
	 */
	typedef struct ControlPointStruct{
		QPointF p; //�p�X��
		QPointF pp; //�n���h����[�ip�𒆐S�ɓ_�Ώ̈ʒu�j
	}ControlPoint;

	Bezier(ControlPoint startPoint, ControlPoint endPoint, double x_max, double y_max);
	~Bezier(){};

	/*!
	 * \brief
	 * Write brief comment for Convert here.
	 * 
	 * \returns
	 * Write description of return value here.
	 * 
	 * \throws <exception class>
	 * Description of criteria for throwing this exception.
	 * 
	 * Write detailed description for GetCurve here.
	 * 
	 * \remarks
	 * Write remarks for GetCurve here.
	 * 
	 * \see
	 * Separate items with the '|' character.
	 */
	void Convert();

	/*!
	 * \brief
	 * �S�̂𕽍s�ړ�����
	 * 
	 * \param translate
	 * ���s�ړ���
	 * 
	 */
	void Move(const QPointF &translate);

	/*!
	 * \brief
	 * �R���g���[���|�C���g�C�n���h������K�؂Ɉړ�����
	 * 
	 * \param translate
	 * ���s�ړ���
	 * 
	 */
	void Drag(const QPointF &translate);

	/*!
	 * \brief
	 * �R���g���[���|�C���g��I������D�I���ʒu���p�X�ォ�R���g���[���|�C���g�łȂ����̓R���g���[���|�C���g��ǉ�����D
	 * �R���g���[���|�C���g���I�����ꂽ���́Cselected �N���X�ϐ��ɑI�����ꂽ�R���g���[���|�C���g�̃|�C���^���L�^�����D
	 * �R���g���[���|�C���g���ǉ����ꂽ���́Cselected �N���X�ϐ��̓[���̂܂܁i�����I�ɑI����Ԃɂ͂Ȃ�Ȃ��j�D
	 * 
	 * \param pos
	 * �|�C���g
	 * 
	 * \returns
	 * �R���g���[���|�C���g�̃C���f�b�N�X
	 * 
	 */
	int Select(const QPointF &pos);

	/*!
	 * \brief
	 * �I����ԉ���
	 * 
	 */
	void UnSelect();

	/*!
	 * \brief
	 * ���T�C�Y�Ɋւ��鏈��
	 * 
	 */
	void Resize();

	/*!
	 * \brief
	 * �w�肵���ʒu�̃R���g���[���|�C���g���폜����
	 * 
	 * \param pos
	 * �ʒu
	 * 
	 */
	void Delete(const QPointF &pos);

	/*!
	 * \brief
	 * �g��k������
	 * 
	 * \param val
	 * �g��k����
	 * 
	 */
	void Scale(double val);

	/*!
	 * \brief
	 * �`�悷��D���`��̓I�v�V�����D
	 * 
	 * \param p
	 * QPainter
	 * 
	 * \see
	 * SetFlagDrawAxis();
	 */
	void Draw(QPainter& p);

	void SetFlagDrawAxis(bool t){ flagDrawAxis = t; }
	bool GetFlagDrawAxis(){ return flagDrawAxis; }
	void SetFlagDrawOperationGraph(bool t){ flagDrawOperationGraph = t; }
	bool GetFlagDrawOperationGraph(){ return flagDrawOperationGraph; }

	QPointF GetOrigin(){ return system->GetOrigin(); }
	double GetXMaxValue(){ return system->GetXMaxValue(); }
	double GetYMaxValue(){ return system->GetYMaxValue(); }
	
	void SetDuration(double v){ duration = v; }
	QMap<int,double> GetOperationGraph(){ return OperationGraph; }
	//QVector<double> GetOperationGraph(){ return OperationGraph; }

private:

	void DrawAxis(QPainter *painter);
	void DrawOperationGraph(QPainter *painter);

   /*!
	* \brief
	* �R���g���[���|�C���g��ǉ�����
	* 
	* \param pos
	* �ǉ��ʒu
	* 
	* \returns
	* �ǉ������R���g���[���|�C���g��ID
	* 
	*/
	int AddControlPoint(const QPointF &pos);

   /*!
	* \brief
	* �L�[�|�C���g�������ۈ��\������
	* 
	* \param painter
	* QPainter
	* 
	* \param pt
	* �L�[�|�C���g�̍��W
	*/
	void drawPoint(QPainter *painter, const QPointF &pos);

	/*!
	* \brief
	* �R���g���[����\������
	* 
	* \param painter
	* QPainter
	* 
	* \param start
	* �R���g���[���̎n�_���W
	* 
	* \param end
	* �R���g���[���̏I�_���W
	* 
	*/
	void drawLine(QPainter *painter, const QPointF &start, const QPointF &end);

	/*!
	 * \brief
	 * ��_�ɑ΂���Q�Ɠ_�̑Ώ̓_��Ԃ�
	 * 
	 * \param center
	 * ��_
	 * 
	 * \param pos
	 * �Q�Ɠ_
	 * 
	 * \returns
	 * �Ώ̓_
	 * 
	 */
	QPointF mirror(const QPointF &center, const QPointF &pos)
	{
		QPointF dif = pos - center;
		return center - dif;		
	}

	/*!
	 * \brief
	 * ��_���d�Ȃ��Ă��邩�ǂ���
	 * 
	 * \param pos
	 * �~�̒��S
	 * 
	 * \param coord
	 * �T�����������W
	 * 
	 * \returns
	 * true : �d�Ȃ��Ă���
	 : false: �d�Ȃ��Ă��Ȃ�
	 * 
	 */
	bool markContains(const QPointF &pos, const QPointF &coord)
	{
	    QRectF rect(pos.x() - pointSize,
		            pos.y() - pointSize,
			        pointSize*2, pointSize*2);
		QPainterPath path;
		path.addEllipse(rect);
		return path.contains(coord);
	}

	//�R���g���[���|�C���g
	QVector<ControlPoint> ControlPoints;

	//����W
	Cartesian* system;

	//�x�W�F�Ȑ��̃p�X�i������j
	QPainterPath strokedBand;

	//�x�W�F�Ȑ��̃p�X�i���Ȃ��j
	QPainterPath strokedLine;

	//���Ԏ�������10msec���ɗ��U�����ꂽ�x�W�F�Ȑ��̒l�C���Ȃ킿 [0[0,100], 1[0,100], 2[0,100], ... ,�S��(msec)/10[0,100]]
	//�x�W�F�Ȑ��̒l��[0,100]�ɐ��K������Ă��邱�Ƃɒ��ӁD
	QMap<int,double> OperationGraph;
	//QVector<double> OperationGraph;

	//�S��
	double duration;

	//�I�����ꂽ�R���g���[���|�C���g
    ControlPoint *selected;

	//�R���g���[���|�C���g�ړ��̃^�C�v
	int movetype;

	//�R���g���[���|�C���g�̃T�C�Y
	static const int pointSize;

	//����`�悷�邩�ǂ���
	bool flagDrawAxis;

	bool flagDrawOperationGraph;
};

#endif