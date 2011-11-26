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

//点サイズ
const int Bezier::pointSize = 5;

Bezier::Bezier(ControlPoint startPoint, ControlPoint endPoint, double x_max, double y_max)
{
	std::cout << "ベジェ曲線を初期化" << std::endl;

	//始点と終点を設定
	ControlPoints.append(startPoint);
	ControlPoints.append(endPoint);

	//座標系を設定
	system = new Cartesian(startPoint.p, x_max, y_max);

	//移動用の情報を初期化
	selected = 0;
	movetype = 0;

	//フラグを初期化
	flagDrawAxis = true;
	flagDrawOperationGraph = false;
}

void Bezier::Convert()
{
	double whole_msec = duration;
	std::cout << "全長: " << duration << "[msec]" << std::endl;
	OperationGraph.clear();	

	//■ 荒い推定
	//一曲の演奏時間を10分（600秒）として，運転グラフが直線の場合で1000分の1で区切ると；
	//600,000ミリ秒÷1,000＝600ミリ秒（0.6秒）
	//同様に1万分の1で区切ると；
	//600,000ミリ秒÷1,0000＝60ミリ秒（0.06秒）
	//同様に10万分の1で区切ると；
	//600,000ミリ秒÷100,000＝6ミリ秒（0.006秒）
	//同様に100万分の1で区切ると；
	//600,000ミリ秒÷1,000,000＝0.6ミリ秒（0.0006秒）
	//実際には運転グラフは曲線であり，直線の場合よりも線の長さは長いので，この推定値より解像度は荒くなる．
	
	//■ 課題
	//運転グラフの離散化が大まかであっても，サンプリング点と点の間は，直線で補完することができる．
	//直線補完は接続部分の離散的滑らかさを損なうので，どの程度サンプリングを荒くすることができるかは，
	//実際に運転してみないと判断できない．

	//■ 実装
	//とりあえず運転グラフの曲線に沿って10万分の1（1.0e-5）で区分してみる
	qreal step  = 1.0e-5;
	std::cout << "離散化:" << step << std::endl;	

	//遅いか？
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
	std::cout << "パス区分処理開始" << std::endl;	
	QPointF coffset;
	while(pos < 1.0){
		//パス上の点を得る
		QPointF p = strokedLine.pointAtPercent(pos);
		//オフセットを戻す
		QPointF c = p - origin;
		//Y軸の値を[0,100]に変換，保存
		double v = c.y()*100. / y_max;
		//val.append(v);
		val[counter] = v;
		//X軸の値を[0,whole_msec]に変換
		double t = (c.x()*whole_msec) / x_max;
		//key.append(t);
		key[counter] = t;
		//パス上のステップを進める
		pos += step;
		counter++;
		if(counter % 10000 == 0){
			std::cout << "パス区分処理: " << counter << std::endl;
		}
	}

	//■ 時間方向に焼き直し＆直線補完
	//＊作成したベジェ曲線が，直交座標系上で解析的に解ければそれに越したことはないですが…
	//パスに沿ってパスを区分する形で得られたデータ列から，横軸に沿って区分したデータ列への変換
	//10msecごとに値を決定していく．全体長さはwhole_msecで与えられている．
	int front = 1; // 解析済位置
	int time =  0; // 現在時刻
	//int size = key.size();
	int size = elem;
	bool found = false;
	int i=0;
	int j=0;
	std::cout << whole_msec << "ミリ秒を" << Controller::GetCycle() << "ミリ秒間隔で離散変換開始..." << std::endl;	
	for(i=0;i<(double)whole_msec/(double)Controller::GetCycle();++i){
		time = i*(double)Controller::GetCycle();
		found = false;
		for(j=front;j<size;++j){
			//if(key[j-1] < time && time <= key[j]){
			if(time <= key[j]){
				//とりあえず直線補完無し
				OperationGraph.insert(i, (double)val[j]); // 端点にベタ付けする
				found = true;
				front = MAX(0,j-2);//時間は進むのみなので，同一区間の一つ前までを解析済みとする．
				break;
			}
		}
		if(!found){
			std::cout << "[ BEZIER ] 変換エラー: i=" << i << ", j=" << j << std::endl;
			throw "[ BEZIER ] 変換エラー";
		}
	}
	//直線を最後に落とす
	OperationGraph.insert(i, 0.);
	std::cout << "データ変換終了" << std::endl;
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
		   //破線にして間隔を見やすくする
		   //i.previous(); // 実線にする
	   }
	}
	p->restore();
}

void Bezier::Scale(double val)
{
	//ベジェ曲線拡大縮小
	QPointF backVector = ControlPoints[0].p - (ControlPoints[0].p*val);
	for(int i=0;i<ControlPoints.size();i++){
		ControlPoints[i].p = ControlPoints[i].p * val;
		ControlPoints[i].pp = ControlPoints[i].pp * val;
	}
	//原点戻し
	Move(backVector);
	//原点再設定
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
			std::cout << "コントロールポイントを追加: " << index << std::endl;			
			ControlPoints.insert(index,cp);
			break;
		}
	}
	return index;
}

void Bezier::Move(const QPointF &translate)
{
	//原点再設定
	system->SetOrigin(system->GetOrigin() + translate);	
	//ベジェ曲線移動
	for(int i=0;i<ControlPoints.size();++i){
		ControlPoints[i].p = ControlPoints[i].p + translate;
		ControlPoints[i].pp= ControlPoints[i].pp+ translate;
	}
}

