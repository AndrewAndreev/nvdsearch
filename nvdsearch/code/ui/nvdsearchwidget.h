#ifndef NVDSEARCHWIDGET_H
#define NVDSEARCHWIDGET_H

#include <QGridLayout>
#include <QTabWidget>
#include <QWidget>

#include "data/database.h"
#include "ui/cvesearchwidget.h"

class NvdSearchWidget : public QWidget
{
  Q_OBJECT
public:
  explicit NvdSearchWidget( QWidget *parent = nullptr );
  ~NvdSearchWidget();

private:
  Database _database;

  QGridLayout *_layout;
  QTabWidget *_tabs;

  CveSearchWidget *_cve_search_widget;
};

#endif  // !NVDSEARCHWIDGET_H
