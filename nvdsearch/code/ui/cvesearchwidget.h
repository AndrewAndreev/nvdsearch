#ifndef CVESEARCHWIDGET_H
#define CVESEARCHWIDGET_H

#include <QComboBox>
#include <QDateTimeEdit>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>
#include "data/database.h"

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
  void setupTableHeaders();

  QGridLayout *_grid;
  QLineEdit *_search_field;
  QLabel *_cvssv_label;
  QComboBox *_cvssv_combo;

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
  QStringList _headers{ "Cve name", "Severity", "CVSSV", "Published date" };
  QStringList _db_columns{ "cve_name", "base_score", "version",
                           "published_date" };
  QTableWidget *_cve_table;

  const QChar _arrow_up = QChar( 0x2191 );
  const QChar _arrow_down = QChar( 0x2193 );

  Qt::SortOrder _sort_order = Qt::AscendingOrder;
  int _header_sort = 3;  // Default sorting is an ascending by Published date

  Database &_database;
  Cves _found_cves;
};

#endif  // !CVESEARCHWIDGET_H
