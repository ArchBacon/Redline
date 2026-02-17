#pragma once

#include <visit_struct/visit_struct.hpp>

// Use this macro to define which fields of a struct are visitable.
// Using the BEE_ version will suppress the appropriate linter warning.
#define BEE_VISITABLE_STRUCT /* NOLINTBEGIN(bugprone-use-after-move) */ \
    VISITABLE_STRUCT         /* NOLINTEND(bugprone-use-after-move) */

// Add this inside your class to allow visit_struct to find private members
#define BEFRIEND_VISITABLE()      \
    template <typename, typename> \
    friend struct ::visit_struct::traits::visitable