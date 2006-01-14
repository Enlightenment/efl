#ifndef QT_QWS_EBX
#define QT_QWS_EBX
#endif
#ifndef QT_QWS_CUSTOM
#define QT_QWS_CUSTOM
#endif
#ifndef QWS
#define QWS
#endif

#include "evas_test_main.h"
#include <qpe/qpeapplication.h>
#include "evas_software_qtopia_main.h"
#include <qstring.h>
#include "Evas.h"
#include "Evas_Engine_Software_Qtopia.h"
#include <stdio.h>
#include <string.h>

EvasTargetWidget::EvasTargetWidget(QWidget *parent, const char *name)
  : QWidget(parent, name)
{
   this->setFocusPolicy(StrongFocus);
   this->setBackgroundMode(NoBackground);
   idleTimer = new QTimer(this);
   connect(idleTimer, SIGNAL(timeout()), SLOT(idleHandler()));
   idleTimer->start(0, FALSE);
   fullscreen = 0;
}

QSizePolicy EvasTargetWidget::sizePolicy() const
{
   return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void EvasTargetWidget::mousePressEvent(QMouseEvent *e)
{
   if (fullscreen) disableFullscreen();
   else enableFullscreen();
   evas_event_feed_mouse_move(evas, e->x(), e->y(), NULL);
   evas_event_feed_mouse_down(evas, e->button(), EVAS_BUTTON_NONE, NULL);
}

void EvasTargetWidget::mouseReleaseEvent(QMouseEvent *e)
{
   evas_event_feed_mouse_move(evas, e->x(), e->y(), NULL);
   evas_event_feed_mouse_up(evas, e->button(). EVAS_BUTTON_NONE, NULL);
}

void EvasTargetWidget::mouseMoveEvent(QMouseEvent *e)
{
   evas_event_feed_mouse_move(evas, e->x(), e->y(), NULL);
}

void EvasTargetWidget::paintEvent(QPaintEvent *e)
{
   QRect r = e->rect();
   evas_damage_rectangle_add(evas, r.x(), r.y(), r.width(), r.height());
}

void EvasTargetWidget::resizeEvent(QResizeEvent *e)
{
   QSize sz;

   sz = e->size();
   evas_output_size_set(evas, sz.width(), sz.height());
}

void EvasTargetWidget::idleHandler(void)
{
   loop();
   evas_render(evas);
}

void EvasTargetWidget::enableFullscreen(void)
{
   fullscreen = 1;
   setFixedSize(qApp->desktop()->size());
   showNormal();
   reparent(0, WStyle_Customize|WStyle_NoBorder, QPoint(0,0));
   showFullScreen();
}

void EvasTargetWidget::disableFullscreen(void)
{
   fullscreen = 0;
   showNormal();
   showMaximized();
}

EvasTestApp::EvasTestApp(QWidget *parent, const char *name, int wFlags)
  : QMainWindow(parent, name, wFlags)
{
   setCaption(tr("Evas Qtopia Demo"));

   evas_target_widget = new EvasTargetWidget(this, "EvasTargetWidget");

   evas_init();
   evas = evas_new();
   evas_output_method_set(evas, evas_render_method_lookup("software_qtopia"));
   evas_output_size_set(evas, win_w, win_h);
   evas_output_viewport_set(evas, 0, 0, win_w, win_h);
     {
	Evas_Engine_Info_Software_Qtopia *einfo;

	einfo = (Evas_Engine_Info_Software_Qtopia *)evas_engine_info_get(evas);

	/* the following is specific to the engine */
	einfo->info.target = evas_target_widget;
	evas_engine_info_set(evas, (Evas_Engine_Info *) einfo);
     }
   setup();
   orig_start_time = start_time = get_time();
}

EvasTestApp::~EvasTestApp()
{
   evas_shutdown();
}

QSizePolicy EvasTestApp::sizePolicy() const
{
   return QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void EvasTestApp::resizeEvent(QResizeEvent *e)
{
   QSize sz;

   sz = e->size();
   evas_target_widget->resize(sz.width(), sz.height());
}

int
main(int argc, char **argv)
{
   QPEApplication a(argc, argv);
   EvasTestApp test_app;
   a.showMainDocumentWidget(&test_app);
   return a.exec();
}
