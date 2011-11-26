#ifndef __JOG_TAB_HPP__
#define __JOG_TAB_HPP__

#include <iostream>
#include <QObject>

class GhostCore;

class JogTab : public QObject
{
	Q_OBJECT
public slots:
	void MoveFinished();

public:
	JogTab(GhostCore *parent = 0);
	~JogTab();

	void Update();
	void Init();

	public slots:
	void AdjustScrollBar(double v,double h, double vc, double hc);
	void Start();

private slots:

	void jogMoveByPosition();
	void jogMove();
	void jogMoveAll();
	void uiCoupling();

private:
	GhostCore *parent;
};

#endif