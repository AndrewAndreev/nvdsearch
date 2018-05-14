#include "nvdsearchwidget.h"

NvdSearchWidget::NvdSearchWidget( QWidget *parent ) : QWidget( parent )
{
  _database.setConnection();

  _tabs = new QTabWidget( this );
  _layout = new QGridLayout( this );
  _cve_search_widget = new CveSearchWidget( this, _database );
  _selected_cve_widget = new CveWidget( this, _database );
  _selected_cve_widget->hide();

  _tabs->addTab( _cve_search_widget, "CVE Search" );

  _layout->addWidget( _tabs );

  setLayout( _layout );
  adjustSize();

  connect( _cve_search_widget, &CveSearchWidget::cveSelected, this,
           &NvdSearchWidget::onCveSelected );
}

NvdSearchWidget::~NvdSearchWidget()
{
}

void NvdSearchWidget::onCveSelected( Cve cve )
{
  if ( _tabs->count() > 1 )
    _tabs->removeTab( 1 );
  _selected_cve_widget->setCve( cve );
  _tabs->insertTab( 1, _selected_cve_widget,
                    _selected_cve_widget->cve()->cveName() );
}
