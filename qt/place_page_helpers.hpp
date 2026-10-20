#pragma once

#include <QtCore/QString>

#include <string_view>

namespace place_page_dialog
{
QString MakeHtmlLink(QString const & url, QString const & text);
QString ShortDescription(std::string_view description);
}  // namespace place_page_dialog
