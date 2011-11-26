#ifndef __TEMPLATE_TAB_HPP__
#define __TEMPLATE_TAB_HPP__

#include <iostream>

class GhostCore;

class TemplateTab : public QObject
{
	Q_OBJECT

public:
	TemplateTab(GhostCore *parent);
	~TemplateTab();

private:
	GhostCore* parent;
};

#endif