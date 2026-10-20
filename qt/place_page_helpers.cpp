#include "qt/place_page_helpers.hpp"

namespace place_page_dialog
{
QString MakeHtmlLink(QString const & url, QString const & text)
{
  return QString("<a href=\"%1\">%2</a>").arg(url.toHtmlEscaped(), text.toHtmlEscaped());
}

QString ShortDescription(std::string_view description)
{
  if (auto const end = description.find("</p>"); description.starts_with("<p>") && end != std::string_view::npos)
    description = description.substr(3, end - 3);

  size_t constexpr kMaxBytes = 500;
  if (description.size() <= kMaxBytes)
    return QString::fromUtf8(description.data(), description.size());

  // Keep the preview byte-limited, reserving three bytes for the ellipsis and preserving UTF-8 code points.
  auto end = kMaxBytes - 3;
  while ((static_cast<unsigned char>(description[end]) & 0xC0) == 0x80)
    --end;
  return QString::fromUtf8(description.data(), end) + QStringLiteral("…");
}
}  // namespace place_page_dialog
