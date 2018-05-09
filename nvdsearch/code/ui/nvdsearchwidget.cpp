#include "nvdsearchwidget.h"

NvdSearchWidget::NvdSearchWidget( QWidget *parent ) : QWidget( parent )
{
  _database.setConnection();

  _tabs = new QTabWidget( this );
  _layout = new QGridLayout( this );
  _cve_search_widget = new CveSearchWidget( this, _database );

  _tabs->addTab( _cve_search_widget, "CVE Search" );

  _layout->addWidget( _tabs );

  setLayout( _layout );
  adjustSize();
}

NvdSearchWidget::~NvdSearchWidget()
{
}
