#include <QAbstractItemView>
#include <QHeaderView>
#include "cvesearchwidget.h"
#include "cvewidget.h"

CveSearchWidget::CveSearchWidget( QWidget *parent, Database &database )
    : QWidget( parent ), _database( database )
{
  _grid = new QGridLayout( this );
  _search_field = new QLineEdit( this );
  _min_date_label = new QLabel( this );
  _max_date_label = new QLabel( this );
  _published_date_label = new QLabel( this );
  _min_date_edit = new QDateTimeEdit( this );
  _max_date_edit = new QDateTimeEdit( this );
  _severity_label = new QLabel( this );
  _lowest_sev_label = new QLabel( this );
  _highest_sev_label = new QLabel( this );
  _lowest_sev_spin = new QDoubleSpinBox( this );
  _highest_sev_spin = new QDoubleSpinBox( this );
  _reset_filters_button = new QPushButton( this );
  _apply_filters_button = new QPushButton( this );
  _cve_table = new QTableWidget( this );

  _search_field->setPlaceholderText( "CVE name filter" );
  _published_date_label->setText( "Published date:" );
  _min_date_label->setText( "Minimum" );
  _max_date_label->setText( "Maximum" );
  _reset_filters_button->setText( "Reset filters" );
  _apply_filters_button->setText( "Apply filters" );

  _severity_label->setText( "Severity:" );
  _lowest_sev_label->setText( "Lowest" );
  _highest_sev_label->setText( "Highest" );
  _lowest_sev_spin->setRange( 0.0, 10.0 );
  _lowest_sev_spin->setDecimals( 1 );
  _highest_sev_spin->setRange( 0.0, 10.0 );
  _highest_sev_spin->setDecimals( 1 );

  _min_date_edit->setMinimumDate( QDate( 2002, 1, 1 ) );
  _min_date_edit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  _max_date_edit->setMaximumDate(
      QDateTime::currentDateTimeUtc().addDays( 1 ).date() );
  _max_date_edit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  _cve_table->setSelectionBehavior( QAbstractItemView::SelectRows );
  _cve_table->setSelectionMode( QAbstractItemView::SingleSelection );

  _grid->addWidget( _search_field, 0, 0, 3, 1 );
  _grid->addWidget( _published_date_label, 0, 1 );
  _grid->addWidget( _min_date_label, 1, 1 );
  _grid->addWidget( _max_date_label, 2, 1 );
  _grid->addWidget( _min_date_edit, 1, 2 );
  _grid->addWidget( _max_date_edit, 2, 2 );
  _grid->addWidget( _severity_label, 0, 3 );
  _grid->addWidget( _lowest_sev_label, 1, 3 );
  _grid->addWidget( _highest_sev_label, 2, 3 );
  _grid->addWidget( _lowest_sev_spin, 1, 4 );
  _grid->addWidget( _highest_sev_spin, 2, 4 );
  _grid->addWidget( _reset_filters_button, 1, 5 );
  _grid->addWidget( _apply_filters_button, 2, 5 );
  _grid->addWidget( _cve_table, 3, 0, 1, 6 );

  _grid->setRowStretch( 3, 1 );

  setLayout( _grid );
  // stylesheet
  setStyleSheet( "CveSearchWidget { background: white; }" );

  connect( _apply_filters_button, &QPushButton::clicked, this,
           &CveSearchWidget::applyFilters );
  connect( _reset_filters_button, &QPushButton::clicked, this,
           &CveSearchWidget::resetFilters );

  resetFilters();
  applyFilters();
}

CveSearchWidget::~CveSearchWidget()
{
}

void CveSearchWidget::resetFilters()
{
  _min_date_edit->setDateTime(
      QDateTime::fromString( "2002-01-01 00:00:00", "yyyy-MM-dd HH:mm:ss" ) );
  _max_date_edit->setDate( QDateTime::currentDateTimeUtc().date() );
  _search_field->setText( "" );
  _lowest_sev_spin->setValue( 0.0 );
  _highest_sev_spin->setValue( 10.0 );
}

void CveSearchWidget::applyFilters()
{
  _apply_filters_button->setEnabled( false );
  _found_cves =
      _database.getCves( _search_field->text(), _min_date_edit->dateTime(),
                         _max_date_edit->dateTime(), _lowest_sev_spin->value(),
                         _highest_sev_spin->value() );
  _cve_table->clear();
  _cve_table->setRowCount( 0 );

  QStringList headers;
  headers << "Cve name"
          << "Severity"
          << "CVSSV"
          << "Published date";
  _cve_table->setColumnCount( headers.size() );
  _cve_table->setHorizontalHeaderLabels( headers );
  _cve_table->horizontalHeader()->setStretchLastSection( true );

  for ( auto &cve : _found_cves ) {
    _cve_table->insertRow( _cve_table->rowCount() );

    _cve_table->setItem( _cve_table->rowCount() - 1, 0,
                         new QTableWidgetItem( cve.cveName() ) );
    _cve_table->setItem(
        _cve_table->rowCount() - 1, 1,
        new QTableWidgetItem( QString::number( cve.severity() ) ) );
    _cve_table->setItem( _cve_table->rowCount() - 1, 2,
                         new QTableWidgetItem( QString::number(
                             static_cast<int>( cve.scoreVersion() ) ) ) );
    _cve_table->setItem( _cve_table->rowCount() - 1, 3,
                         new QTableWidgetItem( cve.publishedDate().toString(
                             "yyyy-MM-dd HH:mm:ss" ) ) );

    // Make row read-only.
    for ( auto i = 0; i < _cve_table->columnCount(); ++i ) {
      auto item = _cve_table->item( _cve_table->rowCount() - 1, i );
      item->setFlags( item->flags() ^ Qt::ItemIsEditable );
    }
  }

  _cve_table->resizeRowsToContents();
  _cve_table->resizeColumnsToContents();

  _apply_filters_button->setEnabled( true );
}
