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

//ズーム単位倍率
const double BezierWidget::zoomAmp = 1.2;

#define MAX(a,b) (((a)>(b))?(a):(b))

BezierWidget::BezierWidget(QWidget *_parent) : parent(_parent)
{
	//// タイル初期化

	tileSize = 40.;
	makeTile();

	//// ベジェ曲線初期化

	//始点を設定
	QPointF origin = QPointF(0, this->height()/2.);
	//始点と終点
	Bezier::ControlPoint start = { origin, origin+QPointF(100,100) };
	Bezier::ControlPoint end   = { origin+QPointF(600,0), origin+QPointF(600,0)-QPointF(100,100) };
	//ベジェ曲線の構築
	bezier = new Bezier(start,end,600,200);

	//// その他

	setMouseTracking(true);
	prev_vs = 0; //画面スクロール用
	prev_hs = 0; //画面スクロール用
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

	//ズーム処理
	tileSize = tileSize * amp;
	makeTile();
	bezier->Scale(amp);

	//はみ出した分だけ残りのスクロールで見切れれば良いので
	double hamidashi_h  = bezier->GetXMaxValue() - (width()-40) ;
	if(hamidashi_h <=0){
		scrollBarHMax = 0;
	}else{
		scrollBarHMax = hamidashi_h/10.;		
	}

	QPointF origin = bezier->GetOrigin();
	//上マージンチェック
	double v_up = MAX(0, (bezier->GetYMaxValue() - origin.y())/10.);
	//下マージンチェック
	double v_down = MAX(0, ( bezier->GetYMaxValue() + origin.y() - (height()-20) )/10. );
	//縦スクロールバー長さ
	scrollBarVMax = v_up + v_down;
	//スクロールバーカレント位置
	scrollBarVCurrent = v_up;
	//動きをキャンセルする
	prev_vs = v_up;
	
	//カレント位置は変更する必要がない？
	emit AdjustScrollBar(scrollBarVMax, scrollBarHMax, scrollBarVCurrent, scrollBarHCurrent);
	update();
}

void BezierWidget::paintEvent(QPaintEvent *e)
{
	//ペインターを登録，描画条件を指定
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());	
	//背景描画
    drawBackground(&p, rect());
	//ベジェ曲線描画
	bezier->Draw(p);
}

void BezierWidget::mousePressEvent(QMouseEvent *e)
{	
	if(e->button() == Qt::LeftButton){
		//検索と登録
		mouseStart = e->pos();
		bezier->Select(mouseStart);
	}else if(e->button() == Qt::RightButton){
		//削除（ただしパス上の場合はコントロールポイントが追加される仕様…）
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
	//100x100背景タイル   
	m_tile = QPixmap(tileSize, tileSize);
	m_tile.fill(Qt::white);

	//白黒タイルを作成
    QPainter pt(&m_tile);
    QColor color(220, 220, 220);
    pt.fillRect(0, 0, tileSize/2., tileSize/2., color);
    pt.fillRect(tileSize/2., tileSize/2., tileSize/2., tileSize/2., color);
    pt.end();
}

//変換と運転
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
