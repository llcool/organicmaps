#include "qt/qt_common/translations.hpp"

#include "platform/preferred_languages.hpp"

#include "base/assert.hpp"
#include "base/logging.hpp"

#include <QtCore/QLocale>
#include <QtCore/QStringList>
#include <QtWidgets/QApplication>

namespace qt
{
std::unique_ptr<QTranslator> InstallTranslator(QApplication & app, std::span<std::string const> preferred)
{
  QStringList candidates;
  for (auto const & language : preferred)
  {
    // Prefer an exact regional catalog, then let the core parser select the language and Chinese script.
    auto original = QString::fromStdString(language);
    original.replace('_', '-');
    candidates << original.toLower() << QString::fromStdString(languages::GetTwine(language)).toLower();
  }
  candidates << "en";
  candidates.removeDuplicates();

  auto translator = std::make_unique<QTranslator>();
  bool loaded = false;
  for (auto const & language : candidates)
  {
    if (translator->load(QString(":/translations/organicmaps_%1.qm").arg(language)))
    {
      loaded = true;
      break;
    }
  }
  CHECK(loaded, ("Missing embedded Qt translations, including English"));
  CHECK(app.installTranslator(translator.get()), ());
  app.setLayoutDirection(QLocale(translator->language()).textDirection());
  LOG(LINFO, ("Loaded Qt translation", translator->filePath().toStdString()));
  return translator;
}

QString Tr(char const * id, int n)
{
  return qtTrId(id, n);
}
}  // namespace qt
