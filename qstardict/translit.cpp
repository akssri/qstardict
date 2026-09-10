#include "translit.h"

#include <QHash>
#include <QVector>
#include <QStringView>

namespace QStarDict::Translit
{

namespace
{

bool hasDevanagari(const QString &s)
{
    for (const QChar c : s) {
        const ushort u = c.unicode();
        if (u >= 0x0900 && u <= 0x097F)
            return true;
    }
    return false;
}

const QHash<QChar, QString> &devaIndepVowels()
{
    static const QHash<QChar, QString> m = {
        {QChar(0x0905), "a"},  {QChar(0x0906), "A"},  {QChar(0x0907), "i"},
        {QChar(0x0908), "I"},  {QChar(0x0909), "u"},  {QChar(0x090A), "U"},
        {QChar(0x090B), "R"},  {QChar(0x0960), "RR"}, {QChar(0x090C), "lR"},
        {QChar(0x0961), "lRR"},
        {QChar(0x090F), "e"},  {QChar(0x0910), "ai"},
        {QChar(0x0913), "o"},  {QChar(0x0914), "au"},
    };
    return m;
}

const QHash<QChar, QString> &devaVowelSigns()
{
    static const QHash<QChar, QString> m = {
        {QChar(0x093E), "A"},  {QChar(0x093F), "i"},  {QChar(0x0940), "I"},
        {QChar(0x0941), "u"},  {QChar(0x0942), "U"},
        {QChar(0x0943), "R"},  {QChar(0x0944), "RR"},
        {QChar(0x0962), "lR"}, {QChar(0x0963), "lRR"},
        {QChar(0x0947), "e"},  {QChar(0x0948), "ai"},
        {QChar(0x094B), "o"},  {QChar(0x094C), "au"},
    };
    return m;
}

const QHash<QChar, QString> &devaConsonants()
{
    static const QHash<QChar, QString> m = {
        {QChar(0x0915), "k"},  {QChar(0x0916), "kh"}, {QChar(0x0917), "g"},
        {QChar(0x0918), "gh"}, {QChar(0x0919), "G"},
        {QChar(0x091A), "c"},  {QChar(0x091B), "ch"}, {QChar(0x091C), "j"},
        {QChar(0x091D), "jh"}, {QChar(0x091E), "J"},
        {QChar(0x091F), "T"},  {QChar(0x0920), "Th"}, {QChar(0x0921), "D"},
        {QChar(0x0922), "Dh"}, {QChar(0x0923), "N"},
        {QChar(0x0924), "t"},  {QChar(0x0925), "th"}, {QChar(0x0926), "d"},
        {QChar(0x0927), "dh"}, {QChar(0x0928), "n"},
        {QChar(0x092A), "p"},  {QChar(0x092B), "ph"}, {QChar(0x092C), "b"},
        {QChar(0x092D), "bh"}, {QChar(0x092E), "m"},
        {QChar(0x092F), "y"},  {QChar(0x0930), "r"},  {QChar(0x0932), "l"},
        {QChar(0x0935), "v"},  {QChar(0x0936), "z"},  {QChar(0x0937), "S"},
        {QChar(0x0938), "s"},  {QChar(0x0939), "h"},
    };
    return m;
}

QString devaToHK(const QString &in)
{
    const auto &vowels    = devaIndepVowels();
    const auto &vsigns    = devaVowelSigns();
    const auto &cons      = devaConsonants();
    QString out;
    out.reserve(in.size() * 2);
    for (int i = 0; i < in.size(); ++i) {
        const QChar c = in[i];
        auto ci = cons.find(c);
        if (ci != cons.end()) {
            out += *ci;
            out += QLatin1Char('a');
            continue;
        }
        auto vi = vowels.find(c);
        if (vi != vowels.end()) {
            out += *vi;
            continue;
        }
        auto si = vsigns.find(c);
        if (si != vsigns.end()) {
            if (!out.isEmpty() && out.back() == QLatin1Char('a'))
                out.chop(1);
            out += *si;
            continue;
        }
        if (c == QChar(0x094D)) {           // virama
            if (!out.isEmpty() && out.back() == QLatin1Char('a'))
                out.chop(1);
            continue;
        }
        if (c == QChar(0x0902)) { out += QLatin1Char('M'); continue; }
        if (c == QChar(0x0903)) { out += QLatin1Char('H'); continue; }
        if (c == QChar(0x0901)) { out += QLatin1Char('~'); continue; }
        if (c == QChar(0x093D)) { out += QLatin1Char('\''); continue; }
        const ushort u = c.unicode();
        if (u >= 0x0966 && u <= 0x096F) {
            out += QChar('0' + (u - 0x0966));
            continue;
        }
        out += c;
    }
    return out;
}

// HK phonemes in longest-first order for greedy tokenization.
const QVector<QString> &hkTokens()
{
    static const QVector<QString> t = {
        "lRR", "RR", "lR",
        "kh", "gh", "ch", "jh", "Th", "Dh", "th", "dh", "ph", "bh", "ai", "au",
        "k", "g", "G", "c", "j", "J", "T", "D", "N",
        "t", "d", "n", "p", "b", "m",
        "y", "r", "l", "v", "z", "S", "s", "h",
        "a", "A", "i", "I", "u", "U", "R", "e", "o", "M", "H"
    };
    return t;
}

QString retokenizeHK(const QString &hk, const QHash<QString, QString> &map)
{
    const auto &tokens = hkTokens();
    QString out;
    out.reserve(hk.size() * 2);
    int i = 0;
    while (i < hk.size()) {
        bool matched = false;
        for (const QString &tok : tokens) {
            if (QStringView(hk).mid(i).startsWith(tok)) {
                auto it = map.find(tok);
                out += (it != map.end()) ? *it : tok;
                i += tok.size();
                matched = true;
                break;
            }
        }
        if (!matched) {
            out += hk[i];
            ++i;
        }
    }
    return out;
}

// Non-identity substitutions per scheme. Anything not listed passes through
// unchanged (i.e. stays in HK form, which is fine for HK targets).
const QHash<QString, QString> &hkToItransMap()
{
    static const QHash<QString, QString> m = {
        {"lRR", "L^I"}, {"RR", "R^I"}, {"lR", "L^i"}, {"R", "R^i"},
        {"G", "~N"}, {"J", "~n"},
        {"A", "aa"}, {"I", "ii"}, {"U", "uu"},
        {"ch", "Ch"}, {"c", "ch"},
        {"z", "sh"}, {"S", "Sh"},
    };
    return m;
}

const QHash<QString, QString> &hkToOptitransMap()
{
    static const QHash<QString, QString> m = {
        {"A", "aa"}, {"I", "ii"}, {"U", "uu"},
        {"G", "~N"}, {"J", "~n"},
        {"ch", "chh"}, {"c", "ch"},
        {"z", "sh"}, {"S", "Sh"},
    };
    return m;
}

const QHash<QString, QString> &hkToSlp1Map()
{
    static const QHash<QString, QString> m = {
        {"kh", "K"}, {"gh", "G"}, {"G", "N"},
        {"ch", "C"}, {"jh", "J"}, {"J", "Y"},
        {"T", "w"}, {"Th", "W"}, {"D", "q"}, {"Dh", "Q"}, {"N", "R"},
        {"th", "T"}, {"dh", "D"},
        {"ph", "P"}, {"bh", "B"},
        {"z", "S"}, {"S", "z"},
        {"R", "f"}, {"RR", "F"}, {"lR", "x"}, {"lRR", "X"},
        {"ai", "E"}, {"au", "O"},
    };
    return m;
}

}  // namespace

QStringList schemeNames()
{
    return {"None", "Devanāgarī", "HK", "ITRANS", "Optitrans", "SLP1"};
}

QString schemeToString(Scheme s)
{
    switch (s) {
    case Scheme::None:       return "None";
    case Scheme::Devanagari: return "Devanāgarī";
    case Scheme::HK:         return "HK";
    case Scheme::ITRANS:     return "ITRANS";
    case Scheme::Optitrans:  return "Optitrans";
    case Scheme::SLP1:       return "SLP1";
    }
    return "None";
}

Scheme schemeFromString(const QString &name)
{
    if (name == "Devanāgarī" || name == "Devanagari") return Scheme::Devanagari;
    if (name == "HK")         return Scheme::HK;
    if (name == "ITRANS")     return Scheme::ITRANS;
    if (name == "Optitrans")  return Scheme::Optitrans;
    if (name == "SLP1")       return Scheme::SLP1;
    return Scheme::None;
}

QString convert(const QString &input, Scheme target)
{
    if (target == Scheme::None || target == Scheme::Devanagari)
        return input;
    if (!hasDevanagari(input))
        return input;
    const QString hk = devaToHK(input);
    switch (target) {
    case Scheme::HK:        return hk;
    case Scheme::ITRANS:    return retokenizeHK(hk, hkToItransMap());
    case Scheme::Optitrans: return retokenizeHK(hk, hkToOptitransMap());
    case Scheme::SLP1:      return retokenizeHK(hk, hkToSlp1Map());
    default:                return input;
    }
}

}
