#include "wx_ogre_render_window.h"
#include "orthographic.h"

#include <OGRE/OgreRoot.h>
#include <OGRE/OgreViewport.h>
#include <OGRE/OgreCamera.h>
#include <OGRE/OgreRenderWindow.h>
#include <OGRE/OgreStringConverter.h>

#ifdef __WXGTK__
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <wx/gtk/win_gtk.h>
#include <GL/glx.h>
#endif

namespace ogre_tools
{

IMPLEMENT_CLASS (wxOgreRenderWindow, wxControl)

BEGIN_EVENT_TABLE (wxOgreRenderWindow, wxControl)
EVT_PAINT (wxOgreRenderWindow::onPaint)
EVT_SIZE (wxOgreRenderWindow::onSize)
EVT_MOUSE_EVENTS (wxOgreRenderWindow::onMouseEvents)
END_EVENT_TABLE ()

//------------------------------------------------------------------------------
unsigned int wxOgreRenderWindow::sm_NextRenderWindowId = 1;
//------------------------------------------------------------------------------
wxOgreRenderWindow::wxOgreRenderWindow (Ogre::Root* ogre_root, wxWindow *parent, wxWindowID id,
                                        const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, bool create_render_window)
    : wxControl( parent, id, pos, size, style, validator )
    , render_window_( 0 )
    , ogre_root_( ogre_root )
    , ortho_scale_( 1.0f )
    , auto_render_(true)
{
  SetBackgroundStyle(wxBG_STYLE_CUSTOM);

  if (create_render_window)
  {
    createRenderWindow();
  }
}

//------------------------------------------------------------------------------
wxOgreRenderWindow::~wxOgreRenderWindow ()
{
  if (render_window_)
  {
    render_window_->removeViewport( 0 );
    ogre_root_->detachRenderTarget(render_window_);
  }

  render_window_ = 0;
}

//------------------------------------------------------------------------------
inline wxSize wxOgreRenderWindow::DoGetBestSize () const
{
  return wxSize (320, 240);
}
//------------------------------------------------------------------------------
Ogre::RenderWindow* wxOgreRenderWindow::getRenderWindow () const
{
  return render_window_;
}

//------------------------------------------------------------------------------
Ogre::Viewport* wxOgreRenderWindow::getViewport () const
{
  return viewport_;
}

void wxOgreRenderWindow::setCamera( Ogre::Camera* camera )
{
  viewport_->setCamera( camera );

  setCameraAspectRatio();

  Refresh();
}

void wxOgreRenderWindow::setCameraAspectRatio()
{
  Ogre::Camera* camera = viewport_->getCamera();
  if ( camera )
  {
    int width;
    int height;
    GetSize( &width, &height );

    camera->setAspectRatio( Ogre::Real( width ) / Ogre::Real( height ) );

    if ( camera->getProjectionType() == Ogre::PT_ORTHOGRAPHIC )
    {
      Ogre::Matrix4 proj;
      buildScaledOrthoMatrix( proj, -width / ortho_scale_ / 2, width / ortho_scale_ / 2, -height / ortho_scale_ / 2, height / ortho_scale_ / 2,
                              camera->getNearClipDistance(), camera->getFarClipDistance() );
      camera->setCustomProjectionMatrix(true, proj);
    }
  }
}

void wxOgreRenderWindow::setOrthoScale( float scale )
{
  ortho_scale_ = scale;

  setCameraAspectRatio();
}

void wxOgreRenderWindow::setPreRenderCallback( boost::function<void ()> func )
{
  pre_render_callback_ = func;
}

void wxOgreRenderWindow::setPostRenderCallback( boost::function<void ()> func )
{
  post_render_callback_ = func;
}

//------------------------------------------------------------------------------
void wxOgreRenderWindow::onPaint (wxPaintEvent &evt)
{
  evt.Skip();

  if (auto_render_)
  {
    if ( pre_render_callback_ )
    {
      pre_render_callback_();
    }

    if( ogre_root_->_fireFrameStarted() )
    {
#if (OGRE_VERSION_MAJOR >= 1 && OGRE_VERSION_MINOR >= 6)
      ogre_root_->_fireFrameRenderingQueued();
#endif

      render_window_->update();

      ogre_root_->_fireFrameEnded();
    }

    if ( post_render_callback_ )
    {
      post_render_callback_();
    }
  }
}

//------------------------------------------------------------------------------
void wxOgreRenderWindow::onSize (wxSizeEvent &evt)
{
  if (render_window_)
  {
    // Setting new size;
    int width;
    int height;
    wxSize size = evt.GetSize ();
    width = size.GetWidth ();
    height = size.GetHeight ();

#if !defined(__WXMAC__)
    render_window_->resize (width, height);
#endif
    // Letting Ogre know the window has been resized;
    render_window_->windowMovedOrResized ();

    setCameraAspectRatio();

    Refresh();
  }

  evt.Skip();
}
//------------------------------------------------------------------------------
void wxOgreRenderWindow::onMouseEvents (wxMouseEvent &evt)
{
  evt.Skip();
}
//------------------------------------------------------------------------------
void wxOgreRenderWindow::createRenderWindow ()
{
  Ogre::NameValuePairList params;
  params["externalWindowHandle"] = getOgreHandle ();

  // Get wx control window size
  int width;
  int height;
  GetSize (&width, &height);
  // Create the render window
  render_window_ = ogre_root_->createRenderWindow (
                     Ogre::String ("OgreRenderWindow") + Ogre::StringConverter::toString (sm_NextRenderWindowId++),
                     width, height, false, &params);

  render_window_->setActive (true);
  render_window_->setVisible(true);
  render_window_->setAutoUpdated(true);

  viewport_ = render_window_->addViewport( NULL );
}
//------------------------------------------------------------------------------
std::string wxOgreRenderWindow::getOgreHandle () const
{
  Ogre::String handle;

#ifdef __WXMSW__
  // Handle for Windows systems
  handle = Ogre::StringConverter::toString((size_t)((HWND)GetHandle()));
#elif defined(__WXGTK__)
  // Handle for GTK-based systems
  std::stringstream str;
  GtkWidget *widget = m_wxwindow;
  gtk_widget_set_double_buffered (widget, FALSE);
  gtk_widget_realize( widget );

      // fake a timer event so we redraw
  // Grab the window object
  GdkWindow *gdkWin = GTK_PIZZA (widget)->bin_window;
  Display* display = GDK_WINDOW_XDISPLAY(gdkWin);
  Window wid = GDK_WINDOW_XWINDOW(gdkWin);

  // Display
  str << (unsigned long)display << ':';

  // Screen (returns "display.screen")
  std::string screenStr = DisplayString(display);
  std::string::size_type dotPos = screenStr.find(".");
  screenStr = screenStr.substr(dotPos+1, screenStr.size());
  str << screenStr << ':';

  // XID
  str << wid << ':';

  // Retrieve XVisualInfo
  int attrlist[] = { GLX_RGBA, GLX_DOUBLEBUFFER, GLX_DEPTH_SIZE, 16, GLX_STENCIL_SIZE, 8, None };
  XVisualInfo* vi = glXChooseVisual(display, DefaultScreen(display), attrlist);
  XSync(GDK_WINDOW_XDISPLAY(widget->window), False);
  str << (unsigned long)vi;

  handle = str.str();
#elif defined(__WXMAC__)
  handle = Ogre::StringConverter::toString((size_t)(GetHandle()));
#else
  // Any other unsupported system
  #error("Not supported on this platform.")
#endif

  return handle;
}

} // namespace ogre_tools
