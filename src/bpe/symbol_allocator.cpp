#include <limits>

#include "bpe/symbol_allocator.hpp"

namespace qualix::bpe
{

std::optional<SymbolId>
SymbolAllocator::Allocate() noexcept
{
    if (next_ == InvalidSymbolId)
        return std::nullopt;

    const SymbolId allocated = next_;

    if (next_ ==
        std::numeric_limits<SymbolId>::max())
    {
        next_ = InvalidSymbolId;
    }
    else
    {
        ++next_;
    }

    return allocated;
}

} // namespace qualix::bpe
