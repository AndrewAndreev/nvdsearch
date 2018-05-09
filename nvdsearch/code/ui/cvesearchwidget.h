#ifndef CVESEARCHWIDGET_H
#define CVESEARCHWIDGET_H

#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>
#include "data\database.h"

class CveSearchWidget : public QWidget
{
  Q_OBJECT

public:
  explicit CveSearchWidget( QWidget *parent, Database &database );
  ~CveSearchWidget();

public slots:
  void resetFilters();
  void applyFilters();

private:
  QGridLayout *_grid;
  QLineEdit *_search_field;

  QLabel *_min_date_label;
  QLabel *_max_date_label;
  QLabel *_published_date_label;
  QDateTimeEdit *_min_date_edit;
  QDateTimeEdit *_max_date_edit;

  QLabel *_severity_label;
  QLabel *_lowest_sev_label;
  QLabel *_highest_sev_label;
  QDoubleSpinBox *_lowest_sev_spin;
  QDoubleSpinBox *_highest_sev_spin;

  QPushButton *_reset_filters_button;
  QPushButton *_apply_filters_button;
  QTableWidget *_cve_table;

  Database &_database;
  Cves _found_cves;
};

#endif  // !CVESEARCHWIDGET_H
