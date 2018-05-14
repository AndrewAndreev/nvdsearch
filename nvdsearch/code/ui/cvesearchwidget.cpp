#include <QAbstractItemView>
#include <QHeaderView>
#include <QVariant>
#include <QtConcurrent>

#include "cvesearchwidget.h"
#include "cvewidget.h"

CveSearchWidget::CveSearchWidget( QWidget *parent, Database &database )
    : QWidget( parent ), _database( database )
{
  _grid = new QGridLayout( this );
  _search_field = new QLineEdit( this );
  _cvssv_label = new QLabel( this );
  _cvssv_combo = new QComboBox( this );
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
  _loading_overlay = new LoadingOverlayWidget( this );
  _loading_overlay->hide();

  _search_field->setPlaceholderText( "Product name filter" );

  _cvssv_label->setText( "CVSS version" );
  _cvssv_combo->addItem( "Any", QVariant( CVSS::ANY ) );
  _cvssv_combo->addItem( "2", QVariant( CVSS::V2 ) );
  _cvssv_combo->addItem( "3", QVariant( CVSS::V3 ) );
  _cvssv_combo->setCurrentIndex( 0 );

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
  _min_date_edit->setMaximumDate(
      QDateTime::currentDateTimeUtc().addDays( 1 ).date() );
  _min_date_edit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );
  _max_date_edit->setMinimumDate( QDate( 2002, 1, 1 ) );
  _max_date_edit->setMaximumDate(
      QDateTime::currentDateTimeUtc().addDays( 1 ).date() );
  _max_date_edit->setDisplayFormat( "yyyy-MM-dd HH:mm:ss" );

  _cve_table->setSelectionBehavior( QAbstractItemView::SelectRows );
  _cve_table->setSelectionMode( QAbstractItemView::SingleSelection );

  _grid->addWidget( _published_date_label, 0, 0 );
  _grid->addWidget( _min_date_label, 1, 0 );
  _grid->addWidget( _max_date_label, 2, 0 );
  _grid->addWidget( _min_date_edit, 1, 1 );
  _grid->addWidget( _max_date_edit, 2, 1 );
  _grid->addWidget( _severity_label, 0, 3 );
  _grid->addWidget( _lowest_sev_label, 1, 3 );
  _grid->addWidget( _highest_sev_label, 2, 3 );
  _grid->addWidget( _lowest_sev_spin, 1, 4 );
  _grid->addWidget( _highest_sev_spin, 2, 4 );
  _grid->addWidget( _reset_filters_button, 1, 5 );
  _grid->addWidget( _apply_filters_button, 2, 5 );
  _grid->addWidget( _search_field, 3, 0, 1, 3 );
  _grid->addWidget( _cvssv_label, 3, 4 );
  _grid->addWidget( _cvssv_combo, 3, 5 );
  _grid->addWidget( _cve_table, 4, 0, 1, 6 );

  _grid->setRowStretch( 4, 1 );
  _grid->setColumnStretch( 2, 1 );

  setLayout( _grid );
  // stylesheet
  setStyleSheet( "CveSearchWidget { background: white; }" );

  connect( _apply_filters_button, &QPushButton::clicked, this,
           &CveSearchWidget::applyFilters );
  connect( _reset_filters_button, &QPushButton::clicked, this,
           &CveSearchWidget::resetFilters );

  connect( _cve_table->selectionModel(), &QItemSelectionModel::selectionChanged,
           this,
           [this]( const QItemSelection &selected, const QItemSelection & ) {
             auto indexes = selected.indexes();
             if ( indexes.size() == 0 )
               return;
             const auto &cve = _found_cves[indexes[0].row()];
             emit cveSelected( cve );
           } );

  QHeaderView *header =
      qobject_cast<QTableView *>( _cve_table )->horizontalHeader();
  connect( header, &QHeaderView::sectionClicked, [this]( int index ) {
    if ( index == _header_sort ) {  // toggle sort order
      _sort_order = ( _sort_order == Qt::AscendingOrder ) ? Qt::DescendingOrder
                                                          : Qt::AscendingOrder;
    } else {  // reset sort order and change header
      _sort_order = Qt::AscendingOrder;
      _header_sort = index;
    }
    applyFilters();
  } );

  resetFilters();
  applyFilters();
}

CveSearchWidget::~CveSearchWidget()
{
  if ( _apply_watcher ) {
    _apply_watcher->cancel();
    _apply_watcher->waitForFinished();
    _apply_watcher->deleteLater();
    _apply_watcher = nullptr;
  }
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
  // Set flag for one run in a queue to run after this one ends.
  if ( _apply_watcher ) {
    _is_apply_filters_run_in_queue = true;
    return;
  }

  _apply_filters_button->setEnabled( false );
  _loading_overlay->show();

  _apply_watcher = new QFutureWatcher<void>( this );

  // Run query in parallel thread.
  QFuture<void> future = QtConcurrent::run( [this]() {
    auto cvss_filter = CVSS(
        _cvssv_combo->itemData( _cvssv_combo->currentIndex() ).value<int>() );
    _found_cves = _database.getProductCves(
        "%" + _search_field->text() + "%", _min_date_edit->dateTime(),
        _max_date_edit->dateTime(), _lowest_sev_spin->value(),
        _highest_sev_spin->value(), cvss_filter,
        QString( "order by %1 %2" )
            .arg( _db_columns[_header_sort] )
            .arg( ( _sort_order == Qt::AscendingOrder ) ? "asc" : "desc" ) );
  } );

  connect( _apply_watcher, &QFutureWatcher<void>::finished, this, [this]() {
    _cve_table->clear();
    _cve_table->setRowCount( 0 );
    setupTableHeaders();

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
    _loading_overlay->hide();

    _apply_watcher->deleteLater();
    _apply_watcher = nullptr;
    _apply_filters_button->setEnabled( true );

    if ( _is_apply_filters_run_in_queue ) {
      _is_apply_filters_run_in_queue = false;
      applyFilters();
    }
  } );

  _apply_watcher->setFuture( future );
}

void CveSearchWidget::resizeEvent( QResizeEvent * )
{
  _loading_overlay->setGeometry( _cve_table->geometry() );
}

void CveSearchWidget::setupTableHeaders()
{
  _cve_table->setColumnCount( _headers.size() );

  QString arrow_up = QString( _arrow_up );
  QString arrow_down = QString( _arrow_down );

  auto headers = _headers;
  headers[_header_sort] +=
      "   " + ( ( _sort_order == Qt::AscendingOrder ) ? arrow_up : arrow_down );
  _cve_table->setHorizontalHeaderLabels( headers );
  _cve_table->horizontalHeader()->setStretchLastSection( true );
}
