#include "qt/place_page_helpers.hpp"

namespace place_page_dialog
{
QString MakeHtmlLink(QString const & url, QString const & text)
{
  return QString("<a href=\"%1\">%2</a>").arg(url.toHtmlEscaped(), text.toHtmlEscaped());
}
}  // namespace place_page_dialog
