#pragma once

#include <QtCore/QString>
#include <QtCore/QTranslator>

#include <memory>
#include <span>
#include <string>

class QApplication;

namespace qt
{
std::unique_ptr<QTranslator> InstallTranslator(QApplication & app, std::span<std::string const> preferred);
QString Tr(char const * id, int n = -1);
}  // namespace qt
