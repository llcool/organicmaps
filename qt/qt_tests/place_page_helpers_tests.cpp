#include "testing/testing.hpp"

#include "qt/place_page_helpers.hpp"

#include <QtCore/QUrl>
#include <QtGui/QGuiApplication>
#include <QtGui/QTextBlock>
#include <QtGui/QTextDocument>
#include <QtGui/QTextFragment>

namespace place_page_helpers_tests
{
void EnsureApp()
{
  static int argc = 0;
  static QGuiApplication app(argc, nullptr);
}

UNIT_TEST(PlacePage_LinkKeepsTargetAndEscapesDisplayedText)
{
  EnsureApp();
  QString const url = "https://example.org/route?a=1&b=2";
  QString const text = QString::fromUtf8("example.org/<b> & café");
  QTextDocument document;
  document.setHtml(place_page_dialog::MakeHtmlLink(url, text));
  TEST_EQUAL(document.toPlainText().toStdString(), text.toStdString(), ());
  auto const target = document.begin().begin().fragment().charFormat().anchorHref();
  TEST_EQUAL(target.toStdString(), url.toStdString(), ());
  TEST(!QUrl(target).isRelative(), (target.toStdString()));
}
}  // namespace place_page_helpers_tests
