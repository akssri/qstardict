#ifndef TRANSLIT_H
#define TRANSLIT_H

#include <QString>
#include <QStringList>

namespace QStarDict::Translit
{

enum class Scheme
{
    None = 0,
    Devanagari,
    HK,
    ITRANS,
    Optitrans,
    SLP1,
};

QStringList schemeNames();
QString schemeToString(Scheme s);
Scheme schemeFromString(const QString &name);

// If input contains any Devanāgarī codepoints and target is a Roman scheme,
// convert. Otherwise return input unchanged.
QString convert(const QString &input, Scheme target);

}

#endif
