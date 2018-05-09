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
CveWidget::CveWidget( QWidget *parent ) : QWidget( parent )
{
  _grid = new QGridLayout( this );
  _cve_name = new QLabel( this );
  _severity_circle = new SeverityCircle( this );

  _grid->addWidget( _cve_name, 0, 0, 1, 2, Qt::AlignLeft );
  _grid->addWidget( _severity_circle, 0, 2 );

  setLayout( _grid );
}

CveWidget::~CveWidget()
{
}

void CveWidget::setCve( Cve *cve )
{
  _cve = cve;
  _severity_circle->setSeverity( _cve->severity() );
  _cve_name->setText( _cve->cveName() );
}

void CveWidget::paintEvent( QPaintEvent * )
{
  QPainter painter( this );

  painter.setPen( Qt::NoPen );
  painter.setBrush( Qt::white );
  painter.drawRect( 0, 0, this->width(), this->height() );
}
