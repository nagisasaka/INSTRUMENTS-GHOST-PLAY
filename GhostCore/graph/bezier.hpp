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
 * ベジェ曲線クラス
 * 
 * \remarks
 * 他の曲線をサポートする場合は，この上に抽象クラスを作る形が良いと思います．
 * 
 * \see
 * bezierWidget.hpp | cartesian.hpp
 */
class Bezier
{
public:
	/*!
	 * \brief
	 * ベジェ曲線のコントロールポイント構造体
	 *
	 * パス上の点とハンドルの先端の点のセット
	 *
	 */
	typedef struct ControlPointStruct{
		QPointF p; //パス上
		QPointF pp; //ハンドル先端（pを中心に点対称位置）
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
	 * 全体を平行移動する
	 * 
	 * \param translate
	 * 平行移動量
	 * 
	 */
	void Move(const QPointF &translate);

	/*!
	 * \brief
	 * コントロールポイント，ハンドル等を適切に移動する
	 * 
	 * \param translate
	 * 平行移動量
	 * 
	 */
	void Drag(const QPointF &translate);

	/*!
	 * \brief
	 * コントロールポイントを選択する．選択位置がパス上かつコントロールポイントでない時はコントロールポイントを追加する．
	 * コントロールポイントが選択された時は，selected クラス変数に選択されたコントロールポイントのポインタが記録される．
	 * コントロールポイントが追加された時は，selected クラス変数はゼロのまま（自動的に選択状態にはならない）．
	 * 
	 * \param pos
	 * ポイント
	 * 
	 * \returns
	 * コントロールポイントのインデックス
	 * 
	 */
	int Select(const QPointF &pos);

	/*!
	 * \brief
	 * 選択状態解除
	 * 
	 */
	void UnSelect();

	/*!
	 * \brief
	 * リサイズに関する処理
	 * 
	 */
	void Resize();

	/*!
	 * \brief
	 * 指定した位置のコントロールポイントを削除する
	 * 
	 * \param pos
	 * 位置
	 * 
	 */
	void Delete(const QPointF &pos);

	/*!
	 * \brief
	 * 拡大縮小する
	 * 
	 * \param val
	 * 拡大縮小率
	 * 
	 */
	void Scale(double val);

	/*!
	 * \brief
	 * 描画する．軸描画はオプション．
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
	* コントロールポイントを追加する
	* 
	* \param pos
	* 追加位置
	* 
	* \returns
	* 追加したコントロールポイントのID
	* 
	*/
	int AddControlPoint(const QPointF &pos);

   /*!
	* \brief
	* キーポイントを示す丸印を表示する
	* 
	* \param painter
	* QPainter
	* 
	* \param pt
	* キーポイントの座標
	*/
	void drawPoint(QPainter *painter, const QPointF &pos);

	/*!
	* \brief
	* コントロールを表示する
	* 
	* \param painter
	* QPainter
	* 
	* \param start
	* コントロールの始点座標
	* 
	* \param end
	* コントロールの終点座標
	* 
	*/
	void drawLine(QPainter *painter, const QPointF &start, const QPointF &end);

	/*!
	 * \brief
	 * 基準点に対する参照点の対称点を返す
	 * 
	 * \param center
	 * 基準点
	 * 
	 * \param pos
	 * 参照点
	 * 
	 * \returns
	 * 対称点
	 * 
	 */
	QPointF mirror(const QPointF &center, const QPointF &pos)
	{
		QPointF dif = pos - center;
		return center - dif;		
	}

	/*!
	 * \brief
	 * 二点が重なっているかどうか
	 * 
	 * \param pos
	 * 円の中心
	 * 
	 * \param coord
	 * 探索したい座標
	 * 
	 * \returns
	 * true : 重なっている
	 : false: 重なっていない
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

	//コントロールポイント
	QVector<ControlPoint> ControlPoints;

	//基準座標
	Cartesian* system;

	//ベジェ曲線のパス（幅あり）
	QPainterPath strokedBand;

	//ベジェ曲線のパス（幅なし）
	QPainterPath strokedLine;

	//時間軸方向に10msec毎に離散化されたベジェ曲線の値，すなわち [0[0,100], 1[0,100], 2[0,100], ... ,全長(msec)/10[0,100]]
	//ベジェ曲線の値は[0,100]に正規化されていることに注意．
	QMap<int,double> OperationGraph;
	//QVector<double> OperationGraph;

	//全長
	double duration;

	//選択されたコントロールポイント
    ControlPoint *selected;

	//コントロールポイント移動のタイプ
	int movetype;

	//コントロールポイントのサイズ
	static const int pointSize;

	//軸を描画するかどうか
	bool flagDrawAxis;

	bool flagDrawOperationGraph;
};

#endif