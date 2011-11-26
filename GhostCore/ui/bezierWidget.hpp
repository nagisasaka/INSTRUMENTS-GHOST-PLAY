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
 * ベジェ曲線表示ウィジェット
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
	 * スクロールバー設定依頼シグナル
	 * 
	 * \param v
	 * 縦スクロールバー最大値 
	 *
	 * \param h
	 * 横スクロールバー最大値
	 *
	 * \param vc
	 * 縦スクロールバー現在値
	 *
	 * \param hc
	 * 横スクロールバー現在値（使用していない；JogTab.hpp を参照）
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
	 * 拡大
	 * 
	 */
	void zoomin();

	/*!
	 * \brief
	 * 縮小
	 * 
	 */
	void zoomout();

	/*!
	 * \brief
	 * 拡大縮小共通処理
	 * 
	 * \param amp
	 * 倍率
	 */
	void zoomCommon(double amp);

	/*!
	 * \brief
	 * 縦スクロール
	 * 
	 * \param amp
	 * スライダーバー位置
	 */
	void verticalScroll(int val);

	/*!
	 * \brief
	 * 横スクロール
	 * 
	 * \param amp
	 * スライダーバー位置
	 */
	void horizontalScroll(int val);

	/*!
	 * \brief
	 * 変換処理
	 * 
	 */
	void convert();
    
	/*!
	 * \brief
	 * 終了時間の設定
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
	 * 背景正方形タイルを作る
	 * 
	 * \param width
	 * 背景正方形タイルの一辺の大きさ
	 * 
	 */
	void makeTile();

	//親ウィジェット
	QWidget *parent;

	//ベジェ曲線
	Bezier* bezier;

	//背景タイル
    QPixmap m_tile;

	//コントロールポイントドラッグ時のマウススタート位置
    QPointF mouseStart;

	//スクロールのための変数
	double prev_hs;
	double prev_vs;

	//タイルサイズ
	double tileSize;

	//ズームインアウトの単位倍率
	static const double zoomAmp;
};

#endif // BezierWidget_H