void Bezier::Drag(const QPointF &translate)
{
    if (selected) {
		if(movetype == 1){
			//ハンドル
	        (*selected).pp += translate;
		}else if(movetype == 2){
			//センター
	        (*selected).p  += translate;
	        (*selected).pp += translate;
		}else if(movetype == 3){
			//逆ハンドル
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
	//選択処理
	int id = Select(pos); // （パス上を右クリックしても追加されてしまうが…）
	if(movetype == 2){
		//コントロールポイントが選択されている場合のみ
		ControlPoints.remove(id);
	}
}

int Bezier::Select(const QPointF &pos)
{
	// (1) 点をクリックしたか？
	selected = 0;
	movetype = 0;
	int size = ControlPoints.size();
	for(int i=0;i<size;++i){
		//ハンドルの片方
		if(markContains(ControlPoints[i].pp, pos)){
			selected = &(ControlPoints[i]);
			movetype = 1; // ハンドルの登録側（pp）を動かす
			return i;
		}
		//ベジェの両端ではない場合のみ
		if(!(i==0 || i == size -1)){
			//コントロールポイント（パス上の点のこと）
			if(markContains(ControlPoints[i].p, pos)){
				selected = &(ControlPoints[i]);
				movetype = 2;// ハンドルごとコントロールポイント全体を動かす
				return i;
			}
			//ハンドルの逆方向先端
			if(markContains(mirror(ControlPoints[i].p, ControlPoints[i].pp), pos)){
				selected = &(ControlPoints[i]);
				movetype = 3; // ハンドルのミラー側（mirror(pp)）のみを動かす
				return i;
			}
		}				
	}

	// (2) パス上をクリックしたか？		
	if(strokedBand.contains(pos)){
		//コントロールポイントを追加して追加したコントロールポイントのIDを返す
		return AddControlPoint(pos);
	}

	// (3) 上記のどこでもない点
	return 0;
}

void Bezier::Draw(QPainter& p)
{
	//軸を描画
	if(flagDrawAxis){
		DrawAxis(&p);
	}

	//運転グラフを描画
	if(flagDrawOperationGraph){
		DrawOperationGraph(&p);
	}

	//始点を登録
	QPainterPath path(ControlPoints[0].p);

	drawPoint(&p, ControlPoints[0].p);
	drawPoint(&p, ControlPoints[0].pp);
	drawLine(&p, ControlPoints[0].p, ControlPoints[0].pp);

	//中間コントロールポイントの数
	int size = ControlPoints.size();
	for(int i=1;i<ControlPoints.size();i++){ // １スタート		

		//カレントコントロール
		QPointF center = ControlPoints[i].p;
		QPointF r_handle = ControlPoints[i].pp;
		QPointF i_handle = mirror(ControlPoints[i].p, ControlPoints[i].pp);

		//パスの描画
		QPointF c_con;
		if(i==size-1){
			c_con = r_handle;
		}else{
			c_con = i_handle;		
		}
		path.cubicTo(ControlPoints[i-1].pp, c_con, center);

		//カレントコントロールの描画
		drawPoint(&p, center);
		drawPoint(&p, r_handle);
		drawLine(&p, center, r_handle);
		if(i != size - 1){
			//対称軸を描画しない
			drawPoint(&p, i_handle);
			drawLine(&p, center, i_handle);
		}
	}
	p.strokePath(path, QPen(Qt::red, 2));

	//パスの保存
	QPainterPathStroker stroker;
	//幅あり
	stroker.setWidth(10);
	strokedBand = stroker.createStroke(path);	
	//幅なし
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
   //X軸
   painter->drawLine(GetOrigin()-QPointF(20,0), system->GetXMax());
   //Y軸
   painter->drawLine(GetOrigin(), system->GetYMin());
   painter->drawLine(GetOrigin(), system->GetYMax());
   //Y軸上限ガイド
   pen.setColor(QColor(0,255,0));
   painter->setPen(pen);
   painter->drawLine(system->GetYMin()-QPointF(20,0), system->GetXMax() + (system->GetYMin() - GetOrigin()));
   painter->drawLine(system->GetYMax()-QPointF(20,0), system->GetXMax() + (system->GetYMax() - GetOrigin()));
   //ラベル
   pen.setColor(QColor(0,0,0));
   painter->setPen(pen);
   //原点ラベル
   painter->drawText(GetOrigin()+QPointF(-15,+15),QObject::tr("O"));
   painter->restore();
}

void Bezier::drawPoint(QPainter *painter, const QPointF &pos)
{
    painter->save();
	//半透明色指定
    painter->setPen(QColor(50, 100, 120, 200));	
    painter->setBrush(QColor(200, 200, 210, 120));
	//輪郭を描画
    painter->drawEllipse(QRectF(pos.x()-pointSize, pos.y()-pointSize, pointSize*2, pointSize*2));	
    painter->restore();
}

void Bezier::drawLine(QPainter *painter, const QPointF &start, const QPointF &end)
{
    painter->save();
	//半透明色，破線
    QPen pen(QColor(255, 0, 255, 127), 1, Qt::DashLine);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(start, end);
    painter->restore();
}
