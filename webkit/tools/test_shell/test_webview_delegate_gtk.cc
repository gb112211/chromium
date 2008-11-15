// Copyright (c) 2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file was forked off the Mac port.

#include "webkit/tools/test_shell/test_webview_delegate.h"

#include <gtk/gtk.h>

#include "base/gfx/point.h"
#include "base/string_util.h"
#include "net/base/net_errors.h"
#include "chrome/common/page_transition_types.h"
#include "webkit/glue/webcursor.h"
#include "webkit/glue/webdatasource.h"
#include "webkit/glue/webdropdata.h"
#include "webkit/glue/weberror.h"
#include "webkit/glue/webframe.h"
#include "webkit/glue/webpreferences.h"
#include "webkit/glue/weburlrequest.h"
#include "webkit/glue/webkit_glue.h"
#include "webkit/glue/webview.h"
#include "webkit/glue/window_open_disposition.h"
#include "webkit/tools/test_shell/test_navigation_controller.h"
#include "webkit/tools/test_shell/test_shell.h"

// WebViewDelegate -----------------------------------------------------------

TestWebViewDelegate::~TestWebViewDelegate() {
}

WebPluginDelegate* TestWebViewDelegate::CreatePluginDelegate(
    WebView* webview,
    const GURL& url,
    const std::string& mime_type,
    const std::string& clsid,
    std::string* actual_mime_type) {
  NOTIMPLEMENTED();
  return NULL;
}

void TestWebViewDelegate::ShowJavaScriptAlert(const std::wstring& message) {
  // TODO(port): remove GTK_WINDOW bit after gfx::WindowHandle is fixed.
  GtkWidget* dialog = gtk_message_dialog_new(
      GTK_WINDOW(shell_->mainWnd()), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO,
      GTK_BUTTONS_OK, "%s", WideToUTF8(message).c_str());
  gtk_window_set_title(GTK_WINDOW(dialog), "JavaScript Alert");
  gtk_dialog_run(GTK_DIALOG(dialog));  // Runs a nested message loop.
  gtk_widget_destroy(dialog);
}

void TestWebViewDelegate::Show(WebWidget* webwidget,
                               WindowOpenDisposition disposition) {
  WebWidgetHost* host = GetHostForWidget(webwidget);
  GtkWidget* drawing_area = host->window_handle();
  GtkWidget* window =
      gtk_widget_get_parent(gtk_widget_get_parent(drawing_area));
  gtk_widget_show_all(window);
}

void TestWebViewDelegate::CloseWidgetSoon(WebWidget* webwidget) {
  if (webwidget == shell_->popup()) {
    shell_->ClosePopup();
  } else {
    // In the Windows code, this closes the main window. However, it's not
    // clear when this would ever be needed by WebKit.
    NOTIMPLEMENTED();
  }
}

void TestWebViewDelegate::SetCursor(WebWidget* webwidget, 
                                    const WebCursor& cursor) {
  GdkCursorType cursor_type = cursor.GetCursorType();
  GdkCursor* gdk_cursor;
  if (cursor_type == GDK_CURSOR_IS_PIXMAP) {
    // TODO(port): WebKit bug https://bugs.webkit.org/show_bug.cgi?id=16388 is
    // that calling gdk_window_set_cursor repeatedly is expensive.  We should
    // avoid it here where possible.
    gdk_cursor = cursor.GetCustomCursor();
  } else {
    // Optimize the common case, where the cursor hasn't changed.
    // However, we can switch between different pixmaps, so only on the
    // non-pixmap branch.
    if (cursor_type_ == cursor_type)
      return;
    gdk_cursor = gdk_cursor_new(cursor_type);
  }
  cursor_type_ = cursor_type;
  gdk_window_set_cursor(shell_->webViewWnd()->window, gdk_cursor);
  // The window now owns the cursor.
  gdk_cursor_unref(gdk_cursor);
}

void TestWebViewDelegate::GetWindowRect(WebWidget* webwidget,
                                        gfx::Rect* out_rect) {
  DCHECK(out_rect);
  WebWidgetHost* host = GetHostForWidget(webwidget);
  GtkWidget* drawing_area = host->window_handle();
  GtkWidget* vbox = gtk_widget_get_parent(drawing_area);
  GtkWidget* window = gtk_widget_get_parent(vbox);

  gint x, y;
  gtk_window_get_position(GTK_WINDOW(window), &x, &y);
  x += vbox->allocation.x + drawing_area->allocation.x;
  y += vbox->allocation.y + drawing_area->allocation.y;

  out_rect->SetRect(x, y, drawing_area->allocation.width,
                    drawing_area->allocation.height);
}

void TestWebViewDelegate::SetWindowRect(WebWidget* webwidget,
                                        const gfx::Rect& rect) {
  if (webwidget == shell_->webView()) {
    // ignored
  } else if (webwidget == shell_->popup()) {
    WebWidgetHost* host = GetHostForWidget(webwidget);
    GtkWidget* drawing_area = host->window_handle();
    GtkWidget* window =
        gtk_widget_get_parent(gtk_widget_get_parent(drawing_area));
    gtk_window_resize(GTK_WINDOW(window), rect.width(), rect.height());
    gtk_window_move(GTK_WINDOW(window), rect.x(), rect.y());
  }
}

void TestWebViewDelegate::GetRootWindowRect(WebWidget* webwidget,
                                            gfx::Rect* out_rect) {
  //if (WebWidgetHost* host = GetHostForWidget(webwidget)) {
    NOTIMPLEMENTED();
  //}
}

void TestWebViewDelegate::RunModal(WebWidget* webwidget) {
  NOTIMPLEMENTED();
}

// Private methods -----------------------------------------------------------

void TestWebViewDelegate::SetPageTitle(const std::wstring& title) {
  gtk_window_set_title(GTK_WINDOW(shell_->mainWnd()),
                       ("Test Shell - " + WideToUTF8(title)).c_str());
}

void TestWebViewDelegate::SetAddressBarURL(const GURL& url) {
  gtk_entry_set_text(GTK_ENTRY(shell_->editWnd()), url.spec().c_str());
}
