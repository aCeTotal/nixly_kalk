#include "text/phrase.h"

#include <QLocale>
#include <QStringList>
#include <algorithm>
#include <array>
#include <optional>

namespace {

enum class Language { English, Bokmal, Nynorsk };

constexpr std::size_t kPhraseCount = 7;
constexpr std::size_t kLanguageCount = 3;

using Phrases = std::array<QStringView, kPhraseCount>;

constexpr std::array<Phrases, kLanguageCount> kPhrases{{
    {u"Calculator", u"Standard", u"Scientific", u"Invalid expression", u"Cannot divide by zero",
     u"Invalid input", u"Overflow"},
    {u"Kalkulator", u"Standard", u"Vitenskapelig", u"Ugyldig uttrykk", u"Kan ikke dele på null",
     u"Ugyldig inndata", u"Overflyt"},
    {u"Kalkulator", u"Standard", u"Vitskapleg", u"Ugyldig uttrykk", u"Kan ikkje dele på null",
     u"Ugyldig inndata", u"Overflyt"},
}};

struct Prefix {
    QStringView tag;
    Language language;
};

constexpr std::array kPrefixes{
    Prefix{u"nb", Language::Bokmal},
    Prefix{u"no", Language::Bokmal},
    Prefix{u"nn", Language::Nynorsk},
    Prefix{u"en", Language::English},
};

std::optional<Language> languageOf(QStringView tag)
{
    const auto match = std::ranges::find_if(kPrefixes, [tag](const Prefix &prefix) {
        return tag.startsWith(prefix.tag);
    });
    if (match == kPrefixes.end())
        return std::nullopt;
    return match->language;
}

Language systemLanguage()
{
    for (const QString &tag : QLocale::system().uiLanguages()) {
        if (const std::optional<Language> language = languageOf(tag))
            return *language;
    }
    return Language::English;
}

} // namespace

QStringView phrase(Phrase phrase)
{
    static const Language language = systemLanguage();
    return kPhrases[static_cast<std::size_t>(language)][static_cast<std::size_t>(phrase)];
}
