#include "window/vim.h"

#include <algorithm>
#include <array>
#include <utility>

namespace {

struct Binding {
    char16_t key;
    VimCommand command;
};

constexpr std::array kSingle{
    Binding{u'h', VimCommand::Left},
    Binding{u'j', VimCommand::Down},
    Binding{u'k', VimCommand::Up},
    Binding{u'l', VimCommand::Right},
    Binding{u' ', VimCommand::Press},
    Binding{u'x', VimCommand::Backspace},
    Binding{u'p', VimCommand::Paste},
    Binding{u'G', VimCommand::Last},
    Binding{u'i', VimCommand::Leave},
    Binding{u'a', VimCommand::Leave},
};

constexpr std::array kDoubled{
    Binding{u'd', VimCommand::Clear},
    Binding{u'y', VimCommand::Copy},
    Binding{u'g', VimCommand::First},
};

template<std::size_t N>
const Binding *bindingFor(const std::array<Binding, N> &bindings, QChar key)
{
    const auto match = std::ranges::find(bindings, key.unicode(), &Binding::key);
    return match == bindings.end() ? nullptr : &*match;
}

} // namespace

VimCommand VimKeys::feed(QChar key)
{
    const QChar pending = std::exchange(pending_, QChar());
    const Binding *doubled = bindingFor(kDoubled, key);
    if (doubled && key == pending)
        return doubled->command;
    if (doubled) {
        pending_ = key;
        return VimCommand::Pending;
    }
    const Binding *single = bindingFor(kSingle, key);
    return single ? single->command : VimCommand::Unbound;
}
