// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/widgets/toggle_widget.hpp"
#include "ttauri/crt.hpp"

using namespace tt;

int tt_main(int argc, char *argv[])
{
    auto gui = gui_system::make_unique();
    auto &window = gui->make_window(l10n("Toggle example"));
    window.content().make_widget<label_widget>("A1", l10n("toggle:"));

/// [Create a toggle]
    observable<int> value = 0;

    auto &tb = window.content().make_widget<toggle_widget>("B1", value, 1, 2);
    tb.on_label = l10n("on");
    tb.off_label = l10n("off");
    tb.other_label = l10n("other");
/// [Create a toggle]

    return gui->loop();
}
