#include "cvewidget.h"

//==============================================================================
// SeverityCircle
//==============================================================================
SeverityCircle::SeverityCircle( QWidget *parent ) : QWidget( parent )
{
  setFixedSize( 64, 64 );  // Default circle size is 64x64 px.
  _severity_label = new QLabel( this );
  _severity_label->setGeometry( 0, 0, this->width(), this->height() );
  _severity_label->setAlignment( Qt::AlignCenter );
  // Default severity value.
  setSeverity( 0.0 );
}

SeverityCircle::~SeverityCircle()
{
}

void SeverityCircle::setSeverity( qreal severity )
{
  _severity = severity;
  _severity_label->setText( QString::number( _severity, 'f', 1 ) );
  update();
}

void SeverityCircle::paintEvent( QPaintEvent * )
{
  QPainter painter( this );
  painter.setRenderHint( QPainter::Antialiasing );
  painter.setRenderHint( QPainter::HighQualityAntialiasing );

  int pen_width = 16;
  QRectF rect( pen_width / 2, pen_width / 2, this->width() - pen_width - 1,
               this->height() - pen_width - 1 );
  // Background Arc
  QPen pen_bg( Qt::gray, pen_width, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin );
  painter.setPen( pen_bg );
  painter.drawArc( rect, 0, 360 * 16 );

  auto severity_norm = _severity / 10.0;
  QColor severity_color =
      QColor::fromHsv( 120 * ( 1.0 - severity_norm ), 255, 200 );
  // Negative value angles - clockwise direction.
  int start_angle = 90 * 16;  // in 1/16 of a degree.
  int span_angle = -int( severity_norm * 360 - 90 ) * 16 - start_angle;
  QPen pen_fg( severity_color, pen_width, Qt::SolidLine, Qt::FlatCap,
               Qt::MiterJoin );
  painter.setPen( pen_fg );
  painter.drawArc( rect, start_angle, span_angle );
}

//==============================================================================
// CveWidget
//==============================================================================
CveWidget::CveWidget( QWidget *parent, Database &database )
    : QWidget( parent ), _database( database )
{
  _grid = new QGridLayout( this );
  _cve_desc = new QLabel( this );
  _nvd_link = new QLabel( this );
  _severity_label = new QLabel( this );
  _severity_circle = new SeverityCircle( this );
  _vendors_label = new QLabel( this );
  _vendors_list = new QListWidget( this );
  _selected_vendor_label = new QLabel( this );
  _products_list = new QListWidget( this );
  _selected_product_label = new QLabel( this );
  _versions_list = new QListWidget( this );

  _nvd_link->setTextFormat( Qt::RichText );
  _nvd_link->setTextInteractionFlags( Qt::TextBrowserInteraction );
  _nvd_link->setOpenExternalLinks( true );

  _cve_desc->setWordWrap( true );
  _vendors_label->setWordWrap( true );
  _selected_vendor_label->setWordWrap( true );
  _selected_product_label->setWordWrap( true );

  _grid->setRowStretch( 1, 1 );
  _grid->setRowStretch( 3, 2 );
  _grid->setColumnStretch( 0, 1 );
  _grid->setColumnStretch( 1, 2 );
  _grid->setColumnStretch( 2, 1 );

  _grid->addWidget( _cve_desc, 0, 0, 2, 2, Qt::AlignLeft );
  _grid->addWidget( _nvd_link, 2, 0, 1, 2, Qt::AlignLeft );
  _grid->addWidget( _severity_circle, 0, 2, Qt::AlignCenter );
  _grid->addWidget( _severity_label, 1, 2, Qt::AlignHCenter | Qt::AlignTop );

  _vendors_label->setText( "Vendors" );

  _grid->addWidget( _vendors_label, 3, 0, Qt::AlignBottom );
  _grid->addWidget( _selected_vendor_label, 3, 1, Qt::AlignBottom );
  _grid->addWidget( _selected_product_label, 3, 2, Qt::AlignBottom );

  _grid->addWidget( _vendors_list, 4, 0 );
  _grid->addWidget( _products_list, 4, 1 );
  _grid->addWidget( _versions_list, 4, 2 );

  setLayout( _grid );

  connect( _vendors_list, &QListWidget::itemSelectionChanged, this, [this]() {
    _products_list->clear();
    _versions_list->clear();
    auto selected_items = _vendors_list->selectedItems();
    if ( selected_items.size() == 0 )
      return;
    const auto &cve_name = _cve.cveName();
    // Get first selected.
    const auto &selected_vendor = selected_items[0]->text();
    _selected_vendor_label->setText(
        QString( "%1 products" ).arg( selected_vendor ) );
    _products_list->addItems(
        _database.getCveProducts( cve_name, selected_vendor ) );
  } );
  connect( _products_list, &QListWidget::itemSelectionChanged, this, [this]() {
    _versions_list->clear();
    auto selected_vendors = _vendors_list->selectedItems();
    auto selected_products = _products_list->selectedItems();
    if ( selected_vendors.size() == 0 || selected_products.size() == 0 )
      return;
    const auto &cve_name = _cve.cveName();
    // Get first selected.
    const auto &selected_vendor = selected_vendors[0]->text();
    const auto &selected_product = selected_products[0]->text();

    _selected_vendor_label->setText(
        QString( "%1 products" ).arg( selected_vendor ) );
    _selected_product_label->setText(
        QString( "%1 versions" ).arg( selected_product ) );

    _versions_list->addItems( _database.getCveProductVersions(
        cve_name, selected_vendor, selected_product ) );
  } );
}

CveWidget::~CveWidget()
{
}

void CveWidget::setCve( const Cve &cve )
{
  _cve = cve;
  const auto &cve_name = _cve.cveName();
  _severity_circle->setSeverity( _cve.severity() );
  _cve_desc->setText( "Description:\n" +
                      _database.getCveDescription( cve_name ) );
  _nvd_link->setText( QString( "<a "
                               "href=\"https://nvd.nist.gov/vuln/detail/%1"
                               "\">https://nvd.nist.gov/vuln/detail/%1</a>" )
                          .arg( cve_name ) );

  QString severity_version;
  if ( _cve.scoreVersion() == CVSS::V2 )
    severity_version = "CVSS V2";
  else
    severity_version = "CVSS V3";
  _severity_label->setText( severity_version );

  // Filling lists up.
  _selected_vendor_label->setText( "Products" );
  _selected_product_label->setText( "Versions" );

  _vendors_list->clear();
  _vendors_list->addItems( _database.getCveVendors( cve_name ) );
}

Cve *CveWidget::cve()
{
  return &_cve;
}

void CveWidget::paintEvent( QPaintEvent * )
{
  QPainter painter( this );

  painter.setPen( Qt::NoPen );
  painter.setBrush( Qt::white );
  painter.drawRect( 0, 0, this->width(), this->height() );
}
