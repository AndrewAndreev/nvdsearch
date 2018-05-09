#ifndef CVEWIDGET_H
#define CVEWIDGET_H

#include <QGridLayout>
#include <QLabel>
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
  explicit CveWidget( QWidget *parent = nullptr );
  ~CveWidget();

  void setCve( Cve *cve );

protected:
  void paintEvent( QPaintEvent *_event ) override;

private:
  Cve *_cve;

  QGridLayout *_grid;
  QLabel *_cve_name;
  SeverityCircle *_severity_circle;
};

#endif  // !CVEWIDGET_H
