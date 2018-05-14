#ifndef CVEWIDGET_H
#define CVEWIDGET_H

#include <QGridLayout>
#include <QLabel>
#include <QListWidget>
#include <QPaintEvent>
#include <QPainter>
#include <QWidget>

#include "data/cve.h"
#include "data/database.h"

class SeverityCircle : public QWidget
{
  Q_OBJECT

public:
  explicit SeverityCircle( QWidget *parent = nullptr );
  ~SeverityCircle();

  // Severity should be qreal value from 0 to 10.
  void setSeverity( qreal severity );

protected:
  // Redraws a circle.
  void paintEvent( QPaintEvent *_event ) override;

private:
  QLabel *_severity_label;
  qreal _severity;
};

class CveWidget : public QWidget
{
  Q_OBJECT

public:
  CveWidget( QWidget *parent, Database &datbase );
  ~CveWidget();

  void setCve( const Cve &cve );

  Cve *cve();

protected:
  void paintEvent( QPaintEvent *_event ) override;

private:
  Database &_database;
  Cve _cve;

  QGridLayout *_grid;
  QLabel *_cve_desc;
  QLabel *_nvd_link;
  QLabel *_severity_label;
  SeverityCircle *_severity_circle;
  QLabel *_vendors_label;
  QListWidget *_vendors_list;
  QLabel *_selected_vendor_label;
  QListWidget *_products_list;
  QLabel *_selected_product_label;
  QListWidget *_versions_list;
};

#endif  // !CVEWIDGET_H
