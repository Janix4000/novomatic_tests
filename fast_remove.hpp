#include <algorithm>
#include <iterator>

template <typename Container>
void fast_erase(Container &container, typename Container::iterator to_erase)
{
    auto last_element = std::prev(container.end());
    std::swap(*to_erase, *last_element);
    container.erase(last_element);
}
